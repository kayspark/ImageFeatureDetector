/*
 * File:   main.cpp
 * Author: chucknorris
 *
 * Created on 05 December 2011, 03:31
 */

#include "windowMain.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QStringList argList = QCoreApplication::arguments();

  WindowMain w;

  // Opções de abrir a aplicação: Maximizado ou FullScreen
  if (argList.contains("--fullscreen")) {
    w.showFullScreen();
  } else if (argList.contains("--maximized")) {
    w.showMaximized();
  } else if (argList.contains("--help")) {
    std::cout << "Parameters:\n\t"
                 "--fullscreen:\tOpen Screen in mode FullScreen\n\t"
                 "--maximized:\tOpen Screen in mode Maximized"
                 ""
              << std::endl;
    exit(0);
  } else {
    w.show();
  }

  return app.exec();
}
