//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/rgg/qt/nuclides/NuclideTable.h"

#include <QApplication>
#include <QGraphicsView>

#include <iostream>

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

  smtk::bridge::rgg::NuclideTable window;

  auto printNuclideName = [](smtk::bridge::rgg::Nuclide* nuclide) {
    std::cout << nuclide->name().toStdString() << std::endl;
  };

  QEventLoop::connect(&window, &smtk::bridge::rgg::NuclideTable::nuclideSelected, printNuclideName);

  window.show();
  window.resize(500, 500);
  window.scene()->views()[0]->centerOn(1000, 1000);

  return app.exec();
}
