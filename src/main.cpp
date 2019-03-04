/*
 * File:   main.cpp
 * Author: kspark@nepes.co.kr and  chucknorris
 *
 * copyright(c) 2018,2019 nepes.co.kr
 * Created on 05 December 2011, 03:31
 */

#include "NeuroMemEngine.hpp"
#include "windowMain.hpp"
#include <QApplication>

// http://stackoverflow.com/questions/15035767/is-the-qt-5-dark-fusion-theme-available-for-windows
void applyDarkTheme(QApplication &app) {
  // set style
  app.setStyle(QStyleFactory::create("Fusion"));
  // increase font size for better reading
  QFont defaultFont = QApplication::font();
  defaultFont.setPointSize(defaultFont.pointSize() + 2);
  app.setFont(defaultFont);
  // modify palette to dark
  QPalette darkPalette;
  darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::WindowText, Qt::white);
  darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
  darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
  darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
  darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
  darkPalette.setColor(QPalette::ToolTipText, Qt::white);
  darkPalette.setColor(QPalette::Text, Qt::white);
  darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
  darkPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
  darkPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
  darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::ButtonText, Qt::white);
  darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
  darkPalette.setColor(QPalette::BrightText, Qt::red);
  darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
  darkPalette.setColor(QPalette::HighlightedText, Qt::white);
  darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));
  app.setPalette(darkPalette);
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QStringList argList = QCoreApplication::arguments();

#ifdef _WIN32
  using nm_device = NeuroMem::NeuroMemDevice;
  using nm_engine = NeuroMem::NeuroMemEngine;
  nm_device ds{};
  const uint16_t detected_device = nm_engine::GetDevices(&ds, 1);
#else
  nm_device ds;
  uint16_t detected_device = nm_get_devices(&ds, 1);
#endif //_WIN32
  if (detected_device < 1) {
    std::cout << "NM devices are not connected. please make sure connection." << std::endl;
    std::cout << "Please refer http://www.theneuromorphic.com to get the device" << std::endl;
    system("pause");
    return (0);
  }

  // make backup dir to make sure
  QDir dir;
  dir.mkdir("backup");
  WindowMain w;
  // apply dark theme
  applyDarkTheme(app);
  if (argList.contains("--fullscreen")) {
    w.showFullScreen();
  } else if (argList.contains("--maximized")) {
    w.showMaximized();
  } else if (argList.contains("--help")) {
    std::cout << "Parameters:\n\t"
                 "--fullscreen:\tOpen Screen in mode FullScreen\n\t"
                 "--maximized:\tOpen Screen in mode Maximized"
              << std::endl;
    exit(0);
  } else {
    w.show();
  }
  return app.exec();
}
