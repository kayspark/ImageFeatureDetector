/*
 * 2018(C) kayspark , 2010-2015 (C) Antonio Redondo
 *
 * https://github.com/kayspark/ImageFeatureDetector
 *
 * Code under the terms of the GNU General Public License v3.
 *
 */

#pragma once

#include "nm_classifier.hpp"
#include "nm_detector.hpp"
#include "ui_windowFastRealTime.h"
#include "vlccap.hpp"
#include "windowMain.h"
#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>
class WindowMain; // http://stackoverflow.com/questions/2133250/does-not-name-a-type-error-in-c

class WindowFastRealTime
    : public QDialog
    , Ui::windowFastRealTime {
  Q_OBJECT
public:
  explicit WindowFastRealTime(WindowMain *wmain);
  void closeEvent(QCloseEvent *closeEvent) override;

private:
  QSettings *mSettings = nullptr;
  std::unique_ptr<QLocale> mLocale;
  vlc_capture mCamera;
  // cv::VideoCapture mCamera;
  std::unique_ptr<QTimer> mTimer;
  QPixmap mPixmap;
  std::unique_ptr<QPainter> mPainter;
  std::vector<cv::KeyPoint> mKeypoints;
  bool mDetecting;
  double mTime;
  std::string _data_file;
  std::string _tracking_algorithm;
  std::vector<std::shared_ptr<QRubberBand>> _bandList;
  std::shared_ptr<QRubberBand> _rubberBand;
  bool _mouse_pressed = false;
  QPoint mLastPoint;
  std::unique_ptr<nm_classifier> _nm_classifier;
  std::unique_ptr<QAction> actNormal;
  std::unique_ptr<QAction> actAbnormal;
  std::unique_ptr<QAction> actClear;

protected:
  nm_detector _predator;

protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void showContextMenu(const QPoint &pos);
private slots:
  void detect();
  void compute();
  void close();
  void saveFastParams();
  void resetUI();
  void learnNormal();
  void learnAbnormal();
  void clearListWidget();
};
