
/*
 * 2018(C) kayspark , 2010-2015 (C) Antonio Redondo
 *
 * https://github.com/kayspark/ImageFeatureDetector
 *
 * Code under the terms of the GNU General Public License v3.
 *
 */

#include "windowImage.hpp"
#include <qstring.h>
#include <utility>
WindowImage::WindowImage(const QString &fileName, QString windowTitle, int windowType)
    : m_capture(cv::VideoCapture(fileName.toStdString()))
    , mWindowTitle(std::move(windowTitle))
    , mWindowType(windowType)
    , mImageN(0)
    , mOriginalWidth(0)
    , mOriginalHeight(0)
    , mModified(false)
    , mFeatureType(0)
    , _data_file(":/dataset/cascade.xml")
    , _tracking_algorithm("CSRT")
    , _predator(nm_detector(this->_data_file, this->_tracking_algorithm))
    , mPainter(std::make_unique<QPainter>())
    , mLocale(std::make_unique<QLocale>(QLocale::English)) {
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(mWindowTitle);

    uiScrollAreaWidgetContents->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    if (m_capture.isOpened()) {
        // setup default values
        m_capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        m_capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
        compute();
        float sizeInKiB = _image->byteCount() / (float)1024;
        if (sizeInKiB > 1024)
            mImageSize = mLocale->toString(sizeInKiB / (float)1024, 'f', 2).append(" MiB");
        else
            mImageSize = mLocale->toString(sizeInKiB, 'f', 2).append(" KiB");
        // For async refresh
        timer = std::make_unique<QTimer>();
        timer->start(40); // 25fps
        QObject::connect(timer.get(), &QTimer::timeout, this, &WindowImage::compute);
    } else {
        uiLabelImage->setText("There is some problem with the cam.\nCannot get images.");
        uiLabelImage->setText("Please check camera");
    }

    mOriginalSize = QSize(800, 600);
    mOriginalWidth = 800;
    mOriginalHeight = 600;

    mScaleFactorAbove100 = 0.5;
    mScaleFactorUnder100 = 0.25;
    mFactorIncrement = 0;
    mCurrentFactor = 1.0;
}
WindowImage::WindowImage(std::shared_ptr<QImage> image, QString windowTitle, int windowType)
    : _image(std::move(image))
    , mWindowTitle(std::move(windowTitle))
    , mWindowType(windowType)
    , mImageN(0)
    , mModified(false)
    , mFeatureType(0)
    , _data_file(":/dataset/cascade.xml")
    , _tracking_algorithm("CSRT")
    , _predator(nm_detector(this->_data_file, this->_tracking_algorithm))
    , mPainter(std::make_unique<QPainter>())
    , mLocale(std::make_unique<QLocale>(QLocale::English)) {
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(mWindowTitle);

    uiScrollAreaWidgetContents->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    mPixmap = QPixmap::fromImage(*_image);
    mPixmapOriginal = mPixmap;
    uiLabelImage->setPixmap(mPixmap);

    mScaleFactorAbove100 = 0.5;
    mScaleFactorUnder100 = 0.25;
    mFactorIncrement = 0;
    mCurrentFactor = 1.0;

    mOriginalSize = _image->size();
    mOriginalWidth = _image->width();
    mOriginalHeight = _image->height();

    mImageZoom = tr("%1%").arg((int)(mCurrentFactor * 100));
    mImageDimensions = tr("%1x%2 px").arg(mOriginalWidth).arg(mOriginalHeight);
    float sizeInKiB = _image->byteCount() / (float)1024;
    if (sizeInKiB > 1024)
        mImageSize = mLocale->toString(sizeInKiB / (float)1024, 'f', 2).append(" MiB");
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

    float relationScroll = scrollWidth / static_cast<float>(scrollHeight);
    float relationImage = mOriginalWidth / static_cast<float>(mOriginalHeight);

    float scaleWidth = scrollWidth / static_cast<float>(mOriginalWidth);
    float scaleHeight = scrollHeight / static_cast<float>(mOriginalHeight);

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

void WindowImage::applyHarris(int sobelApertureSize, int harrisApertureSize, double kValue, int threshold,
                              bool showProcessed) {
    mFeatureType = WindowImage::harris;
    if (mModified)
        mPixmap = mPixmapOriginal;

    cv::Mat image(_image->height(), _image->width(), CV_8UC4, _image->bits(),
                  static_cast<size_t>(_image->bytesPerLine())); // With CV_8UC3 it doesn't work
    cv::Mat imageGrey(_image->height(), _image->width(), CV_8UC1);
    cvtColor(image, imageGrey, cv::COLOR_RGB2GRAY);

    cv::Mat imageHarris(_image->height(), _image->width(), CV_8UC1);
    auto time = static_cast<float>(cv::getTickCount());
    cornerHarris(imageGrey, imageHarris, harrisApertureSize, sobelApertureSize, kValue);

    mImageTime = mLocale->toString((float)((cv::getTickCount() - time) * 1000 / cv::getTickFrequency()), 'f', 2);

    // Increases the contrast. If not only a nearly black image would be seen
    cv::Mat imageHarrisNorm(imageHarris.size(), CV_32FC1);
    normalize(imageHarris, imageHarrisNorm, 0, 255, cv::NORM_MINMAX,
              CV_32FC1); // The same than the next five lines
    // 	double min=0, max=255, minVal, maxVal, scale, shift;
    // 	minMaxLoc(imageHarris, &minVal, &maxVal);
    // 	scale = (max-min)/(maxVal-minVal);
    // 	shift = -minVal*scale+min;
    // 	imageHarris.convertTo(imageHarrisNorm, CV_32FC1, scale, shift);
    mPainter->begin(&mPixmap);
    QPen pen(QColor::fromRgb(255, 0, 0));
    pen.setWidth(2);
    mPainter->setPen(pen);
    mPainter->setRenderHint(QPainter::Antialiasing);
    int keyPoints = 0;
    // std::vector<cv::Point2i> keyPoints;
    std::mutex mutex_;
    if (imageHarrisNorm.isContinuous())
        imageHarrisNorm.forEach<float>(
            [this, &mutex_, &threshold, &keyPoints](const float pixel, const int *position) -> void {
                if (pixel > threshold) {
                    mutex_.lock();
                    mPainter->drawEllipse(position[1], position[0], 4, 4);
                    keyPoints++;
                    mutex_.unlock();
                }
            });
    mPainter->end();
    mImageKeypoints = mLocale->toString(keyPoints);

    if (showProcessed) {
        showProcessedImage(imageHarrisNorm);
    }
    mModified = true;
    uiLabelImage->setPixmap(
        mPixmap.scaled(mCurrentFactor * mOriginalSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void WindowImage::applyFast(int threshold, bool nonMaxSuppression) {
    mFeatureType = WindowImage::fast;
    if (mModified)
        mPixmap = mPixmapOriginal;

    cv::Mat image(_image->height(), _image->width(), CV_8UC4, _image->bits(),
                  static_cast<size_t>(_image->bytesPerLine())); // With CV_8UC3 it doesn't work
    cv::Mat imageGrey(_image->height(), _image->width(), CV_8UC1);
    cv::cvtColor(image, imageGrey, cv::COLOR_RGB2GRAY);

    std::vector<cv::KeyPoint> keyPoints;
    auto time = cv::getTickCount();
    FAST(imageGrey, keyPoints, threshold, nonMaxSuppression);

    mImageTime = mLocale->toString((float)((cv::getTickCount() - time) * 1000 / cv::getTickFrequency()), 'f', 2);
    mImageKeypoints = mLocale->toString((float)keyPoints.size(), 'f', 0);

    mPainter->begin(&mPixmap);
    QPen pen(QColor::fromRgb(255, 0, 0));
    pen.setWidth(2);
    mPainter->setPen(pen);
    mPainter->setRenderHint(QPainter::Antialiasing);
    for (const auto &point : keyPoints)
        mPainter->drawEllipse((int)point.pt.x, (int)point.pt.y, 4, 4);
    mPainter->end();

    mModified = true;
    uiLabelImage->setPixmap(
        mPixmap.scaled(mCurrentFactor * mOriginalSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void WindowImage::applySift(double threshold, double edgeThreshold, int nOctaves, int nOctaveLayers,
                            bool showOrientation) {
    mFeatureType = WindowImage::sift;
    if (mModified)
        mPixmap = mPixmapOriginal;

    cv::Mat image(_image->height(), _image->width(), CV_8UC4, _image->bits(),
                  static_cast<size_t>(_image->bytesPerLine())); // With CV_8UC3 it doesn't work
    cv::Mat imgGray(_image->height(), _image->width(), CV_8UC1);
    cv::cvtColor(image, imgGray, cv::COLOR_RGB2GRAY);

    std::vector<cv::KeyPoint> keyPoints;
    auto time = static_cast<float>(cv::getTickCount());
    cv::Ptr<cv::Feature2D> feature = cv::xfeatures2d::SIFT::create(nOctaveLayers, nOctaves, threshold, edgeThreshold);
    feature->detect(imgGray, keyPoints);

    mImageTime = mLocale->toString((float)((cv::getTickCount() - time) * 1000 / cv::getTickFrequency()), 'f', 2);
    mImageKeypoints = mLocale->toString((int)keyPoints.size());

    QPoint center;
    mPainter->begin(&mPixmap);
    mPainter->setRenderHint(QPainter::Antialiasing);
    for (const auto &point : keyPoints) {
        center.setX((int)point.pt.x);
        center.setY((int)point.pt.y);
        const auto radius = (int)(point.size); // radius = (int)(keyPoints->at(n).size*1.2/9.*2); =
        // 0.266666
        if (showOrientation) {
            mPainter->setPen(QColor::fromRgb(255, 0, 0));
            mPainter->drawLine(QLineF(point.pt.x, point.pt.y,
                                      point.pt.x + point.size * qCos(point.angle * 3.14159265 / 180),
                                      point.pt.y + point.size * qSin(point.angle * 3.14159265 / 180)));
        }
        mPainter->setPen(QColor::fromRgb(0, 0, 255));
        mPainter->drawEllipse(center, radius, radius);
    }
    mPainter->end();

    mModified = true;
    uiLabelImage->setPixmap(
        mPixmap.scaled(mCurrentFactor * mOriginalSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void WindowImage::applySurf(double threshold, int nOctaves, int nOctaveLayers, bool showOrientation) {
    mFeatureType = WindowImage::surf;
    if (mModified)
        mPixmap = mPixmapOriginal;

    cv::Mat image(_image->height(), _image->width(), CV_8UC4, _image->bits(),
                  static_cast<size_t>(_image->bytesPerLine())); // With CV_8UC3 it doesn't work
    cv::Mat imageGrey(_image->height(), _image->width(), CV_8UC1);
    cv::cvtColor(image, imageGrey, cv::COLOR_RGB2GRAY);

    std::vector<cv::KeyPoint> keyPoints;
    float time = cv::getTickCount();
    cv::Ptr<cv::Feature2D> feature = cv::xfeatures2d::SURF::create(threshold, nOctaves, nOctaveLayers, false, false);
    feature->detect(imageGrey, keyPoints);

    mImageTime = mLocale->toString(((cv::getTickCount() - time) * 1000 / cv::getTickFrequency()), 'd', 2);
    mImageKeypoints = mLocale->toString((float)keyPoints.size(), 'd', 0);

    QPoint center;
    mPainter->begin(&mPixmap);
    mPainter->setRenderHint(QPainter::Antialiasing);
    for (const auto &point : keyPoints) {
        center.setX((int)point.pt.x);
        center.setY((int)point.pt.y);
        const auto radius = (int)point.size; // radius = (int)(keyPoints->at(n).size*1.2/9.*2); = 0.266666
        if (showOrientation) {
            mPainter->setPen(QColor::fromRgb(255, 0, 0));
            mPainter->drawLine(QLineF(point.pt.x, point.pt.y,
                                      point.pt.x + point.size * qCos(point.angle * 3.14159265 / 180),
                                      point.pt.y + point.size * qSin(point.angle * 3.14159265 / 180)));
        }
        mPainter->setPen(QColor::fromRgb(0, 0, 255));
        mPainter->drawEllipse(center, radius, radius);
    }
    mPainter->end();

    mModified = true;
    uiLabelImage->setPixmap(
        mPixmap.scaled(mCurrentFactor * mOriginalSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void WindowImage::showProcessedImage(cv::Mat &processedImage) {
    if (mFeatureType == WindowImage::harris) {
        mPixmap = QPixmap::fromImage(convertMat2QImage(processedImage)); // This should be faster than the below lines
        // 		Mat imageColor(_image->height(), _image->width(), CV_8UC4); //
        // With CV_8UC3 it doesn't work 		cvtColor(processedImage,
        // imageColor, CV_GRAY2RGBA); // With CV_GRAY2RGB it doesn't work
        // 		// With Format_RGB888 it doesn't work. It can be Format_ARGB32
        // as welL 		mPixmap = QPixmap::fromImage(QImage(imageColor.data,
        // _image->width(), _image->height(), imageColor.step,
        // QImage::Format_RGB32));
    }
}

void WindowImage::resetImage() {
    mFeatureType = WindowImage::none;
    mPixmap = mPixmapOriginal;
    mImageTime.clear();
    mImageKeypoints.clear();
    mModified = false;
    uiLabelImage->setPixmap(
        mPixmap.scaled(mCurrentFactor * mOriginalSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

// https://stackoverflow.com/questions/17127762/cvmat-to-qimage-and-back/17137998
QImage WindowImage::convertMat2QImage(const cv::Mat &src) {
    cv::Mat temp;                           // make the same cv::Mat
    cvtColor(src, temp, cv::COLOR_BGR2RGB); // cvtColor Makes a copt, that what i need
    QImage dest((const uchar *)temp.data, temp.cols, temp.rows, static_cast<int>(temp.step), QImage::Format_RGB32);
    dest.bits(); // enforce deep copy, see documentation
    // of QImage::QImage ( const uchar * data, int width, int height, Format
    // format )
    return dest;
}

void WindowImage::scaleImage() {
    // 	uiLabelImage->resize(mCurrentFactor*originalSize);
    uiLabelImage->setPixmap(
        mPixmap.scaled(mCurrentFactor * mOriginalSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    adjustScrollBar(horizontalScrollBar());
    adjustScrollBar(verticalScrollBar());
    mImageZoom = tr("%1%").arg((int)(mCurrentFactor * 100));
}

void WindowImage::adjustScrollBar(QScrollBar *scrollBar) {
    scrollBar->setValue(
        int(mFactorIncrement * scrollBar->value() + (mFactorIncrement - 1) * scrollBar->pageStep() / 2));
}

void WindowImage::mousePressEvent(QMouseEvent *event) {
    mLastPoint = event->pos();
    if (_rubberBand == nullptr)
        _rubberBand = std::make_unique<QRubberBand>(QRubberBand::Rectangle, this);
    // setCursor(Qt::ClosedHandCursor);
}

void WindowImage::mouseMoveEvent(QMouseEvent *event) {
    _rubberBand->setGeometry(QRect(mLastPoint, event->pos()).normalized());
    _rubberBand->show();
    band_avaiable = false;
    QToolTip::showText(event->globalPos(),
                       QString("%1,%2").arg(_rubberBand->size().width()).arg(_rubberBand->size().height()), this);
    // mLastPoint = myPos;
}

void WindowImage::mouseReleaseEvent(QMouseEvent * /*event*/) {
    // unsetCursor();
    band_avaiable = true;
    //_rubberBand->hide();
    // determine selection.. QRect::contains..
}

void WindowImage::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton)
        zoomIn();
    else
        zoomOut();
}

void WindowImage::compute() {
    m_capture.read(_imgRT);
    if (!_imgRT.empty()) {

        cv::resize(_imgRT, _imgRT, cv::Size(800, 600), 0, 0, cv::INTER_CUBIC);
        cv::Mat gray;
        cv::cvtColor(_imgRT, gray, cv::COLOR_BGR2GRAY);

        QRect qRect;
        cv::Rect rect1;
        if (_rubberBand && band_avaiable) {
            _rubberBand->showNormal();
            qRect = _rubberBand->geometry();
            rect1 = cv::Rect(qRect.x(), qRect.y(), qRect.width(), qRect.height());
        }
        std::vector<cv::Rect> candidate;
        _predator.detect_candidate(gray, candidate);
        //   cv::resize(_imgRT, _imgRT, cv::Size(640, 480), 0, 0, cv::INTER_CUBIC);
        cvtColor(_imgRT, _imgRT, cv::COLOR_BGR2RGB);
        _image = std::make_unique<QImage>(_imgRT.data, _imgRT.cols, _imgRT.rows, static_cast<int>(_imgRT.step),
                                          QImage::Format_RGB888);

        mPixmap = QPixmap::fromImage(*_image);
        mPainter->begin(&mPixmap);
        QPen pen1(QColor::fromRgb(0, 255, 0));
        pen1.setWidth(5);
        QPen pen(QColor::fromRgb(255, 0, 0));
        pen.setWidth(5);
        for (const auto &r : candidate) {
            if (rect1.contains(r.tl()) || rect1.contains(r.br()))
                mPainter->setPen(pen);
            else
                mPainter->setPen(pen1);
            mPainter->drawRect(r.x, r.y, r.width, r.height);
        }
        mPainter->end();
        uiLabelImage->setPixmap(mPixmap); // With RGB32 doesn't work
        mImageZoom = tr("%1%").arg((int)(mCurrentFactor * 100));
        mImageDimensions = tr("%1x%2px").arg(mOriginalWidth).arg(mOriginalHeight);
    }
}
