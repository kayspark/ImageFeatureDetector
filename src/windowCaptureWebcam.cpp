
#include "windowCaptureWebcam.hpp"
#include "opencvhelper.hpp"
WindowCaptureWebcam::WindowCaptureWebcam(WindowMain *main)
    : QDialog(main, Qt::Dialog)
    , mWindowMain(main)
    , _data_file(":/dataset/cascade.xml")
    , m_tracking_algorithm("CSRT")
    , _nm_classifier(std::make_unique<nm_classifier>())
    , _predator(nm_detector(this->_data_file, this->m_tracking_algorithm))
    , mPainter(std::make_unique<QPainter>())
    , mCamera(cv::VideoCapture(cv::CAP_ANY)) {
  setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);
  m_pen = QColor::fromRgb(0, 255, 0);
  m_pen.setWidth(5);

  QObject::connect(uiPushButtonCapture, &QAbstractButton::clicked, this, &WindowCaptureWebcam::capture);
  QObject::connect(uiPushButtonOK, &QAbstractButton::clicked, this, &WindowCaptureWebcam::ok);
  QObject::connect(uiPushButtonCancel, &QAbstractButton::clicked, this, &WindowCaptureWebcam::close);
  if (mCamera.isOpened()) {
    mTimer = std::make_unique<QTimer>();
    mTimer->start(40); // 25fps
    QObject::connect(mTimer.get(), &QTimer::timeout, this, &WindowCaptureWebcam::compute);
    uiPushButtonCapture->setEnabled(true);
    // 		qDebug() << "Frame format: " << mCamera.get(CV_CAP_PROP_FORMAT);
    // 		qDebug() << "Frame count: " <<
    // mCamera.get(CV_CAP_PROP_FRAME_COUNT); 		qDebug() << "Frame mode:
    // "
    // <<
    // mCamera.get(CV_CAP_PROP_MODE); 		qDebug() << "Frame rate: " <<
    // mCamera.get(CV_CAP_PROP_FPS); 		qDebug() << "Frame width: " <<
    // mCamera.get(CV_CAP_PROP_FRAME_WIDTH); 		qDebug() << "Frame
    // height:
    // "
    // << mCamera.get(CV_CAP_PROP_FRAME_HEIGHT);
  } else {
    uiLabelRealTime->setText("There is some problem with the cam.\nCannot get images.");
    uiLabelCaptured->setText("Please check camera");
  }
  show();
}

void WindowCaptureWebcam::capture() {
  uiLabelCaptured->setPixmap(QPixmap::fromImage(
    QImage(m_imgRT.data, m_imgRT.cols, m_imgRT.rows, static_cast<int>(m_imgRT.step), QImage::Format_RGB888)));
  uiPushButtonOK->setEnabled(true);
}

void WindowCaptureWebcam::ok() {
  mWindowMain->showWindowImage(new WindowImage(std::make_shared<QImage>(uiLabelCaptured->pixmap()->toImage()),
                                               tr("WebCam Captured Image %1").arg(++mWindowMain->mCapturedWebcamImages),
                                               WindowImage::fromWebcam));
  close();
}

void WindowCaptureWebcam::close() {
  if (mTimer)
    mTimer->stop();
  if (mCamera.isOpened())
    mCamera.release();
  QWidget::close();
}

void WindowCaptureWebcam::closeEvent(QCloseEvent *closeEvent) {
  close();
  QDialog::closeEvent(closeEvent);
}

void WindowCaptureWebcam::mousePressEvent(QMouseEvent *event) {
  mLastPoint = event->pos();
  _mouse_pressed = true;
  _rubberBand = std::make_shared<QRubberBand>(QRubberBand::Rectangle, this);
  _bandList.emplace_back(_rubberBand);
  // setCursor(Qt::ClosedHandCursor);
}

void WindowCaptureWebcam::mouseMoveEvent(QMouseEvent *event) {
  if (_mouse_pressed) {
    _rubberBand->setGeometry(QRect(mLastPoint, event->pos()).normalized());
    _rubberBand->show();
    QToolTip::showText(event->globalPos(),
                       QString("%1,%2").arg(_rubberBand->size().width()).arg(_rubberBand->size().height()), this);
  }
  // mLastPoint = myPos;
}

void WindowCaptureWebcam::mouseReleaseEvent(QMouseEvent * /*event*/) {
  // unsetCursor();
  _mouse_pressed = false;
  //_rubberBand->hide();
  // determine selection.. QRect::contains..
}
void WindowCaptureWebcam::compute() {

  mCamera.read(m_imgRT);
  cvtColor(m_imgRT, m_imgRT, cv::COLOR_BGR2RGB);
  if (!m_imgRT.empty()) {
    cv::Mat gray;
    cv::cvtColor(m_imgRT, gray, cv::COLOR_BGR2GRAY);
    std::vector<cv::Rect> roiList;
    for (const auto &band : _bandList) {
      band->showNormal();
      QRect qRect = band->geometry();
      roiList.emplace_back(cv::Rect(qRect.x(), qRect.y(), qRect.width(), qRect.height()));
    }
    std::vector<cv::Rect> motions;
    _predator.detect_candidate(gray, motions);
    //   cv::resize(_imgRT, _imgRT, cv::Size(640, 480), 0, 0,
    //   cv::INTER_CUBIC);
    QPixmap pixmap = QPixmap::fromImage(
      QImage(m_imgRT.data, m_imgRT.cols, m_imgRT.rows, static_cast<int>(m_imgRT.step), QImage::Format_RGB888));
    mPainter->begin(&pixmap);
    for (const auto &r : motions) {
      // check whether in valid roi
      QRect qr(r.x, r.y, r.width, r.height);
      std::for_each(roiList.begin(), roiList.end(), [&r, &qr, &pixmap, this](const auto &roi) {
        if (roi.contains(r.tl()) || roi.contains(r.br())) {
          QPixmap pix = pixmap.copy(qr).scaled(QSize(100, 100), Qt::KeepAspectRatio);
          cv::Mat mat = QPixmap2Mat(pix, true);
          if (_nm_classifier->classify(mat)) {
            m_pen = QColor::fromRgb(255, 0, 0);
            m_pen.setWidth(5);
          } else {
            m_pen = QColor::fromRgb(0, 0, 255);
            m_pen.setWidth(5);
          }
          QString fileName = QString("%1.png").arg(QDateTime::currentDateTime().toString("MMdd_hhmmss"));
          pix.toImage().save(QString("backup/%1").arg(fileName));
          //           mListWidget->addItem(new QListWidgetItem(QIcon(pix), fileName));
        }
      });
      m_pen.setWidth(5);
      mPainter->setPen(m_pen);
      mPainter->drawRect(qr);
    }
    mPainter->end();
    uiLabelRealTime->setPixmap(pixmap);
  }
}
