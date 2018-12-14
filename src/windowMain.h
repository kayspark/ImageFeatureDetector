/*
 * 2018(C) kayspark , 2010-2015 (C) Antonio Redondo
 *
 * https://github.com/kayspark/ImageFeatureDetector
 *
 * Code under the terms of the GNU General Public License v3.
 *
 */

#pragma once

#include "ui_barFeaturesFast.h"
#include "ui_barFeaturesHarris.h"
#include "ui_barFeaturesSift.h"
#include "ui_barFeaturesSurf.h"
#include "ui_windowMain.h"
#include "windowAbout.h"
#include "windowCaptureWebcam.h"
#include "windowDo4.h"
#include "windowFastRealTime.h"
#include "windowImage.h"
#include "windowPreferences.h"
#include "windowStartup.h"
#include <QtCore>
#include <QtWidgets>
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
  void loadFile(const QString &filepath);
  void closeEvent(QCloseEvent *) override;
  void saveSettings();
  void setRecentFile(const QString &filepath);
  void removeRecentFile(const QString &filepath);

  std::unique_ptr<QSettings> mSettings;

public:
  QSettings *getMSettings();

private:
  bool mSeparatorOpenWindowsAdded; // Adding the separator on Qt Designer
  // doesn't work
  std::unique_ptr<QAction> mActionExit;
  QAction *mActionSeparatorRecentFiles = nullptr;
  std::unique_ptr<QSignalMapper> mSignalMapper;

  std::unique_ptr<QActionGroup> mActionGroupZoom;
  std::unique_ptr<QActionGroup> mActionGroupFeatures;
  std::unique_ptr<QActionGroup> mActionGroupWindow;
  enum { maxRecentFiles = 8 };
  std::array<std::unique_ptr<QAction>, maxRecentFiles> mActionRecentFiles;
  QAction *mHarrisAction = nullptr;
  QAction *mFastAction = nullptr;
  QAction *mSiftAction = nullptr;
  QAction *mSurfAction = nullptr;
  QAction *mCurrentFeatureAction = nullptr;
  std::vector<QAction *> mSubwindowActions;
  Ui::barFeaturesHarris mUIHarris{};
  Ui::barFeaturesFast mUIFast{};
  Ui::barFeaturesSift mUISift{};
  Ui::barFeaturesSurf mUISurf{};
  std::unique_ptr<QWidget> mHarrisToolBar;
  std::unique_ptr<QWidget> mSiftToolBar;
  std::unique_ptr<QWidget> mSurfToolBar;
  std::unique_ptr<QWidget> mFastToolBar;

  QMdiSubWindow *mActiveWindow = nullptr;
  WindowImage *mActiveWindowImage = nullptr;
  std::unique_ptr<QIcon> mIconHarris;
  std::unique_ptr<QIcon> mIconFAST;
  std::unique_ptr<QIcon> mIconSIFT;
  std::unique_ptr<QIcon> mIconSURF;

  std::unique_ptr<QLabel> mStatusBarLabelZoom;
  std::unique_ptr<QLabel> mStatusBarLabelDimensions;
  std::unique_ptr<QLabel> mStatusBarLabelSize;
  std::unique_ptr<QLabel> mStatusBarLabelTime;
  std::unique_ptr<QLabel> mStatusBarLabelIcon;
  std::unique_ptr<QLabel> mStatusBarLabelKeypoints;
  std::unique_ptr<QLabel> mStatusBarLabelSpaceRight;
  std::unique_ptr<QLabel> mStatusBarLabelSpaceLeft;

  std::unique_ptr<QFrame> mStatusBarLine;
  std::unique_ptr<QFrame> mStatusBarLine2;
  std::unique_ptr<QFrame> mStatusBarLine3;

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
