/*
* 2010-2015 (C) Antonio Redondo
* http://antonioredondo.com
* https://github.com/AntonioRedondo/ImageFeatureDetector
*
* Code under the terms of the GNU General Public License v3.
*
*/
#pragma once

#include "ui_windowStartup.h"
#include "windowMain.h"

//class WindowMain; // http://stackoverflow.com/questions/2133250/does-not-name-a-type-error-in-c

class WindowStartup : public QDialog, private Ui::windowStartup {
Q_OBJECT
public:
  explicit WindowStartup(WindowMain *windowMain);

private:
  WindowMain *mWindowMain = nullptr;

private slots:
  void open() override;
  void webcam();
  void fastRT();
  void saveSettings();
};

