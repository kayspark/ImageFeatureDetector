/*
 * 2018(C) kayspark , 2010-2015 (C) Antonio Redondo
 *
 * https://github.com/kayspark/ImageFeatureDetector
 *
 * Code under the terms of the GNU General Public License v3.
 *
 */

#include "windowFastRealTime.hpp"
#include "Section.h"
#include "opencvhelper.hpp"

WindowFastRealTime::WindowFastRealTime(WindowMain *wmain)
    : QDialog(wmain, Qt::Dialog)
    , mCamera(std::make_unique<vlc_capture>(960, 544))
    , mTimer(std::make_unique<QTimer>())
    , mDetecting(false)
    , m_nm_classifier(std::make_unique<nm_classifier>())
    , mTime(0.0)
    , m_data_file(":/dataset/cascade.xml")
    , m_tracking_algorithm("CSRT")
    , m_detector(nm_detector(this->m_data_file, this->m_tracking_algorithm))
    , m_pen(QColor::fromRgb(0, 255, 0))
    , mSettings(wmain->getMSettings())
    , mLocale(std::make_unique<QLocale>(QLocale::English))
    , mPainter(std::make_unique<QPainter>()) {
  setupUi(this);

  auto section = new Section("Detect and Learn", 300, this);
  this->layout()->addWidget(section);

  auto vLayout = new QVBoxLayout();
  mDetectorWidget = new QListWidget(section);
  mDetectorWidget->setFixedHeight(100);
  mDetectorWidget->setSelectionMode(QAbstractItemView::MultiSelection);
  mDetectorWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  mDetectorWidget->setViewMode(QListWidget::IconMode);
  mDetectorWidget->setIconSize(QSize(100, 100));
  mDetectorWidget->setResizeMode(QListWidget::Adjust);
  vLayout->addWidget(mDetectorWidget);
  vLayout->addWidget(new QLabel("Learn", section));

  mClassifyWidget = new QListWidget(section);
  mClassifyWidget->setFixedHeight(100);
  mClassifyWidget->setSelectionMode(QAbstractItemView::MultiSelection);
  mClassifyWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  mClassifyWidget->setViewMode(QListWidget::IconMode);
  mClassifyWidget->setIconSize(QSize(100, 100));
  mClassifyWidget->setResizeMode(QListWidget::Adjust);

  vLayout->addWidget(mClassifyWidget);
  section->setContentLayout(*vLayout);
  setAttribute(Qt::WA_DeleteOnClose);
  setContextMenuPolicy(Qt::ActionsContextMenu);
  uiSpinBoxThresholdFAST->setValue(mSettings->value("fastRT/threshold", 25).toInt());
  uiPushButtonNonMaxFAST->setChecked(mSettings->value("fastRT/nonMaxSuppression", true).toBool());

  actAbnormal = std::make_unique<QAction>(tr("&Abnormal"), this);
  actNormal = std::make_unique<QAction>(tr("&Normal"), this);
  actClear = std::make_unique<QAction>(tr("&Clear"), this);

  actLoadKnowledge = std::make_unique<QAction>(tr("&Load Knowledge"), this);
  actSaveKnowledge = std::make_unique<QAction>(tr("&Save Knowledge"), this);
  actDeleteKnowledge = std::make_unique<QAction>(tr("&Delete Knowledge"), this);
  actTestKnowledge = std::make_unique<QAction>(tr("&Test Knowledge"), this);

  QObject::connect(actNormal.get(), &QAction::triggered, this, &WindowFastRealTime::learnNormal);
  QObject::connect(actAbnormal.get(), &QAction::triggered, this, &WindowFastRealTime::learnAbnormal);
  QObject::connect(actClear.get(), &QAction::triggered, this, &WindowFastRealTime::clearListWidget);
  QObject::connect(actLoadKnowledge.get(), &QAction::triggered, this, &WindowFastRealTime::loadKnowledges);
  QObject::connect(actSaveKnowledge.get(), &QAction::triggered, this, &WindowFastRealTime::saveKnowledges);
  QObject::connect(actDeleteKnowledge.get(), &QAction::triggered, this, &WindowFastRealTime::deleteKnowledge);
  QObject::connect(actTestKnowledge.get(), &QAction::triggered, this, &WindowFastRealTime::testKnowledge);
  QObject::connect(mDetectorWidget, &QListWidget::customContextMenuRequested, this,
                   &WindowFastRealTime::showDetectionContextMenu);
  QObject::connect(mClassifyWidget, &QListWidget::customContextMenuRequested, this,
                   &WindowFastRealTime::showClassifyContextMenu);
  QObject::connect(uiPushButtonNonMaxFAST, &QPushButton::toggled, this, &WindowFastRealTime::saveFastParams);
  QObject::connect(uiSpinBoxThresholdFAST, &QSpinBox::editingFinished, this, &WindowFastRealTime::saveFastParams);
  QObject::connect(uiPushButtonResetFAST, &QAbstractButton::clicked, this, &WindowFastRealTime::resetUI);
  QObject::connect(uiPushButtonDetect, &QAbstractButton::clicked, this, &WindowFastRealTime::detect);
  QObject::connect(uiPushButtonCancel, &QAbstractButton::clicked, this, &WindowFastRealTime::close);

  m_pen.setWidth(5);
  std::string url = mSettings->value("rtsp/url1").toString().toStdString();
  mCamera->open(url);
  if (mCamera->isOpened()) {
    mTimer->start(33); // 30fps
    QObject::connect(mTimer.get(), &QTimer::timeout, this, &WindowFastRealTime::compute);
    uiPushButtonDetect->setEnabled(true);
  } else {
    uiLabelRealTime->setText("There is some problem with the cam.\nCannot get images.");
  }
  show();
}

void WindowFastRealTime::saveKnowledges() { m_nm_classifier->neurons_to_file(); }

void WindowFastRealTime::loadKnowledges() {
  // clear widget first
  while (mClassifyWidget->count() > 0) {
    QListWidgetItem *item = mClassifyWidget->takeItem(0);
    delete item;
  }
  int cnt = m_nm_classifier->file_to_neurons();
  if (cnt > 0)
    m_nm_classifier->read_neurons(mClassifyWidget);
}

void WindowFastRealTime::testKnowledge() {
  QListWidgetItem *currentItem = mClassifyWidget->currentItem();
  QIcon icon = currentItem->icon();
  cv::Mat feature = QPixmap2Mat(icon.pixmap(QSize(100, 100)), true);
  m_nm_classifier->classify(feature);
}

void WindowFastRealTime::deleteKnowledge() {
  QListWidgetItem *currentItem = mClassifyWidget->currentItem();
  if (currentItem) {
    QIcon icon = currentItem->icon();
    cv::Mat feature = QPixmap2Mat(icon.pixmap(QSize(100, 100)), true);
    if (m_nm_classifier->deleteKnowledge(feature) < nm_classifier::UNKNOWN) {
      mClassifyWidget->removeItemWidget(currentItem);
      delete currentItem;
    }
  }
}

void WindowFastRealTime::showDetectionContextMenu(const QPoint &pos) {
  QPoint globalPos = mDetectorWidget->mapToGlobal(pos);
  QMenu menu(this);
  menu.addAction(actAbnormal.get());
  menu.addAction(actNormal.get());
  menu.addAction(actClear.get());
  menu.exec(globalPos);
}

void WindowFastRealTime::showClassifyContextMenu(const QPoint &pos) {
  QPoint globalPos = mDetectorWidget->mapToGlobal(pos);
  QMenu menu(this);
  menu.addAction(actLoadKnowledge.get());
  menu.addAction(actSaveKnowledge.get());
  menu.addAction(actDeleteKnowledge.get());
  menu.addAction(actTestKnowledge.get());
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
  if (mCamera)
    (void)(mCamera.release()); // cast to void to disable clang-tdy warning - no use of return value
  QWidget::close();
}

void WindowFastRealTime::closeEvent(QCloseEvent *closeEvent) {
  close();
  QDialog::closeEvent(closeEvent);
}

void WindowFastRealTime::saveFastParams() {
  mSettings->setValue("fastRT/threshol", uiSpinBoxThresholdFAST->value());
  mSettings->setValue("fastRT/nonMaxSuppression", uiPushButtonNonMaxFAST->isChecked());
}

void WindowFastRealTime::resetUI() {
  uiSpinBoxThresholdFAST->setValue(25);
  uiPushButtonNonMaxFAST->setChecked(true);
  uiLabelRealTime->resetUI();
  saveFastParams();
}

void WindowFastRealTime::learnNormal() {}

void WindowFastRealTime::learnAbnormal() {

  QListWidgetItem *currentItem = mDetectorWidget->currentItem();
  QIcon icon = currentItem->icon();
  cv::Mat feature = QPixmap2Mat(icon.pixmap(QSize(100, 100)), true);
  m_nm_classifier->learn(feature);
  mClassifyWidget->addItem(new QListWidgetItem(*currentItem));
}

void WindowFastRealTime::clearListWidget() {
  QList<QListWidgetItem *> selected = mDetectorWidget->selectedItems();
  int size = selected.size();
  if (size > 0) {
    for (auto &item : selected) {
      mDetectorWidget->removeItemWidget(item); 
      // https://stackoverflow.com/questions/25417348/remove-selected-items-from-listwidget
      delete item;
    }
    mDetectorWidget->clearSelection();
  } else {
    mDetectorWidget->clear();
  }
}

void WindowFastRealTime::compute() {
  cv::Mat imgRT;
  mCamera->read(imgRT);
  if (mDetecting) {
    if (!imgRT.empty()) {
      cv::Mat gray;
      cv::cvtColor(imgRT, gray, cv::COLOR_BGR2GRAY);

      std::vector<cv::Rect> motions;
      m_detector.detect_candidate(gray, motions);
      mPixmap = QPixmap::fromImage(
        QImage(imgRT.data, imgRT.cols, imgRT.rows, static_cast<int>(imgRT.step), QImage::Format_RGB888));
      mPainter->begin(&mPixmap);

      for (const auto &motion : motions) {
        QRect motionRect(motion.x, motion.y, motion.width, motion.height);
        const auto &band = std::find_if(uiLabelRealTime->getBandList().begin(), uiLabelRealTime->getBandList().end(),
                                        [&motionRect, this](const auto &b) {
                                          QRect qRect = b->geometry().normalized();
                                          return motionRect.intersects(qRect);
                                        });

        if (band != uiLabelRealTime->getBandList().end()) {
          QPixmap pix = mPixmap.copy(motionRect).scaled(QSize(100, 100), Qt::KeepAspectRatio);
          cv::Mat mat = QPixmap2Mat(pix, true);
          QString fileName = QString("%1.png").arg(QDateTime::currentDateTime().toString("MMdd_hhmmss"));
          pix.toImage().save(QString("backup/%1").arg(fileName));
          mDetectorWidget->addItem(new QListWidgetItem(QIcon(pix), fileName));

          if (m_nm_classifier->classify(mat) < nm_classifier::UNKNOWN) {
            m_pen = QColor::fromRgb(255, 0, 0);
          } else {
            m_pen = QColor::fromRgb(0, 0, 255);
          }
        } else {
          m_pen = QColor::fromRgb(0, 255, 0);
        }
        m_pen.setWidth(5);
        mPainter->setPen(m_pen);
        mPainter->drawRect(motionRect);
      }
      mPainter->end();
      uiLabelRealTime->setPixmap(mPixmap);
      uiLabelTime->setText("Detecting Time: " + QString("%1").arg(m_detector.get_detection_time()));
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
