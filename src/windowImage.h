/*
* 2010-2015 (C) Antonio Redondo
* http://antonioredondo.com
* https://github.com/AntonioRedondo/ImageFeatureDetector
*
* Code under the terms of the GNU General Public License v3.
*
*/

#ifndef WINDOWIMAGE_H
#define WINDOWIMAGE_H

#include <QtWidgets>
#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>
#include "ui_windowImage.h"
//#include "ui_windowImage.h"

class WindowImage : public QScrollArea, Ui::windowImage {
Q_OBJECT
public:
  WindowImage(QImage *image, const QString& windowTitle, int windowType = normal);
  void zoomIn();
  void zoomOut();
  void zoomOriginal();
  void zoomBestFit();
  void resetImage();
  void applyHarris(int sobelApertureSize,
                   int harrisApertureSize,
                   double kValue,
                   int threshold,
                   bool showProcessedImage);
  void applyFast(int threshold, bool nonMaxSuppression);
  void applySift(double threshold, double edgeThreshold, int nOctaves, int nOctaveLayers, bool showOrientation);
  void applySurf(double threshold, int nOctaves, int nOctaveLayers, bool showOrientation);

  enum windowType { normal = 0, duplicated = 1, fromWebcam = 2, do4 = 3 };
  enum featureType { none = 0, harris = 1, fast = 2, sift = 3, surf = 4 };
  QImage *mImage = nullptr;
  QPixmap mPixmap;
  QString mImageZoom;
  QString mImageTime;
  QString mImageKeypoints;
  QString mImageDimensions;
  QString mImageSize;
  QString mWindowTitle;
  QString mUid;
  QString mOriginalUid;
  int mWindowType, mFeatureType, mImageN;
  float mCurrentFactor;

protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void showProcessedImage(cv::Mat processedImage);
  void scaleImage();
  void adjustScrollBar(QScrollBar *scrollBar);
  QImage convertMat2QImage(const cv::Mat_<double> &src);

  QPixmap mPixmapOriginal;
  QSize mOriginalSize;
  QPoint mLastPoint;
  QLocale *mLocale = nullptr;
  QPainter *mPainter = nullptr;
  bool mModified;
  int mOriginalWidth, mOiginalHeight;
  float mScaleFactorAbove100, mScaleFactorUnder100, mFactorIncrement;
};

#endif
