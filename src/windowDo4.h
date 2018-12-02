/*
* 2010-2015 (C) Antonio Redondo
* http://antonioredondo.com
* https://github.com/AntonioRedondo/ImageFeatureDetector
*
* Code under the terms of the GNU General Public License v3.
*
*/

#pragma once

#include "ui_windowDo4.h"
#include "windowImage.h"

class WindowDo4 : public QWidget, Ui::windowDo4 {
Q_OBJECT
public:
  WindowDo4(const QString &windowTitle,
            WindowImage *harrisImage,
            WindowImage *fastImage,
            WindowImage *siftImage,
            WindowImage *surfImage);

private:
  void changeEvent(QEvent *event) override;

  WindowImage *mHarrisImage = nullptr;
  WindowImage *mFastImage = nullptr;
  WindowImage *mSiftImage = nullptr;
  WindowImage *mSurfImage = nullptr;

  QTimer *mTimer = nullptr;

private slots:
  void zoomBestFit();
};
