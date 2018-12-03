#pragma once

#include "ui_windowCaptureWebcam.h"
#include "windowImage.h"
#include "windowMain.h"
#include <opencv2/opencv.hpp>

class
    WindowMain; // http://stackoverflow.com/questions/2133250/does-not-name-a-type-error-in-c

class WindowCaptureWebcam : public QDialog, private Ui::windowCaptureWebcam {
  Q_OBJECT
public:
  explicit WindowCaptureWebcam(WindowMain * main);
  void closeEvent(QCloseEvent *) override;

  WindowMain *mWindowMain = nullptr;

private:
  QTimer *mTimer = nullptr;
  cv::VideoCapture mCamera;
  cv::Mat mImageRT;

private slots:
  void capture();
  void ok();
  void compute();
  void close();
};
