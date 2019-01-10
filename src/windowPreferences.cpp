/*
 * 2018(C) kayspark , 2010-2015 (C) Antonio Redondo
 *
 * https://github.com/kayspark/ImageFeatureDetector
 *
 * Code under the terms of the GNU General Public License v3.
 *
 */

#include "windowPreferences.h"

WindowPreferences::WindowPreferences(WindowMain *windowMain)
    : QDialog::QDialog(windowMain, Qt::Dialog)
    , mWindowMain(windowMain) {
  setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);
  if (mWindowMain->getMSettings()->value("recentFiles").toStringList().isEmpty()) {
    uiPushButtonClearRecentFiles->setEnabled(false);
    uiPushButtonClearRecentFiles->setText("Recent Files List Is Empty");
  }
  uiCheckBoxStartupDialog->setChecked(mWindowMain->getMSettings()->value("startupDialog", true).toBool());
  uiCheckBoxBestFit->setChecked(mWindowMain->getMSettings()->value("bestFit", true).toBool());
  uiCheckBoxRecentFiles->setChecked(mWindowMain->getMSettings()->value("rememberRecentFiles", true).toBool());

  QObject::connect(uiPushButtonClearRecentFiles, &QAbstractButton::clicked, this,
                   &WindowPreferences::clearRecentFilesPrompt);
  QObject::connect(uiDialogButtonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this,
                   &WindowPreferences::restore);
  QObject::connect(uiDialogButtonBox, &QDialogButtonBox::accepted, this, &WindowPreferences::save);
  QObject::connect(uiDialogButtonBox, &QDialogButtonBox::rejected, this, &WindowPreferences::close);

  show();
}

void WindowPreferences::clearRecentFilesPrompt() {
  if (QMessageBox::Yes ==
      QMessageBox::warning(this, tr("Image Feature Detector"), tr("Do you want to clear the Recent Files List?"),
                           QMessageBox::Yes | QMessageBox::Cancel))
    clearRecentFiles();
}

void WindowPreferences::clearRecentFiles() {
  QStringList files = mWindowMain->getMSettings()->value("recentFiles").toStringList();
  files.clear();
  mWindowMain->getMSettings()->setValue("recentFiles", files);
  uiPushButtonClearRecentFiles->setEnabled(false);
  uiPushButtonClearRecentFiles->setText("Recent File List Cleared");
  mWindowMain->updateRecentFilesMenu();
}

void WindowPreferences::restore() {
  uiCheckBoxStartupDialog->setChecked(true);
  uiCheckBoxBestFit->setChecked(true);
  uiCheckBoxRecentFiles->setChecked(true);
}

void WindowPreferences::save() {
  if (!uiCheckBoxRecentFiles->isChecked())
    clearRecentFiles();
  mWindowMain->getMSettings()->setValue("rememberRecentFiles", uiCheckBoxRecentFiles->isChecked());
  mWindowMain->getMSettings()->setValue("bestFit", uiCheckBoxBestFit->isChecked());
  mWindowMain->getMSettings()->setValue("startupDialog", uiCheckBoxStartupDialog->isChecked());
  close();
}
