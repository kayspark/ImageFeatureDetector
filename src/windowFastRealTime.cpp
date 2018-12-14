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
    : QDialog(wmain, Qt::Dialog), mCamera(vlc_capture("RV24", 800, 600)),
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

  mCamera.open("rtsp://canon:adminadmin@223.171.38.5:80/rtpstream/config5=r");
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
      cv::cvtColor(mImageRT, gray, cv::COLOR_BGR2GRAY);
      _predator.update_tracker(gray, mImageRT);
      cvtColor(mImageRT, mImageRT, cv::COLOR_BGR2RGB);
      mPixmap = QPixmap::fromImage(
          QImage(mImageRT.data, mImageRT.cols, mImageRT.rows,
                 static_cast<int>(mImageRT.step), QImage::Format_RGB888)
              .rgbSwapped());
    }

/*     cv::Mat imgGray(mImageRT.rows, mImageRT.cols, CV_8UC1);
    cv::cvtColor(mImageRT, imgGray, cv::COLOR_RGB2GRAY);
    mTime = cv::getTickCount();
    FAST(imgGray, mKeypoints,
         mSettings->value("fastRT/threshold", true).toInt(),
         mSettings->value("fastRT/nonMaxSuppression", true).toBool());
    uiLabelTime->setText(
        QString("Detecting Time: ")
            .append(mLocale
                        ->toString(((cv::getTickCount() - mTime) /
                                    (cv::getTickFrequency() * 1000)),
                                   'f', 2)
                        .append(" ms")));
    uiLabelKeypoints->setText(
        QString("Key points: ")
            .append(mLocale->toString((float)mKeypoints.size(), 'f', 0)));

    mPixmap = QPixmap::fromImage(
        QImage(mImageRT.data, mImageRT.cols, mImageRT.rows,
               static_cast<int>(mImageRT.step), QImage::Format_RGB888)
            .rgbSwapped());
    mPainter->begin(&mPixmap);
    QPen pen(QColor::fromRgb(255, 0, 0));
    pen.setWidth(2);
    mPainter->setPen(pen);
    mPainter->setRenderHint(QPainter::Antialiasing);
    for (const auto &keyPoint : mKeypoints)
      mPainter->drawEllipse((int)keyPoint.pt.x, (int)keyPoint.pt.y, 4, 4);
    mPainter->end();
    uiLabelRealTime->setPixmap(mPixmap);
 */  } else {
  uiLabelRealTime->setPixmap(QPixmap::fromImage(
      QImage(mImageRT.data, mImageRT.cols, mImageRT.rows,
             static_cast<int>(mImageRT.step), QImage::Format_RGB888)
          .rgbSwapped()));
  uiLabelTime->setText("Detecting Time: -");
  uiLabelKeypoints->setText("Key points: -");
}
}
