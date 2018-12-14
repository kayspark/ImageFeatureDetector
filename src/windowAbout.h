/*
* 2018(C) kayspark , 2010-2015 (C) Antonio Redondo
*
* https://github.com/kayspark/ImageFeatureDetector
*
* Code under the terms of the GNU General Public License v3.
*
*/

#pragma once
#include <QDialog>
#include "ui_windowAbout.h"

class WindowAbout : public QDialog, private Ui::windowAbout {
Q_OBJECT
public:
  WindowAbout() = default;
  explicit WindowAbout(QWidget *windowMain) : QDialog(windowMain, Qt::Dialog) {
    setupUi(this);
// 		setWindowFlags(Qt::Dialog);
    show();
  }
};
