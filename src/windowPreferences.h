/*
* 2010-2015 (C) Antonio Redondo
* http://antonioredondo.com
* https://github.com/AntonioRedondo/ImageFeatureDetector
*
* Code under the terms of the GNU General Public License v3.
*
*/

#pragma once

#include "ui_windowPreferences.h"
#include "windowMain.h"

class WindowMain; // http://stackoverflow.com/questions/2133250/does-not-name-a-type-error-in-c

class WindowPreferences : public QDialog, private Ui::windowPreferences {
Q_OBJECT
public:
  explicit WindowPreferences(WindowMain *windowMain);

private:
  WindowMain *mWindowMain = nullptr;
  QSettings *mSettings = nullptr;

private slots:
  void clearRecentFilesPrompt();
  void clearRecentFiles();
  void restore();
  void save();
};
