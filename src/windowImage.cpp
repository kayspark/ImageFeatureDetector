#include <utility>

/*
 * 2010-2015 (C) Antonio Redondo
 * http://antonioredondo.com
 * https://github.com/AntonioRedondo/ImageFeatureDetector
 *
 * Code under the terms of the GNU General Public License v3.
 *
 */

#include "windowImage.h"

WindowImage::WindowImage(const QString &fileName, QString windowTitle,
                         int windowType)
    : mCamera(cv::VideoCapture(fileName.toStdString())),
      mWindowTitle(std::move(windowTitle)), mWindowType(windowType), mImageN(0),
      mOriginalWidth(0), mOriginalHeight(0), mModified(false), mFeatureType(0),
      mPainter(std::make_unique<QPainter>()),
      mLocale(std::make_unique<QLocale>(QLocale::English)) {
  setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);

  setWindowTitle(mWindowTitle);

  uiScrollAreaWidgetContents->setSizePolicy(
      QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

  if (mCamera.isOpened()) {
    // setup default values
    compute();
    float sizeInKiB = mImage->byteCount() / (float) 1024;
    if (sizeInKiB > 1024)
      mImageSize =
          mLocale->toString(sizeInKiB / (float) 1024, 'f', 2).append(" MiB");
    else
      mImageSize = mLocale->toString(sizeInKiB, 'f', 2).append(" KiB");
    // For async refresh
    timer = std::make_unique<QTimer>();
    timer->start(40); // 25fps
    QObject::connect(timer.get(), &QTimer::timeout, this,
                     &WindowImage::compute);
  } else {
    uiLabelImage->setText(
        "There is some problem with the cam.\nCannot get images.");
    uiLabelImage->setText("Please check camera");
  }

  mScaleFactorAbove100 = 0.5;
  mScaleFactorUnder100 = 0.25;
  mFactorIncrement = 0;
  mCurrentFactor = 1.0;
}
WindowImage::WindowImage(std::shared_ptr<QImage> image, QString windowTitle,
                         int windowType)
    : mImage(std::move(image)), mWindowTitle(std::move(windowTitle)),
      mWindowType(windowType), mImageN(0), mModified(false), mFeatureType(0),
      mPainter(std::make_unique<QPainter>()),
      mLocale(std::make_unique<QLocale>(QLocale::English)) {
  setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);

  setWindowTitle(mWindowTitle);

  uiScrollAreaWidgetContents->setSizePolicy(
      QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

  mPixmap = QPixmap::fromImage(*mImage);
  mPixmapOriginal = mPixmap;
  uiLabelImage->setPixmap(mPixmap);

  mScaleFactorAbove100 = 0.5;
  mScaleFactorUnder100 = 0.25;
  mFactorIncrement = 0;
  mCurrentFactor = 1.0;

  mOriginalSize = mImage->size();
  mOriginalWidth = mImage->width();
  mOriginalHeight = mImage->height();

  mImageZoom = tr("%1%").arg((int) (mCurrentFactor * 100));
  mImageDimensions = tr("%1x%2 px").arg(mOriginalWidth).arg(mOriginalHeight);
  float sizeInKiB = mImage->byteCount() / (float) 1024;
  if (sizeInKiB > 1024)
    mImageSize =
        mLocale->toString(sizeInKiB / (float) 1024, 'f', 2).append(" MiB");
  else
    mImageSize = mLocale->toString(sizeInKiB, 'f', 2).append(" KiB");
}

void WindowImage::zoomIn() {
  if (mCurrentFactor >= 1.0) {
    mFactorIncrement = (mCurrentFactor + mScaleFactorAbove100) / mCurrentFactor;
    mCurrentFactor += mScaleFactorAbove100;
  } else {
    mFactorIncrement = (mCurrentFactor + mScaleFactorUnder100) / mCurrentFactor;
    mCurrentFactor += mScaleFactorUnder100;
  }
  scaleImage();
}

void WindowImage::zoomOut() {
  if (mCurrentFactor > 1.0) {
    mFactorIncrement = (mCurrentFactor - mScaleFactorAbove100) / mCurrentFactor;
    mCurrentFactor -= mScaleFactorAbove100;
  } else {
    mFactorIncrement = (mCurrentFactor - mScaleFactorUnder100) / mCurrentFactor;
    mCurrentFactor -= mScaleFactorUnder100;
  }
  scaleImage();
}

void WindowImage::zoomBestFit() {
  float correctF = 0.99; // This correction factor allows the image to fit the
  // subwindow area without scrollbars.
  int scrollWidth = width();
  int scrollHeight = height();

  float relationScroll = scrollWidth / (float) scrollHeight;
  float relationImage = mOriginalWidth / (float) mOriginalHeight;

  float scaleWidth = scrollWidth / (float) mOriginalWidth;
  float scaleHeight = scrollHeight / (float) mOriginalHeight;

  if (relationScroll > relationImage) {
    mFactorIncrement = correctF * scaleHeight / mCurrentFactor;
    mCurrentFactor = correctF * scaleHeight;
  } else {
    mFactorIncrement = correctF * scaleWidth / mCurrentFactor;
    mCurrentFactor = correctF * scaleWidth;
  }

  scaleImage();
}

void WindowImage::zoomOriginal() {
  mFactorIncrement = 1 / mCurrentFactor;
  mCurrentFactor = 1.0;
  scaleImage();
}

void WindowImage::applyHarris(int sobelApertureSize, int harrisApertureSize,
                              double kValue, int threshold,
                              bool showProcessed) {
  mFeatureType = WindowImage::harris;
  if (mModified)
    mPixmap = mPixmapOriginal;

  cv::Mat image(mImage->height(), mImage->width(), CV_8UC4, mImage->bits(),
                static_cast<size_t>(
                    mImage->bytesPerLine())); // With CV_8UC3 it doesn't work
  cv::Mat imageGrey(mImage->height(), mImage->width(), CV_8UC1);
  cvtColor(image, imageGrey, cv::COLOR_RGB2GRAY);

  cv::Mat imageHarris(mImage->height(), mImage->width(), CV_8UC1);
  auto time = (float) cv::getTickCount();
  cornerHarris(imageGrey, imageHarris, harrisApertureSize, sobelApertureSize,
               kValue);

  mImageTime = mLocale->toString(
      (float) ((cv::getTickCount() - time) * 1000 / cv::getTickFrequency()), 'f',
      2);

  // Increases the contrast. If not only a nearly black image would be seen
  cv::Mat imageHarrisNorm(imageHarris.size(), CV_32FC1);
  normalize(imageHarris, imageHarrisNorm, 0, 255, cv::NORM_MINMAX,
            CV_32FC1); // The same than the next five lines
  // 	double min=0, max=255, minVal, maxVal, scale, shift;
  // 	minMaxLoc(imageHarris, &minVal, &maxVal);
  // 	scale = (max-min)/(maxVal-minVal);
  // 	shift = -minVal*scale+min;
  // 	imageHarris.convertTo(imageHarrisNorm, CV_32FC1, scale, shift);

  int keypoints = 0;
  mPainter->begin(&mPixmap);
  QPen pen(QColor::fromRgb(255, 0, 0));
  pen.setWidth(2);
  mPainter->setPen(pen);
  mPainter->setRenderHint(QPainter::Antialiasing);
  for (int j = 0; j < imageHarrisNorm.rows; j++)
    for (int i = 0; i < imageHarrisNorm.cols; i++)
      if ((int) imageHarrisNorm.at<float>(j, i) > threshold) {
        mPainter->drawEllipse(i, j, 4, 4);
        ++keypoints;
      }

  mPainter->end();
  mImageKeypoints = mLocale->toString((float) keypoints, 'f', 0);

  if (showProcessed)
    showProcessedImage(imageHarrisNorm);
  mModified = true;
  uiLabelImage->setPixmap(mPixmap.scaled(mCurrentFactor * mOriginalSize,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation));
}

void WindowImage::applyFast(int threshold, bool nonMaxSuppression) {
  mFeatureType = WindowImage::fast;
  if (mModified)
    mPixmap = mPixmapOriginal;

  cv::Mat image(mImage->height(), mImage->width(), CV_8UC4, mImage->bits(),
                static_cast<size_t>(
                    mImage->bytesPerLine())); // With CV_8UC3 it doesn't work
  cv::Mat imageGrey(mImage->height(), mImage->width(), CV_8UC1);
  cv::cvtColor(image, imageGrey, cv::COLOR_RGB2GRAY);

  std::vector<cv::KeyPoint> keyPoints;
  auto time = cv::getTickCount();
  FAST(imageGrey, keyPoints, threshold, nonMaxSuppression);

  mImageTime = mLocale->toString(
      (float) ((cv::getTickCount() - time) * 1000 / cv::getTickFrequency()), 'f',
      2);
  mImageKeypoints = mLocale->toString((float) keyPoints.size(), 'f', 0);

  mPainter->begin(&mPixmap);
  QPen pen(QColor::fromRgb(255, 0, 0));
  pen.setWidth(2);
  mPainter->setPen(pen);
  mPainter->setRenderHint(QPainter::Antialiasing);
  for (const auto &point : keyPoints)
    mPainter->drawEllipse((int) point.pt.x, (int) point.pt.y, 4, 4);
  mPainter->end();

  mModified = true;
  uiLabelImage->setPixmap(mPixmap.scaled(mCurrentFactor * mOriginalSize,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation));
}

void WindowImage::applySift(double threshold, double edgeThreshold,
                            int nOctaves, int nOctaveLayers,
                            bool showOrientation) {
  mFeatureType = WindowImage::sift;
  if (mModified)
    mPixmap = mPixmapOriginal;

  cv::Mat image(mImage->height(), mImage->width(), CV_8UC4, mImage->bits(),
                static_cast<size_t>(
                    mImage->bytesPerLine())); // With CV_8UC3 it doesn't work
  cv::Mat imageGrey(mImage->height(), mImage->width(), CV_8UC1);
  cv::cvtColor(image, imageGrey, cv::COLOR_RGB2GRAY);

  std::vector<cv::KeyPoint> keyPoints;
  auto time = (float) cv::getTickCount();
  cv::Ptr<cv::Feature2D> feature = cv::xfeatures2d::SIFT::create(
      nOctaveLayers, nOctaves, threshold, edgeThreshold);
  feature->detect(imageGrey, keyPoints);

  mImageTime = mLocale->toString(
      (float) ((cv::getTickCount() - time) * 1000 / cv::getTickFrequency()), 'f',
      2);
  mImageKeypoints = mLocale->toString((float) keyPoints.size(), 'f', 0);

  QPoint center;
  mPainter->begin(&mPixmap);
  mPainter->setRenderHint(QPainter::Antialiasing);
  for (auto &point : keyPoints) {
    center.setX((int) point.pt.x);
    center.setY((int) point.pt.y);
    auto radius =
        (int) (point.size); // radius = (int)(keyPoints->at(n).size*1.2/9.*2); =
    // 0.266666
    if (showOrientation) {
      mPainter->setPen(QColor::fromRgb(255, 0, 0));
      mPainter->drawLine(QLineF(
          point.pt.x, point.pt.y,
          point.pt.x + point.size * qCos(point.angle * 3.14159265 / 180),
          point.pt.y + point.size * qSin(point.angle * 3.14159265 / 180)));
    }
    mPainter->setPen(QColor::fromRgb(0, 0, 255));
    mPainter->drawEllipse(center, radius, radius);
  }
  mPainter->end();

  mModified = true;
  uiLabelImage->setPixmap(mPixmap.scaled(mCurrentFactor * mOriginalSize,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation));
}

void WindowImage::applySurf(double threshold, int nOctaves, int nOctaveLayers,
                            bool showOrientation) {
  mFeatureType = WindowImage::surf;
  if (mModified)
    mPixmap = mPixmapOriginal;

  cv::Mat image(mImage->height(), mImage->width(), CV_8UC4, mImage->bits(),
                static_cast<size_t>(
                    mImage->bytesPerLine())); // With CV_8UC3 it doesn't work
  cv::Mat imageGrey(mImage->height(), mImage->width(), CV_8UC1);
  cv::cvtColor(image, imageGrey, cv::COLOR_RGB2GRAY);

  std::vector<cv::KeyPoint> keyPoints;
  float time = cv::getTickCount();
  cv::Ptr<cv::Feature2D> feature = cv::xfeatures2d::SURF::create(
      threshold, nOctaves, nOctaveLayers, false, false);
  feature->detect(imageGrey, keyPoints);

  mImageTime = mLocale->toString(
      ((cv::getTickCount() - time) * 1000 / cv::getTickFrequency()), 'd', 2);
  mImageKeypoints = mLocale->toString((float) keyPoints.size(), 'd', 0);

  QPoint center;
  mPainter->begin(&mPixmap);
  mPainter->setRenderHint(QPainter::Antialiasing);
  for (auto &point : keyPoints) {
    center.setX((int) point.pt.x);
    center.setY((int) point.pt.y);
    auto radius =
        (int) point
            .size; // radius = (int)(keyPoints->at(n).size*1.2/9.*2); = 0.266666
    if (showOrientation) {
      mPainter->setPen(QColor::fromRgb(255, 0, 0));
      mPainter->drawLine(QLineF(
          point.pt.x, point.pt.y,
          point.pt.x + point.size * qCos(point.angle * 3.14159265 / 180),
          point.pt.y + point.size * qSin(point.angle * 3.14159265 / 180)));
    }
    mPainter->setPen(QColor::fromRgb(0, 0, 255));
    mPainter->drawEllipse(center, radius, radius);
  }
  mPainter->end();

  mModified = true;
  uiLabelImage->setPixmap(mPixmap.scaled(mCurrentFactor * mOriginalSize,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation));
}

void WindowImage::showProcessedImage(cv::Mat &processedImage) {
  if (mFeatureType == WindowImage::harris) {
    mPixmap = QPixmap::fromImage(convertMat2QImage(
        processedImage)); // This should be faster than the below lines
    // 		Mat imageColor(mImage->height(), mImage->width(), CV_8UC4); //
    // With CV_8UC3 it doesn't work 		cvtColor(processedImage,
    // imageColor, CV_GRAY2RGBA); // With CV_GRAY2RGB it doesn't work
    // 		// With Format_RGB888 it doesn't work. It can be Format_ARGB32
    // as welL 		mPixmap = QPixmap::fromImage(QImage(imageColor.data,
    // mImage->width(), mImage->height(), imageColor.step,
    // QImage::Format_RGB32));
  }
}

void WindowImage::resetImage() {
  mFeatureType = WindowImage::none;
  mPixmap = mPixmapOriginal;
  mImageTime.clear();
  mImageKeypoints.clear();
  mModified = false;
  uiLabelImage->setPixmap(mPixmap.scaled(mCurrentFactor * mOriginalSize,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation));
}

// http://stackoverflow.com/questions/5026965/how-to-convert-an-opencv-cvmat-to-qimage
QImage WindowImage::convertMat2QImage(const cv::Mat_<double> &src) {
  double scale = 1; // Value for CV_32FC1 images. Use -255 for CV_8UC1 images.
  QImage dest(src.cols, src.rows, QImage::Format_RGB32);
  for (int y = 0; y < src.rows; ++y) {
    const auto &pDouble = src[y];
    auto line = dest.scanLine(y);
    for (int x = 0; x < src.cols; ++x) {
      const auto &color = static_cast<unsigned int>(pDouble[x] * scale);
      line[x] = static_cast<uchar>(qRgb(color, color, color));
    }
  }
  return dest;
}

void WindowImage::scaleImage() {
  // 	uiLabelImage->resize(mCurrentFactor*originalSize);
  uiLabelImage->setPixmap(mPixmap.scaled(mCurrentFactor * mOriginalSize,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation));
  adjustScrollBar(horizontalScrollBar());
  adjustScrollBar(verticalScrollBar());
  mImageZoom = tr("%1%").arg((int) (mCurrentFactor * 100));
}

void WindowImage::adjustScrollBar(QScrollBar *scrollBar) {
  scrollBar->setValue(int(mFactorIncrement * scrollBar->value() +
      (mFactorIncrement - 1) * scrollBar->pageStep() / 2));
}

void WindowImage::mousePressEvent(QMouseEvent *event) {
  mLastPoint = event->pos();
  setCursor(Qt::ClosedHandCursor);
}

void WindowImage::mouseMoveEvent(QMouseEvent *event) {
  QPoint myPos = event->pos();
  int hValue = horizontalScrollBar()->value();
  int vValue = verticalScrollBar()->value();
  horizontalScrollBar()->setValue(hValue + (mLastPoint.x() - myPos.x()));
  verticalScrollBar()->setValue(vValue + (mLastPoint.y() - myPos.y()));
  mLastPoint = myPos;
}

void WindowImage::mouseReleaseEvent(QMouseEvent * /*event*/) { unsetCursor(); }

void WindowImage::mouseDoubleClickEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton)
    zoomIn();
  else
    zoomOut();
}

void WindowImage::compute() {
  mCamera >> mImageRT;
  if (!mImageRT.empty()) {
    cvtColor(mImageRT, mImageRT, cv::COLOR_BGR2RGB);
    mImage = std::make_unique<QImage>(
        mImageRT.data, mImageRT.cols, mImageRT.rows,
        static_cast<int>(mImageRT.step), QImage::Format_RGB888);
    mPixmapOriginal = QPixmap::fromImage(*mImage);
    uiLabelImage->setPixmap(mPixmapOriginal); // With RGB32 doesn't work
    mOriginalSize = mImage->size();
    mOriginalWidth = mImage->width();
    mOriginalHeight = mImage->height();

    mImageZoom = tr("%1%").arg((int) (mCurrentFactor * 100));
    mImageDimensions = tr("%1x%2 px").arg(mOriginalWidth).arg(mOriginalHeight);
  }
}
