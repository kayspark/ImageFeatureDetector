#pragma once

#include "nm_classifier.hpp"
#include "nm_detector.hpp"
#include "ui_windowCaptureWebcam.h"
#include "windowImage.h"
#include "windowMain.h"
#include <opencv2/opencv.hpp>
class WindowMain; // http://stackoverflow.com/questions/2133250/does-not-name-a-type-error-in-c

class WindowCaptureWebcam
    : public QDialog
    , Ui::windowCaptureWebcam {
  Q_OBJECT
public:
  explicit WindowCaptureWebcam(WindowMain *main);
  void closeEvent(QCloseEvent *closeEvent) override;

  WindowMain *mWindowMain = nullptr;

private:
  std::unique_ptr<QTimer> mTimer;
  cv::VideoCapture mCamera;
  cv::Mat _imgRT;
  std::string _data_file;
  std::string _tracking_algorithm;
  std::unique_ptr<QPainter> mPainter;
  std::vector<std::shared_ptr<QRubberBand>> _bandList;
  std::shared_ptr<QRubberBand> _rubberBand;
  bool _mouse_pressed = false;
  QPoint mLastPoint;
  std::unique_ptr<nm_classifier> _nm_classifier;
  std::unique_ptr<QAction> actNormal;
  std::unique_ptr<QAction> actAbnormal;
  std::unique_ptr<QAction> actClear;
  QPen _penG;
  QPen _penR;
  QPen _penB;

protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

protected:
  nm_detector _predator;

private slots:
  void capture();
  void ok();
  void compute();
  void close();
};
