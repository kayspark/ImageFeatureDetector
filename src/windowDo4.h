/*
 * 2018(C) kayspark , 2010-2015 (C) Antonio Redondo
 *
 * https://github.com/kayspark/ImageFeatureDetector
 *
 * Code under the terms of the GNU General Public License v3.
 *
 */

#pragma once

#include "ui_windowDo4.h"
#include "windowImage.h"

class WindowDo4
    : public QWidget
    , Ui::windowDo4 {
  Q_OBJECT
public:
  WindowDo4(const QString &windowTitle, std::unique_ptr<WindowImage> harrisImage,
            std::unique_ptr<WindowImage> fastImage, std::unique_ptr<WindowImage> siftImage,
            std::unique_ptr<WindowImage> surfImage);

private:
  void changeEvent(QEvent *event) override;

  std::unique_ptr<WindowImage> mHarrisImage;
  std::unique_ptr<WindowImage> mFastImage;
  std::unique_ptr<WindowImage> mSiftImage;
  std::unique_ptr<WindowImage> mSurfImage;

  std::unique_ptr<QTimer> mTimer;

private slots:
  void zoomBestFit();
};
