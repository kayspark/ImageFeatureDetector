/*
* 2010-2015 (C) Antonio Redondo
* http://antonioredondo.com
* https://github.com/AntonioRedondo/ImageFeatureDetector
*
* Code under the terms of the GNU General Public License v3.
*
*/

#pragma once

#include <QtCore>
#include <QtWidgets>
#include "ui_windowMain.h"
#include "ui_barFeaturesHarris.h"
#include "ui_barFeaturesFast.h"
#include "ui_barFeaturesSift.h"
#include "ui_barFeaturesSurf.h"
#include "windowImage.h"
#include "windowCaptureWebcam.h"
#include "windowPreferences.h"
#include "windowAbout.h"
#include "windowStartup.h"
#include "windowFastRealTime.h"
#include "windowDo4.h"
#include <memory>

class WindowMain : public QMainWindow, public Ui::windowMain {
Q_OBJECT
public:
  WindowMain();
  void updateRecentFilesMenu();
  void showWindowImage(WindowImage *windowImage);

  std::unique_ptr<QToolButton> mToolButtonOpenRecent;
  std::unique_ptr<QMenu> mMenuRecentFiles;
  int mCapturedWebcamImages;
  int mTotalImages;

public slots:
  void open();
  void captureWebcam();
  void openFastRT();
private:
  void applyCommonTasks();
  void loadFile(const QString& filepath);
  void closeEvent(QCloseEvent *) override;
  void saveSettings();
  void setRecentFile(const QString & filepath);
  void removeRecentFile(const QString & filepath);

  std::unique_ptr<QSettings> mSettings;
public:
  QSettings *getMSettings();
private:
  bool mSeparatorOpenWindowsAdded; // Adding the separator on Qt Designer doesn't work
  QAction *mActionExit = nullptr;
  QAction *mActionSeparatorRecentFiles = nullptr;
  QSignalMapper *mSignalMapper = nullptr;
  QActionGroup *mActionGroupZoom = nullptr;
  QActionGroup *mActionGroupFeatures = nullptr;
  QActionGroup *mActionGroupWindow = nullptr;
  enum { maxRecentFiles = 8 };
  std::array<QAction *, maxRecentFiles> mActionRecentFiles;
  QAction *mHarrisAction = nullptr;
  QAction *mFastAction = nullptr;
  QAction *mSiftAction = nullptr;
  QAction *mSurfAction = nullptr;
  QAction *mCurrentFeatureAction = nullptr;
  std::vector<QAction *> mSubwindowActions;
  Ui::barFeaturesHarris mUIHarris;
  Ui::barFeaturesFast mUIFast;
  Ui::barFeaturesSift mUISift;
  Ui::barFeaturesSurf mUISurf;
  QWidget *mHarrisToolBar = nullptr;
  QWidget *mSiftToolBar = nullptr;
  QWidget *mSurfToolBar = nullptr;
  QWidget *mFastToolBar = nullptr;

  QMdiSubWindow *mActiveWindow = nullptr;
  WindowImage *mActiveWindowImage = nullptr;
  QIcon *mIconHarris = nullptr;
  QIcon *mIconFAST = nullptr;
  QIcon *mIconSIFT = nullptr;
  QIcon *mIconSURF = nullptr;
  QLabel *mStatusBarLabelZoom = nullptr;
  QLabel *mStatusBarLabelDimensions = nullptr;
  QLabel *mStatusBarLabelSize = nullptr;
  QLabel *mStatusBarLabelTime = nullptr;
  QLabel *mStatusBarLabelIcon = nullptr;
  QLabel *mStatusBarLabelKeypoints = nullptr;
  QLabel *mStatusBarLabelSpaceRight = nullptr;
  QLabel *mStatusBarLabelSpaceLeft = nullptr;
  QFrame *mStatusBarLine = nullptr;
  QFrame *mStatusBarLine2 = nullptr;
  QFrame *mStatusBarLine3 = nullptr;

private slots:
  void saveCopyAs();
  void preferences();
  void exit();

  void copy();
  void resetImage();

  void startupDialog();
  void zoom();

  void showHarrisToolBar();
  void applyHarris();
  void saveHarrisParams();
  void resetHarrisParams();

  void showFastToolBar();
  void applyFast();
  void saveFastParams();
  void restFastParams();

  void showSiftToolBar();
  void applySift();
  void saveSiftParams();
  void resetSiftParams();

  void showSurfToolBar();
  void applySurf();
  void saveSurfParams();
  void resetSurfParams();

  void do4();

  void tile();
  void cascade();
  void duplicate();
  void closeActiveSubWindow();
  void closeAllSubWindows();

  void website();
  void about();

  void updateWindowMenu(QMdiSubWindow *);
  void openRecentFile();
  void setActiveSubWindow(QWidget *);
};
