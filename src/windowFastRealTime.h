/*
 * 2010-2015 (C) Antonio Redondo
 * http://antonioredondo.com
 * https://github.com/AntonioRedondo/ImageFeatureDetector
 *
 * Code under the terms of the GNU General Public License v3.
 *
 */

#ifndef WINDOWFASTREALTIME_H
#define WINDOWFASTREALTIME_H

#include "ui_windowFastRealTime.h"
#include "windowMain.h"
#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
using namespace xfeatures2d;

class WindowFastRealTime : public QDialog, Ui::windowFastRealTime {
  Q_OBJECT
public:
  explicit WindowFastRealTime(WindowMain *);
  void closeEvent(QCloseEvent *) override;

private:
  QSettings *mSettings;
  QLocale *mLocale;
  VideoCapture mCamera;
  QTimer *mTimer;
  QPixmap mPixmap;
  QPainter *mPainter;
  Mat mImageRT;
  vector<KeyPoint> mKeypoints;
  bool mDetecting;
  float mTime;

private slots:
  void detect();
  void compute();
  void close();
  void saveFastParams();
  void resetFastParams();
};

#endif
