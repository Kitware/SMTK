//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <QApplication>

#include "TemplateEditorMain.h"

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  TemplateEditorMain mainWindow;

  if (argc == 2)
  {
    char* fileName = argv[1];
    mainWindow.load(fileName);
  }

  mainWindow.show();

  return app.exec();
}
