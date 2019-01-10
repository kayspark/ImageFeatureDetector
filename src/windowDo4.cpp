/*
 * 2018(C) kayspark , 2010-2015 (C) Antonio Redondo
 *
 * https://github.com/kayspark/ImageFeatureDetector
 *
 * Code under the terms of the GNU General Public License v3.
 *
 */

#include "windowDo4.h"

#define qtApp (dynamic_cast<QApplication *>(QCoreApplication::instance()))

WindowDo4::WindowDo4(const QString &windowTitle, std::unique_ptr<WindowImage> harrisImage,
                     std::unique_ptr<WindowImage> fastImage, std::unique_ptr<WindowImage> siftImage,
                     std::unique_ptr<WindowImage> surfImage)
    : mHarrisImage(std::move(harrisImage))
    , mFastImage(std::move(fastImage))
    , mSiftImage(std::move(siftImage))
    , mSurfImage(std::move(surfImage))
    , mTimer(std::move(std::make_unique<QTimer>())) {
  setupUi(this);

  setWindowTitle(windowTitle + " - Do4!");
  setAttribute(Qt::WA_DeleteOnClose);

  uiHLayout1->insertWidget(1, mHarrisImage.get());
  uiHLayout1->insertWidget(3, mFastImage.get());
  uiHLayout2->insertWidget(1, mSiftImage.get());
  uiHLayout2->insertWidget(3, mSurfImage.get());

  uiHarrisTimeLabel->setText(mHarrisImage->mImageTime);
  uiHarrisKPLabel->setText(mHarrisImage->mImageKeypoints);
  uiFastTimeLabel->setText(mFastImage->mImageTime);
  uiFastKPLabel->setText(mFastImage->mImageKeypoints);
  uiSiftTimeLabel->setText(mSiftImage->mImageTime);
  uiSiftKPLabel->setText(mSiftImage->mImageKeypoints);
  uiSurfTimeLabel->setText(mSurfImage->mImageTime);
  uiSurfKPLabel->setText(mSurfImage->mImageKeypoints);

  QObject::connect(uiPushButtonZoomBestFit, &QPushButton::released, this, &WindowDo4::zoomBestFit);
  QObject::connect(mTimer.get(), &QTimer::timeout, this, &WindowDo4::zoomBestFit);

  // http://wiki.qt.io/Center_a_Window_on_the_Screen
  setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qtApp->desktop()->availableGeometry()));

  show();

  // This timer is to resize the images once the WindowStateChange event is
  // fired because the window is actually resized after the event is fired,
  // i.e., when the window's size is not yet ready and images would get a not
  // fit size.
  mTimer->setSingleShot(true);
  mTimer->setInterval(200);
  mTimer->start();
}

void WindowDo4::zoomBestFit() {
  mHarrisImage->zoomBestFit();
  mFastImage->zoomBestFit();
  mSiftImage->zoomBestFit();
  mSurfImage->zoomBestFit();
}

void WindowDo4::changeEvent(QEvent *event) {
  if (event->type() == QEvent::WindowStateChange) {
    auto pEvent = dynamic_cast<QWindowStateChangeEvent *>(event);
    if (pEvent->oldState() == Qt::WindowMaximized && this->windowState() == Qt::WindowNoState)
      mTimer->start();
    else if (pEvent->oldState() == Qt::WindowNoState && this->windowState() == Qt::WindowMaximized)
      mTimer->start();
  }
}
