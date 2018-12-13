/*
 * 2010-2015 (C) Antonio Redondo
 * http://antonioredondo.com
 * https://github.com/AntonioRedondo/ImageFeatureDetector
 *
 * Code under the terms of the GNU General Public License v3.
 *
 */

#include "windowMain.h"

WindowMain::WindowMain()
    : mTotalImages(0), mCapturedWebcamImages(0),
      mSettings(std::make_unique<QSettings>("imageFeatureDetectorSettings.ini",
                                            QSettings::IniFormat)),
      mSeparatorOpenWindowsAdded(false),
      mMenuRecentFiles(std::make_unique<QMenu>(this)),
      mToolButtonOpenRecent(std::make_unique<QToolButton>(this)),
      mIconHarris(std::make_unique<QIcon>("icons/Harris.png")),
      mIconFAST(std::make_unique<QIcon>("icons/Fast.png")),
      mIconSIFT(std::make_unique<QIcon>("icons/Sift.png")),
      mIconSURF(std::make_unique<QIcon>("icons/Surf.png")),
      mSignalMapper(std::make_unique<QSignalMapper>(
          this)), // for the Open Windows menu entries
      mHarrisToolBar(std::make_unique<QWidget>()),
      mSiftToolBar(std::make_unique<QWidget>()),
      mSurfToolBar(std::make_unique<QWidget>()),
      mFastToolBar(std::make_unique<QWidget>()),
      mActionGroupZoom(std::make_unique<QActionGroup>(this)),
      mActionGroupFeatures(std::make_unique<QActionGroup>(this)),
      mActionGroupWindow(std::make_unique<QActionGroup>(this)),
      mActionExit(std::make_unique<QAction>(this)) {

  setupUi(this);
  // below initialization is depends on uiStatusBar. So must be done at here, not above initialization list.

  mStatusBarLine = std::make_unique<QFrame>(uiStatusBar);
  mStatusBarLabelDimensions = std::make_unique<QLabel>(uiStatusBar);
  mStatusBarLabelSize = std::make_unique<QLabel>(uiStatusBar);
  mStatusBarLabelTime = std::make_unique<QLabel>(uiStatusBar);
  mStatusBarLabelIcon = std::make_unique<QLabel>(uiStatusBar);
  mStatusBarLabelKeypoints = std::make_unique<QLabel>(uiStatusBar);
  mStatusBarLabelSpaceRight = std::make_unique<QLabel>(uiStatusBar);
  mStatusBarLabelSpaceLeft = std::make_unique<QLabel>(uiStatusBar);
  mStatusBarLabelZoom = std::make_unique<QLabel>(uiStatusBar);
  mStatusBarLine2 =
      std::make_unique<QFrame>(mStatusBarLabelTime.get());
  mStatusBarLine3 =
      std::make_unique<QFrame>(mStatusBarLabelTime.get());
  setContextMenuPolicy(Qt::PreventContextMenu);
  resize(mSettings->value("size", QSize(700, 480)).toSize());
  move(mSettings->value("pos", QPoint(150, 40)).toPoint());
  uiToolBarFile->setVisible(mSettings->value("uiToolBarFile", true).toBool());
  uiToolBarZoom->setVisible(mSettings->value("uiToolBarZoom", true).toBool());
  uiToolBarFeatures->setVisible(
      mSettings->value("uiToolBarFeatures", true).toBool());

  for (auto &recentFile : mActionRecentFiles) {
    recentFile = std::make_unique<QAction>(this);
    recentFile->setVisible(false);
    QObject::connect(recentFile.get(), &QAction::triggered, this,
                     &WindowMain::openRecentFile);
    uiMenuFile->addAction(recentFile.get());
  }
  mActionSeparatorRecentFiles = uiMenuFile->addSeparator();
  mActionSeparatorRecentFiles->setVisible(false);
  updateRecentFilesMenu();
  mActionExit->setObjectName(QString::fromUtf8("actionExit"));
  mActionExit->setText(QApplication::translate("mainWindow", "Exit", nullptr));
  mActionExit->setShortcut(
      QApplication::translate("mainWindow", "Ctrl+Q", nullptr));
  mActionExit->setIcon(QIcon("icons/window-close.svg"));
  uiMenuFile->addAction(mActionExit.get());

  mToolButtonOpenRecent->setFocusPolicy(Qt::NoFocus);
  mToolButtonOpenRecent->setPopupMode(QToolButton::MenuButtonPopup);
  mToolButtonOpenRecent->setMenu(mMenuRecentFiles.get());
  mToolButtonOpenRecent->setToolButtonStyle(Qt::ToolButtonIconOnly);
  mToolButtonOpenRecent->setAutoRaise(true);
  mToolButtonOpenRecent->setDefaultAction(uiActionOpen);
  uiToolBarFile->insertWidget(uiActionCaptureWebcam,
                              mToolButtonOpenRecent.get());

  mActionGroupZoom->setEnabled(false);
  mActionGroupZoom->addAction(uiActionZoomIn);
  mActionGroupZoom->addAction(uiActionZoomOut);
  mActionGroupZoom->addAction(uiActionZoomOriginal);
  mActionGroupZoom->addAction(uiActionZoomBestFit);

  mActionGroupFeatures->setEnabled(false);
  mActionGroupFeatures->addAction(uiActionSIFT);
  mActionGroupFeatures->addAction(uiActionSURF);
  mActionGroupFeatures->addAction(uiActionHarris);
  mActionGroupFeatures->addAction(uiActionFAST);
  mActionGroupFeatures->addAction(uiActionDo4);

  mActionGroupWindow->setEnabled(false);
  mActionGroupWindow->addAction(uiActionTile);
  mActionGroupWindow->addAction(uiActionCascade);
  mActionGroupWindow->addAction(uiActionNext);
  mActionGroupWindow->addAction(uiActionPrevious);
  mActionGroupWindow->addAction(uiActionDuplicate);
  mActionGroupWindow->addAction(uiActionClose);
  mActionGroupWindow->addAction(uiActionCloseAll);

  mStatusBarLabelZoom->setFrameShape(QFrame::NoFrame);
  mStatusBarLabelZoom->setAlignment(Qt::AlignHCenter);
  mStatusBarLabelDimensions->setFrameShape(QFrame::NoFrame);
  mStatusBarLabelDimensions->setAlignment(Qt::AlignHCenter);
  mStatusBarLabelSize->setFrameShape(QFrame::NoFrame);
  mStatusBarLabelSize->setAlignment(Qt::AlignHCenter);
  mStatusBarLabelTime->setFrameShape(QFrame::NoFrame);
  mStatusBarLabelTime->setAlignment(Qt::AlignHCenter);
  mStatusBarLabelIcon->setSizePolicy(
      QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  mStatusBarLabelIcon->setMinimumSize(QSize(16, 16));
  mStatusBarLabelIcon->setMaximumSize(QSize(16, 16));
  mStatusBarLabelIcon->setScaledContents(true);
  mStatusBarLabelIcon->setAlignment(Qt::AlignHCenter);
  mStatusBarLabelIcon->setVisible(false);
  mStatusBarLabelKeypoints->setAlignment(Qt::AlignHCenter);
  mStatusBarLabelSpaceRight->setMinimumSize(0, 1);
  mStatusBarLabelSpaceLeft->setMinimumSize(0, 1);

  mStatusBarLine->setVisible(false);
  mStatusBarLine->setFrameShape(QFrame::VLine);
  mStatusBarLine->setFrameShadow(QFrame::Sunken);
  mStatusBarLine2->setFrameShape(QFrame::VLine);
  mStatusBarLine2->setFrameShadow(QFrame::Sunken);
  mStatusBarLine2->setVisible(false);

  mStatusBarLine3->setFrameShape(QFrame::VLine);
  mStatusBarLine3->setFrameShadow(QFrame::Sunken);
  mStatusBarLine3->setVisible(false);
  uiStatusBar->addWidget(mStatusBarLabelSpaceLeft.get());
  uiStatusBar->addWidget(mStatusBarLabelZoom.get());
  uiStatusBar->addPermanentWidget(mStatusBarLabelTime.get());
  uiStatusBar->addPermanentWidget(mStatusBarLine2.get());
  uiStatusBar->addPermanentWidget(mStatusBarLabelIcon.get());
  uiStatusBar->addPermanentWidget(mStatusBarLabelKeypoints.get());
  uiStatusBar->addPermanentWidget(mStatusBarLine3.get());
  uiStatusBar->addPermanentWidget(mStatusBarLabelDimensions.get());
  uiStatusBar->addPermanentWidget(mStatusBarLine.get());
  uiStatusBar->addPermanentWidget(mStatusBarLabelSize.get());
  uiStatusBar->addPermanentWidget(mStatusBarLabelSpaceRight.get());

  mHarrisToolBar->setVisible(false);
  mUIHarris.setupUi(mHarrisToolBar.get());
  mUIHarris.uiComboBoxSobelApertureSize->setCurrentIndex(
      mSettings->value("harris/sobelApertureSize", 1).toInt());
  mUIHarris.uiSpinBoxHarrisApertureSize->setValue(
      mSettings->value("harris/harrisApertureSize", 2).toInt());
  mUIHarris.uiDoubleSpinBoxKValue->setValue(
      mSettings->value("harris/kValue", 0.01).toDouble());
  mUIHarris.uiSpinBoxThreshold->setValue(
      mSettings->value("harris/threshold", 63).toInt());
  mUIHarris.uiPushButtonShowProcessedImage->setChecked(
      mSettings->value("harris/showProcessedImage", false).toBool());
  // http://stackoverflow.com/questions/16794695/qt5-overloaded-signals-and-slots
  QObject::connect<void (QComboBox::*)(int)>(
      mUIHarris.uiComboBoxSobelApertureSize, &QComboBox::currentIndexChanged,
      this, &WindowMain::saveHarrisParams);
  QObject::connect(mUIHarris.uiSpinBoxHarrisApertureSize,
                   &QSpinBox::editingFinished, this,
                   &WindowMain::saveHarrisParams);
  QObject::connect(mUIHarris.uiDoubleSpinBoxKValue, &QSpinBox::editingFinished,
                   this, &WindowMain::saveHarrisParams);
  QObject::connect(mUIHarris.uiSpinBoxThreshold, &QSpinBox::editingFinished,
                   this, &WindowMain::saveHarrisParams);
  QObject::connect(mUIHarris.uiPushButtonShowProcessedImage,
                   &QPushButton::toggled, this, &WindowMain::saveHarrisParams);
  QObject::connect(mUIHarris.uiButtonBox->button(QDialogButtonBox::Apply),
                   &QAbstractButton::clicked, this, &WindowMain::applyHarris);
  QObject::connect(mUIHarris.uiButtonBox->button(QDialogButtonBox::Reset),
                   &QAbstractButton::clicked, this,
                   &WindowMain::resetHarrisParams);
  mHarrisAction = uiToolBarParameters->addWidget(mHarrisToolBar.get());

  mFastToolBar->setVisible(false);
  mUIFast.setupUi(mFastToolBar.get());
  mUIFast.uiSpinBoxThreshold->setValue(
      mSettings->value("fast/threshold", 50).toInt());
  mUIFast.uiPushButtonNonMax->setChecked(
      mSettings->value("fast/nonMaxSuppression", true).toBool());
  QObject::connect(mUIFast.uiSpinBoxThreshold, &QSpinBox::editingFinished, this,
                   &WindowMain::saveFastParams);
  QObject::connect(mUIFast.uiPushButtonNonMax, &QPushButton::toggled, this,
                   &WindowMain::saveFastParams);
  QObject::connect(mUIFast.uiButtonBox->button(QDialogButtonBox::Apply),
                   &QAbstractButton::clicked, this, &WindowMain::applyFast);
  QObject::connect(mUIFast.uiButtonBox->button(QDialogButtonBox::Reset),
                   &QAbstractButton::clicked, this,
                   &WindowMain::restFastParams);
  mFastAction = uiToolBarParameters->addWidget(mFastToolBar.get());

  mSiftToolBar->setVisible(false);
  mUISift.setupUi(mSiftToolBar.get());
  mUISift.uiDoubleSpinBoxThreshold->setValue(
      mSettings->value("sift/threshold", 0.04).toDouble());
  mUISift.uiDoubleSpinBoxEdgeThreshold->setValue(
      mSettings->value("sift/edgeThreshold", 10.0).toDouble());
  mUISift.uiSpinBoxFeatures->setValue(
      mSettings->value("sift/features", 0).toInt());
  mUISift.uiSpinBoxLayers->setValue(mSettings->value("sift/layers", 3).toInt());
  mUISift.uiPushButtonShowOrientation->setChecked(
      mSettings->value("sift/showOrientation", true).toBool());
  QObject::connect(mUISift.uiDoubleSpinBoxThreshold, &QSpinBox::editingFinished,
                   this, &WindowMain::saveSiftParams);
  QObject::connect(mUISift.uiDoubleSpinBoxEdgeThreshold,
                   &QSpinBox::editingFinished, this,
                   &WindowMain::saveSiftParams);
  QObject::connect(mUISift.uiSpinBoxFeatures, &QSpinBox::editingFinished, this,
                   &WindowMain::saveSiftParams);
  QObject::connect(mUISift.uiSpinBoxLayers, &QSpinBox::editingFinished, this,
                   &WindowMain::saveSiftParams);
  QObject::connect(mUISift.uiPushButtonShowOrientation, &QPushButton::toggled,
                   this, &WindowMain::saveSiftParams);
  QObject::connect(mUISift.uiButtonBox->button(QDialogButtonBox::Apply),
                   &QAbstractButton::clicked, this, &WindowMain::applySift);
  QObject::connect(mUISift.uiButtonBox->button(QDialogButtonBox::Reset),
                   &QAbstractButton::clicked, this,
                   &WindowMain::resetSiftParams);
  mSiftAction = uiToolBarParameters->addWidget(mSiftToolBar.get());

  mSurfToolBar->setVisible(false);
  mUISurf.setupUi(mSurfToolBar.get());
  mUISurf.uiSpinBoxThreshold->setValue(
      mSettings->value("surf/threshold", 100).toInt());
  mUISurf.uiSpinBoxOctaves->setValue(
      mSettings->value("surf/octaves", 4).toInt());
  mUISurf.uiSpinBoxLayers->setValue(mSettings->value("surf/layers", 3).toInt());
  mUISurf.uiPushButtonShowOrientation->setChecked(
      mSettings->value("surf/showOrientation", true).toBool());
  QObject::connect(mUISurf.uiSpinBoxThreshold, &QSpinBox::editingFinished, this,
                   &WindowMain::saveSurfParams);
  connect(mUISurf.uiSpinBoxOctaves, &QSpinBox::editingFinished, this,
          &WindowMain::saveSurfParams);
  connect(mUISurf.uiSpinBoxLayers, &QSpinBox::editingFinished, this,
          &WindowMain::saveSurfParams);
  connect(mUISurf.uiPushButtonShowOrientation, &QPushButton::toggled, this,
          &WindowMain::saveSurfParams);
  connect(mUISurf.uiButtonBox->button(QDialogButtonBox::Apply),
          &QAbstractButton::clicked, this, &WindowMain::applySurf);
  connect(mUISurf.uiButtonBox->button(QDialogButtonBox::Reset),
          &QAbstractButton::clicked, this, &WindowMain::resetSurfParams);
  mSurfAction = uiToolBarParameters->addWidget(mSurfToolBar.get());

  switch (mSettings->value("startupParameters", 0).toInt()) {
  case 0:showHarrisToolBar();
    break;
  case 1:showFastToolBar();
    break;
  case 2:showSiftToolBar();
    break;
  case 3:showSurfToolBar();
    break;
  default:break;
  }

  QObject::connect(uiActionOpen, &QAction::triggered, this, &WindowMain::open);
  QObject::connect(uiActionCaptureWebcam, &QAction::triggered, this,
                   &WindowMain::captureWebcam);
  QObject::connect(uiActionSaveCopyAs, &QAction::triggered, this,
                   &WindowMain::saveCopyAs);
  QObject::connect(uiActionPreferences, &QAction::triggered, this,
                   &WindowMain::preferences);
  QObject::connect(mActionExit.get(), &QAction::triggered, this,
                   &WindowMain::exit);
  QObject::connect(uiActionCopy, &QAction::triggered, this, &WindowMain::copy);
  QObject::connect(uiActionResetImage, &QAction::triggered, this,
                   &WindowMain::resetImage);
  QObject::connect(uiActionStartupDialog, &QAction::triggered, this,
                   &WindowMain::startupDialog);
  QObject::connect(uiActionZoomIn, &QAction::triggered, this,
                   &WindowMain::zoom);
  QObject::connect(uiActionZoomOut, &QAction::triggered, this,
                   &WindowMain::zoom);
  QObject::connect(uiActionZoomOriginal, &QAction::triggered, this,
                   &WindowMain::zoom);
  QObject::connect(uiActionZoomBestFit, &QAction::triggered, this,
                   &WindowMain::zoom);
  QObject::connect(uiActionHarris, &QAction::triggered, this,
                   &WindowMain::showHarrisToolBar);
  QObject::connect(uiActionFAST, &QAction::triggered, this,
                   &WindowMain::showFastToolBar);
  QObject::connect(uiActionSIFT, &QAction::triggered, this,
                   &WindowMain::showSiftToolBar);
  QObject::connect(uiActionSURF, &QAction::triggered, this,
                   &WindowMain::showSurfToolBar);
  QObject::connect(uiActionDo4, &QAction::triggered, this, &WindowMain::do4);
  QObject::connect(uiActionFastRT, &QAction::triggered, this,
                   &WindowMain::openFastRT);
  QObject::connect(uiActionTile, &QAction::triggered, this, &WindowMain::tile);
  QObject::connect(uiActionCascade, &QAction::triggered, this,
                   &WindowMain::cascade);
  QObject::connect(uiActionNext, &QAction::triggered, uiMdiArea,
                   &QMdiArea::activateNextSubWindow);
  QObject::connect(uiActionPrevious, &QAction::triggered, uiMdiArea,
                   &QMdiArea::activatePreviousSubWindow);
  QObject::connect(uiActionDuplicate, &QAction::triggered, this,
                   &WindowMain::duplicate);
  QObject::connect(uiActionClose, &QAction::triggered, this,
                   &WindowMain::closeActiveSubWindow);
  QObject::connect(uiActionCloseAll, &QAction::triggered, this,
                   &WindowMain::closeAllSubWindows);
  QObject::connect(uiActionWebsite, &QAction::triggered, this,
                   &WindowMain::website);
  QObject::connect(uiActionAbout, &QAction::triggered, this,
                   &WindowMain::about);
  QObject::connect(uiMdiArea, &QMdiArea::subWindowActivated, this,
                   &WindowMain::updateWindowMenu);
  QObject::connect(
      mSignalMapper.get(),
      static_cast<void (QSignalMapper::*)(QWidget *)>(&QSignalMapper::mapped),
      this, &WindowMain::setActiveSubWindow);

  if (mSettings->value("maximized", true).toBool())
    showMaximized();
  else
    show();

  if (mSettings->value("startupDialog", true).toBool())
    startupDialog();
}

void WindowMain::open() {
  loadFile(QFileDialog::getOpenFileName(
      this, tr("Open File"), "/home",
      tr("Images (*.png *.bmp *.jpg *.avi *.mp4 *.mov)")));
}

void WindowMain::captureWebcam() { new WindowCaptureWebcam(this); }

void WindowMain::saveCopyAs() {
  QString fileName = QFileDialog::getSaveFileName(
      nullptr, tr("Save Copy As"),
      QFileInfo(mActiveWindowImage->mWindowTitle).baseName().append(".png"),
      tr("Images (*.bmp *.png)"));
  if (!fileName.isEmpty())
    mActiveWindowImage->mPixmap.save(fileName);
}

void WindowMain::exit() {
  saveSettings();
  close();
}

void WindowMain::copy() {
  QApplication::clipboard()->setPixmap(mActiveWindowImage->mPixmap);
}

void WindowMain::preferences() { new WindowPreferences(this); }

void WindowMain::startupDialog() { new WindowStartup(this); }

void WindowMain::zoom() {
  if (sender() == uiActionZoomIn)
    mActiveWindowImage->zoomIn();
  else if (sender() == uiActionZoomOut)
    mActiveWindowImage->zoomOut();
  else if (sender() == uiActionZoomOriginal)
    mActiveWindowImage->zoomOriginal();
  else if (sender() == uiActionZoomBestFit)
    mActiveWindowImage->zoomBestFit();

  uiActionZoomIn->setEnabled(mActiveWindowImage->mCurrentFactor < 3.0);
  uiActionZoomOut->setEnabled(mActiveWindowImage->mCurrentFactor > 0.25);
  mStatusBarLabelZoom->setText(mActiveWindowImage->mImageZoom);
}

void WindowMain::showHarrisToolBar() {
  if (!mCurrentFeatureAction) {
  } else
    mCurrentFeatureAction->setVisible(false);
  mCurrentFeatureAction = mHarrisAction;
  mCurrentFeatureAction->setVisible(true);
  mSettings->setValue("startupParameters", 0);
  uiActionHarris->setChecked(true);
}

void WindowMain::resetHarrisParams() {
  mUIHarris.uiComboBoxSobelApertureSize->setCurrentIndex(1);
  mUIHarris.uiSpinBoxHarrisApertureSize->setValue(2);
  mUIHarris.uiDoubleSpinBoxKValue->setValue(0.01);
  mUIHarris.uiSpinBoxThreshold->setValue(63);
  mUIHarris.uiPushButtonShowProcessedImage->setChecked(false);
  saveHarrisParams();
}

void WindowMain::applyHarris() {
  int sobelApertureSize = 0;
  switch (mSettings->value("harris/sobelApertureSize", 1).toInt()) {
  case 0:sobelApertureSize = 1;
    break;
  case 1:sobelApertureSize = 3;
    break;
  case 2:sobelApertureSize = 5;
    break;
  case 3:sobelApertureSize = 7;
    break;
  default:break;
  }
  mActiveWindowImage->applyHarris(
      sobelApertureSize,
      mSettings->value("harris/harrisApertureSize", 2).toInt(),
      mSettings->value("harris/kValue", 0.01).toDouble(),
      mSettings->value("harris/threshold", 64).toInt(),
      mSettings->value("harris/showProcessedImage", false).toBool());
  mStatusBarLabelIcon->setPixmap(
      QPixmap::fromImage(QImage("icons/Harris.png")));
  mActiveWindow->setWindowIcon(*mIconHarris);
  applyCommonTasks();
}

void WindowMain::saveHarrisParams() {
  mSettings->setValue("harris/sobelApertureSize",
                      mUIHarris.uiComboBoxSobelApertureSize->currentIndex());
  mSettings->setValue("harris/harrisApertureSize",
                      mUIHarris.uiSpinBoxHarrisApertureSize->value());
  mSettings->setValue("harris/kValue",
                      mUIHarris.uiDoubleSpinBoxKValue->value());
  mSettings->setValue("harris/threshold",
                      mUIHarris.uiSpinBoxThreshold->value());
  mSettings->setValue("harris/showProcessedImage",
                      mUIHarris.uiPushButtonShowProcessedImage->isChecked());
}

void WindowMain::showFastToolBar() {
  if (nullptr != mCurrentFeatureAction)
    mCurrentFeatureAction->setVisible(false);
  else {
  }
  mCurrentFeatureAction = mFastAction;
  mCurrentFeatureAction->setVisible(true);
  mSettings->setValue("startupParameters", 1);
  uiActionFAST->setChecked(true);
}

void WindowMain::restFastParams() {
  mUIFast.uiSpinBoxThreshold->setValue(50);
  mUIFast.uiPushButtonNonMax->setChecked(true);
  saveFastParams();
}

void WindowMain::applyFast() {
  mActiveWindowImage->applyFast(
      mSettings->value("fast/threshold", 50).toInt(),
      mSettings->value("fast/nonMaxSuppression", true).toBool());
  mStatusBarLabelIcon->setPixmap(QPixmap::fromImage(QImage("icons/Fast.png")));
  mActiveWindow->setWindowIcon(*mIconFAST);
  applyCommonTasks();
}

void WindowMain::saveFastParams() {
  mSettings->setValue("fast/threshold", mUIFast.uiSpinBoxThreshold->value());
  mSettings->setValue("fast/nonMaxSuppression",
                      mUIFast.uiPushButtonNonMax->isChecked());
}

void WindowMain::showSiftToolBar() {
  if (mCurrentFeatureAction)
    mCurrentFeatureAction->setVisible(false);
  mCurrentFeatureAction = mSiftAction;
  mCurrentFeatureAction->setVisible(true);
  mSettings->setValue("startupParameters", 2);
  uiActionSIFT->setChecked(true);
}

void WindowMain::resetSiftParams() {
  mUISift.uiDoubleSpinBoxThreshold->setValue(0.014);
  mUISift.uiDoubleSpinBoxEdgeThreshold->setValue(10.0);
  mUISift.uiSpinBoxFeatures->setValue(3);
  mUISift.uiSpinBoxLayers->setValue(1);
  mUISift.uiPushButtonShowOrientation->setChecked(true);
  saveSiftParams();
}

void WindowMain::applySift() {
  mActiveWindowImage->applySift(
      mSettings->value("sift/threshold", 0.04).toDouble(),
      mSettings->value("sift/edgeThreshold", 10.0).toDouble(),
      mSettings->value("sift/features", 0).toInt(),
      mSettings->value("sift/layers", 3).toInt(),
      mSettings->value("sift/showOrientation", true).toBool());
  mStatusBarLabelIcon->setPixmap(QPixmap::fromImage(QImage("icons/Sift.png")));
  mActiveWindow->setWindowIcon(*mIconSIFT);
  applyCommonTasks();
}

void WindowMain::saveSiftParams() {
  mSettings->setValue("sift/threshold",
                      mUISift.uiDoubleSpinBoxThreshold->value());
  mSettings->setValue("sift/edgeThreshold",
                      mUISift.uiDoubleSpinBoxEdgeThreshold->value());
  mSettings->setValue("sift/features", mUISift.uiSpinBoxFeatures->value());
  mSettings->setValue("sift/layers", mUISift.uiSpinBoxLayers->value());
  mSettings->setValue("sift/showOrientation",
                      mUISift.uiPushButtonShowOrientation->isChecked());
}

void WindowMain::showSurfToolBar() {
  if (mCurrentFeatureAction)
    mCurrentFeatureAction->setVisible(false);
  mCurrentFeatureAction = mSurfAction;
  mCurrentFeatureAction->setVisible(true);
  mSettings->setValue("startupParameters", 3);
  uiActionSURF->setChecked(true);
}

void WindowMain::resetSurfParams() {
  mUISurf.uiSpinBoxThreshold->setValue(4000);
  mUISurf.uiSpinBoxOctaves->setValue(3);
  mUISurf.uiSpinBoxLayers->setValue(1);
  mUISurf.uiPushButtonShowOrientation->setChecked(true);
  saveSurfParams();
}

void WindowMain::applySurf() {
  mActiveWindowImage->applySurf(
      mSettings->value("surf/threshold", 100).toInt(),
      mSettings->value("surf/octaves", 4).toInt(),
      mSettings->value("surf/layers", 3).toInt(),
      mSettings->value("surf/showOrientation", true).toBool());
  mStatusBarLabelIcon->setPixmap(QPixmap::fromImage(QImage("icons/Surf.png")));
  mActiveWindow->setWindowIcon(*mIconSURF);
  applyCommonTasks();
}

void WindowMain::saveSurfParams() {
  mSettings->setValue("surf/threshold", mUISurf.uiSpinBoxThreshold->value());
  mSettings->setValue("surf/octaves", mUISurf.uiSpinBoxOctaves->value());
  mSettings->setValue("surf/layers", mUISurf.uiSpinBoxLayers->value());
  mSettings->setValue("surf/showOrientation",
                      mUISurf.uiPushButtonShowOrientation->isChecked());
}

void WindowMain::applyCommonTasks() {
  uiActionResetImage->setEnabled(true);
  mStatusBarLabelTime->setText(mActiveWindowImage->mImageTime + " ms");
  mStatusBarLabelKeypoints->setText(mActiveWindowImage->mImageKeypoints +
      " keypoints");
  mStatusBarLabelIcon->setVisible(true);
  mStatusBarLine2->setVisible(true);
  mStatusBarLine3->setVisible(true);
}

void WindowMain::resetImage() {
  mActiveWindowImage->resetImage();
  uiActionResetImage->setEnabled(false);
  mActiveWindow->setWindowIcon(QApplication::windowIcon());
  mStatusBarLabelIcon->clear();
  mStatusBarLabelIcon->setVisible(false);
  mStatusBarLabelTime->clear();
  mStatusBarLabelKeypoints->clear();
  mStatusBarLine2->setVisible(false);
  mStatusBarLine3->setVisible(false);
}

void WindowMain::do4() {
  auto harrisImage = std::make_unique<WindowImage>(
      mActiveWindowImage->mImage, mActiveWindowImage->mWindowTitle);
  auto fastImage = std::make_unique<WindowImage>(
      mActiveWindowImage->mImage, mActiveWindowImage->mWindowTitle);
  auto siftImage = std::make_unique<WindowImage>(
      mActiveWindowImage->mImage, mActiveWindowImage->mWindowTitle);
  auto surfImage = std::make_unique<WindowImage>(
      mActiveWindowImage->mImage, mActiveWindowImage->mWindowTitle);

  auto sobelApertureSize = 0;
  switch (mSettings->value("harris/sobelApertureSize", 1).toInt()) {
  case 0:sobelApertureSize = 1;
    break;
  case 1:sobelApertureSize = 3;
    break;
  case 2:sobelApertureSize = 5;
    break;
  case 3:sobelApertureSize = 7;
    break;
  default:break;
  }
  harrisImage->applyHarris(
      sobelApertureSize,
      mSettings->value("harris/harrisApertureSize", 2).toInt(),
      mSettings->value("harris/kValue", 0.01).toDouble(),
      mSettings->value("harris/threshold", 64).toInt(),
      mSettings->value("harris/showProcessedImage", false).toBool());

  fastImage->applyFast(
      mSettings->value("fast/threshold", 50).toInt(),
      mSettings->value("fast/nonMaxSuppression", true).toBool());

  siftImage->applySift(mSettings->value("sift/threshold", 0.04).toDouble(),
                       mSettings->value("sift/edgeThreshold", 10.0).toDouble(),
                       mSettings->value("sift/features", 0).toInt(),
                       mSettings->value("sift/layers", 3).toInt(),
                       mSettings->value("sift/showOrientation", true).toBool());

  surfImage->applySurf(mSettings->value("surf/threshold", 100).toInt(),
                       mSettings->value("surf/octaves", 4).toInt(),
                       mSettings->value("surf/layers", 3).toInt(),
                       mSettings->value("surf/showOrientation", true).toBool());

  auto do4 = new WindowDo4(mActiveWindowImage->mWindowTitle,
                           std::move(harrisImage), std::move(fastImage),
                           std::move(siftImage), std::move(surfImage));
  do4->show();
}

void WindowMain::openFastRT() { new WindowFastRealTime(this); }

void WindowMain::tile() {
  uiMdiArea->tileSubWindows();
  if (mSettings->value("bestFit").toBool()) {
    auto list = uiMdiArea->subWindowList();
    for (const auto &item : list)
      qobject_cast<WindowImage *>(item->widget())->zoomBestFit();
  }
}

void WindowMain::cascade() {
  uiMdiArea->cascadeSubWindows();
  if (mSettings->value("bestFit").toBool()) {
    auto list = uiMdiArea->subWindowList();
    for (const auto &item : list)
      qobject_cast<WindowImage *>(item->widget())->zoomBestFit();
  }
}

void WindowMain::duplicate() {
  auto *imageOriginal = uiMdiArea->findChild<WindowImage *>(
      QString(mActiveWindowImage->mOriginalUid));
  ++imageOriginal->mImageN;
  WindowImage *imageDuplicated = new WindowImage(
      std::move(mActiveWindowImage->mImage),
      imageOriginal->mWindowTitle +
          QString(" (Duplicated %1)").arg(imageOriginal->mImageN),
      WindowImage::duplicated);
  imageDuplicated->mOriginalUid = imageOriginal->mUid;
  showWindowImage(imageDuplicated);
}

void WindowMain::closeActiveSubWindow() { uiMdiArea->closeActiveSubWindow(); }

void WindowMain::closeAllSubWindows() { uiMdiArea->closeAllSubWindows(); }

void WindowMain::website() {
  QDesktopServices::openUrl(
      QUrl::fromEncoded("https://github.com/kayspark/ImageFeatureDetector"));
}

void WindowMain::about() { new WindowAbout(this); }

// http://doc.qt.io/qt-5/qtwidgets-mainwindows-mdi-example.html
void WindowMain::updateWindowMenu(QMdiSubWindow *mdiSubWindow) {
  if (!mSeparatorOpenWindowsAdded) { // Adding the separator on Qt Designer
    // doesn't work
    uiMenuWindow->addSeparator();
    mSeparatorOpenWindowsAdded = true;
  }
  for (auto &aAction : mSubwindowActions) {
    // 		uiMenuWindow->removeAction(mSubwindowActions->at(n)); // Makes
    // not to trigger new actions added
    aAction->setVisible(false);
  }
  mSubwindowActions.clear();

  if (mdiSubWindow != nullptr) {
    mActiveWindow = mdiSubWindow;
    mActiveWindowImage = qobject_cast<WindowImage *>(mdiSubWindow->widget());

    uiActionSaveCopyAs->setEnabled(true);
    uiActionCopy->setEnabled(true);
    mActionGroupZoom->setEnabled(true);
    mActionGroupFeatures->setEnabled(true);
    mActionGroupWindow->setEnabled(true);
    uiActionZoomIn->setEnabled(mActiveWindowImage->mCurrentFactor < 3.0);
    uiActionZoomOut->setEnabled(mActiveWindowImage->mCurrentFactor > 0.25);

    if (!mActiveWindowImage->mImageTime.isEmpty()) {
      uiActionResetImage->setEnabled(true);
      switch (mActiveWindowImage->mFeatureType) {
      case WindowImage::harris:
        mStatusBarLabelIcon->setPixmap(
            QPixmap::fromImage(QImage(":/icons/Harris.png")));
        break;
      case WindowImage::fast:
        mStatusBarLabelIcon->setPixmap(
            QPixmap::fromImage(QImage(":/icons/Fast.png")));
        break;
      case WindowImage::sift:
        mStatusBarLabelIcon->setPixmap(
            QPixmap::fromImage(QImage("icons/Sift.png")));
        break;
      case WindowImage::surf:
        mStatusBarLabelIcon->setPixmap(
            QPixmap::fromImage(QImage(":/icons/Surf.png")));
        break;
      default:break;
      }
      mStatusBarLabelIcon->setVisible(true);
      mStatusBarLabelTime->setText(mActiveWindowImage->mImageTime + " ms");
      mStatusBarLabelKeypoints->setText(mActiveWindowImage->mImageKeypoints +
          " keypoints");
      mStatusBarLine2->setVisible(true);
      mStatusBarLine3->setVisible(true);
    } else {
      uiActionResetImage->setEnabled(false);
      mStatusBarLabelIcon->setVisible(false);
      mStatusBarLabelTime->clear();
      mStatusBarLabelKeypoints->clear();
      mStatusBarLine2->setVisible(false);
      mStatusBarLine3->setVisible(false);
    }

    mStatusBarLabelZoom->setText(mActiveWindowImage->mImageZoom);
    mStatusBarLabelDimensions->setText(mActiveWindowImage->mImageDimensions);
    mStatusBarLabelSize->setText(mActiveWindowImage->mImageSize);
    mStatusBarLine->setVisible(true);

    int n = 0;
    for (auto &w : uiMdiArea->subWindowList()) {
      auto windowImage = qobject_cast<WindowImage *>(w->widget());
      QString actionName;
      actionName = tr(n < 9 ? "&%1 %2" : "%1 %2")
          .arg(n + 1)
          .arg(windowImage->windowTitle());
      n++;
      auto action = uiMenuWindow->addAction(actionName);
      mSubwindowActions.push_back(action);
      action->setCheckable(true);
      action->setChecked(uiMdiArea->activeSubWindow()
                         ? mActiveWindowImage == windowImage
                         : false);
      mActionGroupWindow->addAction(action);
      mSignalMapper->setMapping(action, w);
      QObject::connect(
          action, &QAction::triggered, mSignalMapper.get(),
          static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    }
  } else {
    if (uiMdiArea->subWindowList().empty()) {
      uiActionSaveCopyAs->setEnabled(false);
      uiActionCopy->setEnabled(false);
      mActionGroupZoom->setEnabled(false);
      mActionGroupFeatures->setEnabled(false);
      mActionGroupWindow->setEnabled(false);
      uiActionResetImage->setEnabled(false);
      uiToolBarParameters->setEnabled(false);

      mStatusBarLabelTime->clear();
      mStatusBarLabelIcon->setVisible(false);
      mStatusBarLabelKeypoints->clear();
      mStatusBarLabelZoom->clear();
      mStatusBarLabelDimensions->clear();
      mStatusBarLabelSize->clear();
      mStatusBarLine->setVisible(false);
      mStatusBarLine2->setVisible(false);
      mStatusBarLine3->setVisible(false);
    }
  }
}

void WindowMain::loadFile(const QString &filepath) {
  if (!filepath.isEmpty()) {
    if (filepath.contains(".avi") || filepath.contains(".mp4") ||
        filepath.contains(".mov")) {
      setRecentFile(filepath);
      showWindowImage(new WindowImage(filepath, filepath));
    } else {
      auto image = std::make_shared<QImage>(filepath);
      if (!image->isNull()) {
        setRecentFile(filepath);
        showWindowImage(new WindowImage(image, filepath));
      } else {
        removeRecentFile(filepath);
        QMessageBox::warning(this, tr("Image Feature Detector"),
                             tr("Cannot open %1.").arg(filepath));
      }
    }
  }
}

void WindowMain::showWindowImage(WindowImage *windowImage) {
  windowImage->mUid = ++mTotalImages;
  if (windowImage->mWindowType != WindowImage::duplicated)
    windowImage->mOriginalUid = mTotalImages;
  windowImage->setObjectName(QString(mTotalImages));
  uiMdiArea->addSubWindow(windowImage);
  windowImage->parentWidget()->setGeometry(
      0, 0, windowImage->mImage->width() + 8,
      windowImage->mImage->height() + 31); // 8 and 31 are hardcoded values for
  // the decorations of the subwindow
  if (windowImage->mImage->width() > uiMdiArea->width())
    windowImage->parentWidget()->setGeometry(
        0, 0, uiMdiArea->width(), windowImage->parentWidget()->height());
  if (windowImage->mImage->height() > uiMdiArea->height())
    windowImage->parentWidget()->setGeometry(
        0, 0, windowImage->parentWidget()->width(), uiMdiArea->height());
  windowImage->show();
  uiToolBarParameters->setEnabled(true);
}

void WindowMain::setRecentFile(const QString &filepath) {
  if (mSettings->value("rememberRecentFiles", true).toBool()) {
    QStringList files = mSettings->value("recentFiles").toStringList();
    files.removeAll(filepath);
    files.prepend(filepath);
    while (files.size() > maxRecentFiles)
      files.removeLast();
    mSettings->setValue("recentFiles", files);
    updateRecentFilesMenu();
  }
}

void WindowMain::removeRecentFile(const QString &filePath) {
  QStringList files = mSettings->value("recentFiles").toStringList();
  files.removeAll(filePath);
  mSettings->setValue("recentFiles", files);
  updateRecentFilesMenu();
}

// http://doc.qt.io/qt-5/qtwidgets-mainwindows-recentfiles-example.html
void WindowMain::updateRecentFilesMenu() {
  QStringList files = mSettings->value("recentFiles").toStringList();
  int numRecentFiles = maxRecentFiles;
  if (files.size() < maxRecentFiles)
    numRecentFiles = files.size();
  mActionSeparatorRecentFiles->setVisible(numRecentFiles > 0);
  mMenuRecentFiles->clear();
  int n = 0;
  for (const auto &file : mActionRecentFiles) {
    if (n < numRecentFiles) {
      file->setText(
          tr("&%1 %2").arg(n + 1).arg(QFileInfo(files[n]).fileName()));
      file->setData(files[n]);
      file->setVisible(true);
      mMenuRecentFiles->addAction(file.get());
    } else {
      file->setVisible(false);
    }
    n++;
  }
}

void WindowMain::openRecentFile() {
  loadFile(dynamic_cast<QAction *>(sender())->data().toString());
}

void WindowMain::setActiveSubWindow(QWidget *subWindow) {
  uiMdiArea->setActiveSubWindow(dynamic_cast<QMdiSubWindow *>(subWindow));
}

void WindowMain::closeEvent(QCloseEvent *eventConstr) {
  saveSettings();
  QWidget::closeEvent(eventConstr);
}

void WindowMain::saveSettings() {
  mSettings->setValue("maximized", isMaximized());
  if (!isMaximized()) {
    mSettings->setValue("pos", pos());
    mSettings->setValue("size", size());
  }
}
QSettings *WindowMain::getMSettings() { return mSettings.get(); }
