/*
 * 2018(C) kayspark , 2010-2015 (C) Antonio Redondo
 *
 * https://github.com/kayspark/ImageFeatureDetector
 *
 * Code under the terms of the GNU General Public License v3.
 *
 */

#pragma once
#include "nm_detector.h"
#include "ui_windowImage.h"
#include "vlccap.h"
#include <QtWidgets>
#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

class WindowImage
    : public QScrollArea
    , Ui::windowImage {
  Q_OBJECT
public:
  WindowImage(const QString &fileName, QString windowTitle, int windowType = normal);
  WindowImage(std::shared_ptr<QImage> image, QString windowTitle, int windowType = normal);
  void zoomIn();
  void zoomOut();
  void zoomOriginal();
  void zoomBestFit();
  void resetImage();
  void compute();
  void applyHarris(int sobelApertureSize, int harrisApertureSize, double kValue, int threshold,
                   bool showProcessedImage);
  void applyFast(int threshold, bool nonMaxSuppression);
  void applySift(double threshold, double edgeThreshold, int nOctaves, int nOctaveLayers, bool showOrientation);
  void applySurf(double threshold, int nOctaves, int nOctaveLayers, bool showOrientation);

  enum windowType { normal = 0, duplicated = 1, fromWebcam = 2, do4 = 3 };
  enum featureType { none = 0, harris = 1, fast = 2, sift = 3, surf = 4 };
  std::shared_ptr<QImage> _image;
  cv::Mat _imgRT;
  QPixmap mPixmap;
  // vlc_capture _capture;
  cv::VideoCapture _capture;
  std::unique_ptr<QTimer> timer;
  QString mImageZoom;
  QString mImageTime;
  QString mImageKeypoints;
  QString mImageDimensions;
  QString mImageSize;
  QString mWindowTitle;
  QString mUid;
  QString mOriginalUid;
  std::string _data_file;
  std::string _tracking_algorithm;
  nm_detector _predator;
  int mWindowType, mFeatureType, mImageN;
  double mCurrentFactor;
  std::unique_ptr<QRubberBand> _rubberBand;
  bool band_avaiable = false;

protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
  void showProcessedImage(cv::Mat &processedImage);
  void scaleImage();
  void adjustScrollBar(QScrollBar *scrollBar);
  QImage convertMat2QImage(const cv::Mat &src);

  QPixmap mPixmapOriginal;
  QSize mOriginalSize;
  QPoint mLastPoint;
  std::unique_ptr<QLocale> mLocale;
  std::unique_ptr<QPainter> mPainter;
  bool mModified;
  int mOriginalWidth, mOriginalHeight;
  double mScaleFactorAbove100, mScaleFactorUnder100, mFactorIncrement;
};
