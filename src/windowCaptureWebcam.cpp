
#include "windowCaptureWebcam.h"
#include "windowMain.h"
#include <QDialog>

WindowCaptureWebcam::WindowCaptureWebcam(WindowMain *main)
    : QDialog(main, Qt::Dialog), mWindowMain(main),
      mCamera(cv::VideoCapture(cv::CAP_ANY)) {
  setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);

  QObject::connect(uiPushButtonCapture, &QAbstractButton::clicked, this,
                   &WindowCaptureWebcam::capture);
  QObject::connect(uiPushButtonOK, &QAbstractButton::clicked, this,
                   &WindowCaptureWebcam::ok);
  QObject::connect(uiPushButtonCancel, &QAbstractButton::clicked, this,
                   &WindowCaptureWebcam::close);
  if (mCamera.isOpened()) {
    mTimer = std::make_unique<QTimer>();
    mTimer->start(40); // 25fps
    QObject::connect(mTimer.get(), &QTimer::timeout, this,
                     &WindowCaptureWebcam::compute);
    uiPushButtonCapture->setEnabled(true);
    // 		qDebug() << "Frame format: " << mCamera.get(CV_CAP_PROP_FORMAT);
    // 		qDebug() << "Frame count: " <<
    // mCamera.get(CV_CAP_PROP_FRAME_COUNT); 		qDebug() << "Frame mode: "
    // <<
    // mCamera.get(CV_CAP_PROP_MODE); 		qDebug() << "Frame rate: " <<
    // mCamera.get(CV_CAP_PROP_FPS); 		qDebug() << "Frame width: " <<
    // mCamera.get(CV_CAP_PROP_FRAME_WIDTH); 		qDebug() << "Frame height: "
    // << mCamera.get(CV_CAP_PROP_FRAME_HEIGHT);
  } else {
    uiLabelRealTime->setText(
        "There is some problem with the cam.\nCannot get images.");
    uiLabelCaptured->setText("Please check camera");
  }

  show();
}

void WindowCaptureWebcam::capture() {
  uiLabelCaptured->setPixmap(QPixmap::fromImage(
      QImage(mImageRT.data, mImageRT.cols, mImageRT.rows,
             static_cast<int>(mImageRT.step), QImage::Format_RGB888)));
  uiPushButtonOK->setEnabled(true);
  // 	qDebug() << "Frame camptured?";
  // 	qDebug() << "Frame width: " << mImageRT.cols;
  // 	qDebug() << "Frame height: " << mImageRT.rows;
  // 	qDebug() << "Frame height: " << mImageRT.step;
  // 	qDebug() << "Image type: " << mImageRT.type();
  // 	qDebug() << "CV_8UC3: " << (mImageRT.type() == CV_8UC3);
  // 	qDebug() << "CV_8UC4: " << (mImageRT.type() == CV_8UC4);
  // 	qDebug() << "CV_32FC1: " << (mImageRT.type() == CV_32FC1);
}

void WindowCaptureWebcam::ok() {
  mWindowMain->showWindowImage(new WindowImage(
      std::make_shared<QImage>(uiLabelCaptured->pixmap()->toImage()),
      tr("WebCam Captured Image %1").arg(++mWindowMain->mCapturedWebcamImages),
      WindowImage::fromWebcam));
  close();
}

void WindowCaptureWebcam::close() {
  if (mTimer)
    mTimer->stop();
  if (mCamera.isOpened())
    mCamera.release();
  QWidget::close();
}

void WindowCaptureWebcam::closeEvent(QCloseEvent *closeEvent) {
  close();
  QDialog::closeEvent(closeEvent);
}

void WindowCaptureWebcam::compute() {
  mCamera >> mImageRT;
  cvtColor(mImageRT, mImageRT, cv::COLOR_BGR2RGB);
  uiLabelRealTime->setPixmap(QPixmap::fromImage(
      QImage(mImageRT.data, mImageRT.cols, mImageRT.rows,
             static_cast<int>(mImageRT.step),
             QImage::Format_RGB888))); // With RGB32 doesn't work
  // 	uiLabelRealTime->setPixmap(QPixmap::fromImage(QImage(mImageRT.data,
  // mImageRT.cols, mImageRT.rows, mImageRT.step,
  // QImage::Format_RGB888).rgbSwapped()));
}
