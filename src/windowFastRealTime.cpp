/*
 * 2010-2015 (C) Antonio Redondo
 * http://antonioredondo.com
 * https://github.com/AntonioRedondo/ImageFeatureDetector
 *
 * Code under the terms of the GNU General Public License v3.
 *
 */

#include "windowFastRealTime.h"

WindowFastRealTime::WindowFastRealTime(WindowMain *windowMain)
    :  QDialog(windowMain, Qt::Dialog),
      mCamera(cv::VideoCapture(0)), mTimer(new QTimer()), mDetecting(false)  {
  setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);
  mSettings =
      new QSettings("imageFeatureDetectorSettings.ini", QSettings::IniFormat);
  mLocale = new QLocale(QLocale::English);

  uiSpinBoxThresholdFAST->setValue(
      mSettings->value("fastRT/threshold", 25).toInt());
  uiPushButtonNonMaxFAST->setChecked(
      mSettings->value("fastRT/nonMaxSuppression", true).toBool());

  connect(uiPushButtonNonMaxFAST, &QPushButton::toggled, this,
          &WindowFastRealTime::saveFastParams);
  connect(uiSpinBoxThresholdFAST, &QSpinBox::editingFinished, this,
          &WindowFastRealTime::saveFastParams);
  connect(uiPushButtonResetFAST, &QAbstractButton::clicked, this,
          &WindowFastRealTime::resetFastParams);
  connect(uiPushButtonDetect, &QAbstractButton::clicked, this,
          &WindowFastRealTime::detect);
  connect(uiPushButtonCancel, &QAbstractButton::clicked, this,
          &WindowFastRealTime::close);

  if (mCamera.isOpened()) {
    mPainter = new QPainter();
    mTimer->start(40); // 25fps
    connect(mTimer, &QTimer::timeout, this, &WindowFastRealTime::compute);
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
  mCamera >> mImageRT;
  if (mDetecting) {
          cv::Mat mImageGrey(mImageRT.rows, mImageRT.cols, CV_8UC1);
          cv::cvtColor(mImageRT, mImageGrey, CV_RGB2GRAY);
    mTime = cv::getTickCount();
    FAST(mImageGrey, mKeypoints,
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
    for (auto &keyPoint : mKeypoints)
      mPainter->drawEllipse((int)keyPoint.pt.x, (int)keyPoint.pt.y, 4, 4);
    mPainter->end();
    uiLabelRealTime->setPixmap(mPixmap);
  } else {
    uiLabelRealTime->setPixmap(QPixmap::fromImage(
        QImage(mImageRT.data, mImageRT.cols, mImageRT.rows,
               static_cast<int>(mImageRT.step), QImage::Format_RGB888)
            .rgbSwapped()));
    uiLabelTime->setText("Detecting Time: -");
    uiLabelKeypoints->setText("Key points: -");
  }
}
