/*
 * 2010-2015 (C) Antonio Redondo
 * http://antonioredondo.com
 * https://github.com/AntonioRedondo/ImageFeatureDetector
 *
 * Code under the terms of the GNU General Public License v3.
 *
 */

#pragma once

#include "ui_windowFastRealTime.h"
#include "windowMain.h"
#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>


class WindowFastRealTime : public QDialog, Ui::windowFastRealTime {
  Q_OBJECT
public:
  explicit WindowFastRealTime(WindowMain *);
  void closeEvent(QCloseEvent *) override;

private:
  QSettings *mSettings = nullptr;
  QLocale *mLocale = nullptr;
  cv::VideoCapture mCamera;
  QTimer *mTimer = nullptr;
  QPixmap mPixmap;
  QPainter *mPainter = nullptr;
  cv::Mat mImageRT;
  std::vector<cv::KeyPoint> mKeypoints;
  bool mDetecting;
  float mTime;

private slots:
  void detect();
  void compute();
  void close();
  void saveFastParams();
  void resetFastParams();
};

