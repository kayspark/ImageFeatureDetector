/*
 * 2018(C) kayspark , 2010-2015 (C) Antonio Redondo
 *
 * https://github.com/kayspark/ImageFeatureDetector
 *
 * Code under the terms of the GNU General Public License v3.
 *
 */

#pragma once

#include "ui_windowFastRealTime.h"
#include "windowMain.h"
#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>

class
    WindowMain; // http://stackoverflow.com/questions/2133250/does-not-name-a-type-error-in-c

class WindowFastRealTime : public QDialog, Ui::windowFastRealTime {
  Q_OBJECT
public:
  explicit WindowFastRealTime(WindowMain *wmain);
  void closeEvent(QCloseEvent *) override;

private:
  QSettings *mSettings = nullptr;
  std::unique_ptr<QLocale> mLocale;
  cv::VideoCapture mCamera;
  std::unique_ptr<QTimer> mTimer;
  QPixmap mPixmap;
  std::unique_ptr<QPainter> mPainter;
  cv::Mat mImageRT;
  std::vector<cv::KeyPoint> mKeypoints;
  bool mDetecting;
  double mTime;

private slots:
  void detect();
  void compute();
  void close();
  void saveFastParams();
  void resetFastParams();
};
