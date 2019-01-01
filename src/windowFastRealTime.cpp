/*
 * 2018(C) kayspark , 2010-2015 (C) Antonio Redondo
 *
 * https://github.com/kayspark/ImageFeatureDetector
 *
 * Code under the terms of the GNU General Public License v3.
 *
 */

#include "windowFastRealTime.h"

WindowFastRealTime::WindowFastRealTime(WindowMain *wmain)
    : QDialog(wmain, Qt::Dialog), mCamera(vlc_capture(960, 544)),
      mTimer(std::make_unique<QTimer>()), mDetecting(false), mTime(0.0),
      _data_file("./dataset/cascade.xml"), _tracking_algorithm("CSRT"),
      _predator(nm_detector(this->_data_file, this->_tracking_algorithm)),
      mSettings(wmain->getMSettings()),
      mLocale(std::make_unique<QLocale>(QLocale::English)),
      mPainter(std::make_unique<QPainter>()) {
  setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);
  uiSpinBoxThresholdFAST->setValue(
      mSettings->value("fastRT/threshold", 25).toInt());
  uiPushButtonNonMaxFAST->setChecked(
      mSettings->value("fastRT/nonMaxSuppression", true).toBool());

  QObject::connect(uiPushButtonNonMaxFAST, &QPushButton::toggled, this,
                   &WindowFastRealTime::saveFastParams);
  QObject::connect(uiSpinBoxThresholdFAST, &QSpinBox::editingFinished, this,
                   &WindowFastRealTime::saveFastParams);
  QObject::connect(uiPushButtonResetFAST, &QAbstractButton::clicked, this,
                   &WindowFastRealTime::resetFastParams);
  QObject::connect(uiPushButtonDetect, &QAbstractButton::clicked, this,
                   &WindowFastRealTime::detect);
  QObject::connect(uiPushButtonCancel, &QAbstractButton::clicked, this,
                   &WindowFastRealTime::close);
  std::string url = mSettings->value("rtsp/url1").toString().toStdString();
  mCamera.open(url);
  if (mCamera.isOpened()) {
    mTimer->start(40); // 25fps
    QObject::connect(mTimer.get(), &QTimer::timeout, this,
                     &WindowFastRealTime::compute);
    uiPushButtonDetect->setEnabled(true);
  } else {
    uiLabelRealTime->setText(
        "There is some problem with the cam.\nCannot get images.");
  }

  show();
}

void WindowFastRealTime::detect() {
  if (!mDetecting) {
    uiPushButtonDetect->setIcon(QIcon("icons/media-playback-stop.svg"));
    uiPushButtonDetect->setText("Stop Detecting");
  } else {
    uiPushButtonDetect->setIcon(QIcon("icons/media-playback-start.svg"));
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
  if (_rubberBand == nullptr)
    _rubberBand = std::make_unique<QRubberBand>(QRubberBand::Rectangle, this);
  // setCursor(Qt::ClosedHandCursor);
}

void WindowFastRealTime::mouseMoveEvent(QMouseEvent *event) {
  QPoint myPos = event->pos();
  //  int hValue = horizontalScrollBar()->value();
  //  int vValue = verticalScrollBar()->value();
  // horizontalScrollBar()->setValue(hValue + (mLastPoint.x() - myPos.x()));
  // verticalScrollBar()->setValue(vValue + (mLastPoint.y() - myPos.y()));
  _rubberBand->setGeometry(QRect(mLastPoint, event->pos()).normalized());
  _rubberBand->show();
  band_avaiable = false;
  QToolTip::showText(event->globalPos(),
                     QString("%1,%2")
                         .arg(_rubberBand->size().width())
                         .arg(_rubberBand->size().height()),
                     this);
  // mLastPoint = myPos;
}

void WindowFastRealTime::mouseReleaseEvent(QMouseEvent * /*event*/) {
  // unsetCursor();
  band_avaiable = true;
  //_rubberBand->hide();
  // determine selection.. QRect::contains..
}
void WindowFastRealTime::saveFastParams() {
  mSettings->setValue("fastRT/threshold", uiSpinBoxThresholdFAST->value());
  mSettings->setValue("fastRT/nonMaxSuppression",
                      uiPushButtonNonMaxFAST->isChecked());
}

void WindowFastRealTime::resetFastParams() {
  uiSpinBoxThresholdFAST->setValue(25);
  uiPushButtonNonMaxFAST->setChecked(true);
  saveFastParams();
}

void WindowFastRealTime::compute() {
  mCamera.read(mImageRT);
  if (mDetecting) {
    if (!mImageRT.empty()) {
      cv::Mat gray;
      cv::cvtColor(mImageRT, gray, cv::COLOR_RGB2GRAY);
      _predator.update_tracker(gray);
      mPixmap = QPixmap::fromImage(
          QImage(mImageRT.data, mImageRT.cols, mImageRT.rows,
                 static_cast<int>(mImageRT.step), QImage::Format_RGB888));
      cv::Rect2d object = _predator.get_detected();
      uiLabelTime->setText("Detecting Time: " +
          QString("%1").arg(_predator.get_detection_time()));
      uiLabelKeypoints->setText("Key points: -" + QString("%1").arg(1));
      mPainter->begin(&mPixmap);
      QPen pen(QColor::fromRgb(255, 0, 0));
      pen.setWidth(2);
      mPainter->setPen(pen);
      mPainter->setRenderHint(QPainter::Antialiasing);
      mPainter->drawRect(object.x, object.y, object.width, object.height);
      mPainter->end();
      uiLabelRealTime->setPixmap(mPixmap);
    }
  } else {
    mPixmap = QPixmap::fromImage(
        QImage(mImageRT.data, mImageRT.cols, mImageRT.rows,
               static_cast<int>(mImageRT.step), QImage::Format_RGB888));
    uiLabelRealTime->setPixmap(mPixmap);
    uiLabelTime->setText("Detecting Time: -");
    uiLabelKeypoints->setText("Key points: -");
  }
}
