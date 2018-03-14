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

#include "smtk/operation/Manager.h"
#include "smtk/operation/operators/ReadResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/environment/Environment.h"

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  TemplateEditorMain mainWindow;

  if (argc >= 2)
  {
    char* fileName = argv[1];
    mainWindow.load(fileName);
  }

  mainWindow.show();

  // Attempt to load a model file so that component/resource items will have something to display,
  // which can help debug issues setting the "acceptable" item filters.
  if (argc >= 3)
  {
    char* filename = argv[2];
    auto operMgr = smtk::environment::OperationManager::instance();
    auto rdr = operMgr->create<smtk::operation::ReadResource>();
    if (rdr)
    {
      rdr->parameters()->findFile("filename")->setValue(filename);
      auto result = rdr->operate();
      if (result->findInt("outcome")->value() ==
        int(smtk::operation::Operation::Outcome::SUCCEEDED))
      {
        auto rsrc = result->findResource("resource")->value();
        auto rsrcMgr = smtk::environment::ResourceManager::instance();
        rsrcMgr->add(rsrc);
      }
    }
  }

  return app.exec();
}
