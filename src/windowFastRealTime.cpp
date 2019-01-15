/*
 * 2018(C) kayspark , 2010-2015 (C) Antonio Redondo
 *
 * https://github.com/kayspark/ImageFeatureDetector
 *
 * Code under the terms of the GNU General Public License v3.
 *
 */

#include "windowFastRealTime.h"
#include "opencvhelper.h"

WindowFastRealTime::WindowFastRealTime(WindowMain *wmain)
    : QDialog(wmain, Qt::Dialog)
    , mCamera(vlc_capture(960, 544))
    , mTimer(std::make_unique<QTimer>())
    , mDetecting(false)
    , _nm_classifier(std::make_unique<nm_classifier>())
    , mTime(0.0)
    , _data_file(":/dataset/cascade.xml")
    , _tracking_algorithm("CSRT")
    , _predator(nm_detector(this->_data_file, this->_tracking_algorithm))
    , mSettings(wmain->getMSettings())
    , mLocale(std::make_unique<QLocale>(QLocale::English))
    , mPainter(std::make_unique<QPainter>()) {
  setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);
  setContextMenuPolicy(Qt::ActionsContextMenu);
  uiSpinBoxThresholdFAST->setValue(mSettings->value("fastRT/threshold", 25).toInt());
  uiPushButtonNonMaxFAST->setChecked(mSettings->value("fastRT/nonMaxSuppression", true).toBool());
  listWidget->setViewMode(QListWidget::IconMode);
  listWidget->setIconSize(QSize(100, 100));
  listWidget->setResizeMode(QListWidget::Adjust);
  listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

  actAbnormal = std::make_unique<QAction>(tr("&Abnormal"), this);
  actNormal = std::make_unique<QAction>(tr("&Normal"), this);
  actClear = std::make_unique<QAction>(tr("&Clear"), this);

  QObject::connect(actNormal.get(), &QAction::triggered, this, &WindowFastRealTime::learnNormal);
  QObject::connect(actAbnormal.get(), &QAction::triggered, this, &WindowFastRealTime::learnAbnormal);
  QObject::connect(actClear.get(), &QAction::triggered, this, &WindowFastRealTime::clearListWidget);

  QObject::connect(listWidget, &QListWidget::customContextMenuRequested, this, &WindowFastRealTime::showContextMenu);
  QObject::connect(uiPushButtonNonMaxFAST, &QPushButton::toggled, this, &WindowFastRealTime::saveFastParams);
  QObject::connect(uiSpinBoxThresholdFAST, &QSpinBox::editingFinished, this, &WindowFastRealTime::saveFastParams);
  QObject::connect(uiPushButtonResetFAST, &QAbstractButton::clicked, this, &WindowFastRealTime::resetUI);
  QObject::connect(uiPushButtonDetect, &QAbstractButton::clicked, this, &WindowFastRealTime::detect);
  QObject::connect(uiPushButtonCancel, &QAbstractButton::clicked, this, &WindowFastRealTime::close);
  std::string url = mSettings->value("rtsp/url1").toString().toStdString();
  mCamera.open(url);
  if (mCamera.isOpened()) {
    mTimer->start(40); // 25fps
    QObject::connect(mTimer.get(), &QTimer::timeout, this, &WindowFastRealTime::compute);
    uiPushButtonDetect->setEnabled(true);
  } else {
    uiLabelRealTime->setText("There is some problem with the cam.\nCannot get images.");
  }
  show();
}
void WindowFastRealTime::showContextMenu(const QPoint &pos) {
  QPoint globalPos = listWidget->mapToGlobal(pos);
  QMenu menu(this);
  menu.addAction(actAbnormal.get());
  menu.addAction(actNormal.get());
  menu.addAction(actClear.get());
  menu.exec(globalPos);
}
void WindowFastRealTime::detect() {
  if (!mDetecting) {
    uiPushButtonDetect->setIcon(QIcon(":icons/media-playback-stop.svg"));
    uiPushButtonDetect->setText("Stop Detecting");
  } else {
    uiPushButtonDetect->setIcon(QIcon(":icons/media-playback-play.svg"));
    uiPushButtonDetect->setText("Detect");
  }
  mDetecting = !mDetecting;
}

void WindowFastRealTime::close() {
  if (mTimer)
    mTimer->stop();
  mCamera.release();
  QWidget::close();
}

void WindowFastRealTime::closeEvent(QCloseEvent *closeEvent) {
  close();
  QDialog::closeEvent(closeEvent);
}

void WindowFastRealTime::mousePressEvent(QMouseEvent *event) {
  mLastPoint = event->pos();
  _mouse_pressed = true;
  _rubberBand = std::make_shared<QRubberBand>(QRubberBand::Rectangle, this);
  _bandList.emplace_back(_rubberBand);
  // setCursor(Qt::ClosedHandCursor);
}

void WindowFastRealTime::mouseMoveEvent(QMouseEvent *event) {
  if (_mouse_pressed) {
    _rubberBand->setGeometry(QRect(mLastPoint, event->pos()).normalized());
    _rubberBand->show();
    QToolTip::showText(event->globalPos(),
                       QString("%1,%2").arg(_rubberBand->size().width()).arg(_rubberBand->size().height()), this);
  }
  // mLastPoint = myPos;
}

void WindowFastRealTime::mouseReleaseEvent(QMouseEvent * /*event*/) {
  // unsetCursor();
  _mouse_pressed = false;
  //_rubberBand->hide();
  // determine selection.. QRect::contains..
}
void WindowFastRealTime::saveFastParams() {
  mSettings->setValue("fastRT/threshol", uiSpinBoxThresholdFAST->value());
  mSettings->setValue("fastRT/nonMaxSuppression", uiPushButtonNonMaxFAST->isChecked());
}

void WindowFastRealTime::resetUI() {
  uiSpinBoxThresholdFAST->setValue(25);
  uiPushButtonNonMaxFAST->setChecked(true);
  if (_bandList.size() > 0) {
    for (auto &band : _bandList) {
      band->hide();
      band = nullptr;
    }
    _bandList.clear();
    _rubberBand = nullptr;
  }
  saveFastParams();
}

void WindowFastRealTime::learnNormal() {}

void WindowFastRealTime::learnAbnormal() {

  QListWidgetItem *currentItem = listWidget->currentItem();
  QIcon icon = currentItem->icon();
  cv::Mat feature = QPixmap2Mat(icon.pixmap(QSize(100, 100)), true);
  _nm_classifier->learn(feature);
}

void WindowFastRealTime::clearListWidget() {
  QList<QListWidgetItem *> selected = listWidget->selectedItems();
  int size = selected.size();
  if (size > 0) {
    for (auto &item : selected) {
      // https://stackoverflow.com/questions/25417348/remove-selected-items-from-listwidget
      listWidget->removeItemWidget(item);
      delete item;
    }
    listWidget->clearSelection();
  } else
    listWidget->clear();
}

void WindowFastRealTime::compute() {

  cv::Mat imgRT;

  mCamera.read(imgRT);
  if (mDetecting) {
    if (!imgRT.empty()) {
      cv::Mat gray;
      cv::cvtColor(imgRT, gray, cv::COLOR_BGR2GRAY);
      std::vector<cv::Rect> roiList;
      for (const auto &band : _bandList) {
        band->showNormal();
        QRect qRect = band->geometry();
        roiList.emplace_back(cv::Rect(qRect.x(), qRect.y(), qRect.width(), qRect.height()));
      }
      std::vector<cv::Rect> motions;
      _predator.detect_candidate(gray, motions);
      //   cv::resize(_imgRT, _imgRT, cv::Size(640, 480), 0, 0,
      //   cv::INTER_CUBIC);
      mPixmap = QPixmap::fromImage(
        QImage(imgRT.data, imgRT.cols, imgRT.rows, static_cast<int>(imgRT.step), QImage::Format_RGB888));
      mPainter->begin(&mPixmap);
      QPen penG(QColor::fromRgb(0, 255, 0));
      penG.setWidth(5);
      QPen penR(QColor::fromRgb(255, 0, 0));
      penR.setWidth(5);
      QPen penB(QColor::fromRgb(0, 0, 255));
      penB.setWidth(5);
      for (const auto &r : motions) {
        // check whether in valid roi
        QRect qr(r.x, r.y, r.width, r.height);
        for (const auto &roi : roiList) {
          if (roi.contains(r.tl()) || roi.contains(r.br())) {
            QPixmap tpix = mPixmap.copy(qr).scaled(QSize(100, 100), Qt::KeepAspectRatio);
            if (_nm_classifier->classify(QPixmap2Mat(tpix, true))) {
              mPainter->setPen(penR);
            } else
              mPainter->setPen(penB);
            QString fileName = QString("%1.png").arg(QDateTime::currentDateTime().toString("MMdd_hhmmss"));
            tpix.toImage().save(QString("backup/%1").arg(fileName));
            listWidget->addItem(new QListWidgetItem(QIcon(tpix), fileName));
            break;
          } else
            mPainter->setPen(penG);
          // fill suspicious belt
        }
        mPainter->drawRect(qr);
      }
      mPainter->end();
      uiLabelRealTime->setPixmap(mPixmap);
      uiLabelTime->setText("Detecting Time: " + QString("%1").arg(_predator.get_detection_time()));
      uiLabelKeypoints->setText("Key points: -" + QString("%1").arg(1));
    }
  } else {
    mPixmap = QPixmap::fromImage(
      QImage(imgRT.data, imgRT.cols, imgRT.rows, static_cast<int>(imgRT.step), QImage::Format_RGB888));
    uiLabelRealTime->setPixmap(mPixmap);
    uiLabelTime->setText("Detecting Time: -");
    uiLabelKeypoints->setText("Key points: -");
  }
}
