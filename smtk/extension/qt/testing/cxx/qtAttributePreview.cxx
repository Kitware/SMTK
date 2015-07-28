//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME qtAttributePreview - Standalone test program for qtRootView instances
// .SECTION Description
// .SECTION See Also

#include "smtk/extension/qt/qtRootView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include "smtk/common/View.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/System.h"

#include "smtk/model/Manager.h"

#include <QApplication>
#include <QFrame>
#include <QVBoxLayout>
#include <QWidget>

#ifndef _MSC_VER
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored"-Wunused-parameter"
#endif
#include <boost/lexical_cast.hpp>
#ifndef _MSC_VER
#  pragma GCC diagnostic pop
#endif

#include <iostream>

int main(int argc, char *argv[])
{
  if (argc < 2)
    {
    std::cout << "\n"
              << "Simple program to load attribute system and display corresponding editor panel" << "\n"
              << "Usage: qtAttributePreview attribute_filename  [output_filename]"
              << "  [view_name | view_number]" << "\n"
              << std::endl;
    return -1;
    }

  // Instantiate and load attribute system
  smtk::attribute::System system;
  char *inputPath = argv[1];
  std::cout << "Loading simulation file: " << inputPath << std::endl;
  smtk::io::AttributeReader reader;
  smtk::io::Logger inputLogger;
  bool  err = reader.read(system, inputPath, true, inputLogger);
  if (err)
    {
    std::cout << "Error loading simulation file -- exiting" << "\n";
    std::cout << inputLogger.convertToString() << std::endl;
    return -2;
    }

  // If system contains no views, create InstancedView by default
  // assume there is at most one root type view
  smtk::common::ViewPtr root = system.findViewByType("Root");
  
  if (!root)
    {
    root = smtk::common::View::New("Root", "RootView");
    root->details().setAttribute("TopLevel", "true");
    system.addView(root);
    }
  // If the top view is empty then
  int viewsIndex = root->details().findChild("Views");
  if (viewsIndex < 0)
    {
    smtk::common::View::Component temp = root->details().addChild("Views");
    viewsIndex = root->details().findChild("Views");
    }

  // If there the number of child views  is 0 then  lets add instances of all
  // non-abstract attributE definitions
  smtk::common::View::Component viewsComp = root->details().child(viewsIndex);
  if (!viewsComp.numberOfChildren())
    {
    std::vector<smtk::attribute::DefinitionPtr> defs;
    std::vector<smtk::attribute::DefinitionPtr> baseDefinitions;
    system.findBaseDefinitions(baseDefinitions);
    std::vector<smtk::attribute::DefinitionPtr>::const_iterator baseIter;
    
    for (baseIter = baseDefinitions.begin();
         baseIter != baseDefinitions.end();
         baseIter++)
      {
      // Add def if not abstract
      if (!(*baseIter)->isAbstract())
        {
        //defs.push_back(*baseIter);
        }
      
      std::vector<smtk::attribute::DefinitionPtr> derivedDefs;
      system.findAllDerivedDefinitions(*baseIter, true, derivedDefs);
      defs.insert(defs.end(), derivedDefs.begin(), derivedDefs.end());
      }
    
    // Instantiate attribute for each concrete definition
    std::vector<smtk::attribute::DefinitionPtr>::const_iterator defIter;
    for (defIter = defs.begin(); defIter != defs.end(); defIter++)
      {
      smtk::common::ViewPtr instanced = smtk::common::View::New((*defIter)->type(),
                                                                "Instanced");
      smtk::common::View::Component &comp =
        instanced->details().addChild("InstancedAttributes").addChild("Att");
      comp.setAttribute("Type", (*defIter)->type());
      system.addView(instanced);
      smtk::attribute::AttributePtr instance =
        system.createAttribute((*defIter)->type());
      comp.setContents(instance->name());
      viewsComp.addChild("View").setAttribute("Title", (*defIter)->type());
      }
    }

  smtk::model::ManagerPtr modelManager = system.refModelManager();

  // Instantiate Qt application
  QApplication *app = new QApplication(argc, argv);

  // Instantiate smtk's qtUIManager
  smtk::attribute::qtUIManager *uiManager =
    new smtk::attribute::qtUIManager(system);

  // Instantiate empty widget as containter for qtUIManager
  QWidget *widget = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout();
  widget->setLayout(layout);

  bool useInternalFileBrowser = true;

  smtk::common::ViewPtr view;

  // Check for input "view" argument
  if (argc <= 3)
    {
    // Generate tab group with all views (standard)
    uiManager->setSMTKView(root, widget, useInternalFileBrowser);
    }
  else
    {
    // Render one view (experimental)
    // First check if argv[3] isa name
    std::string input = argv[3];
    view = system.findView(input);
    if (!view)
      {
      std::cout << "ERROR: View \"" << input << "\" not found" << std::endl;
      return -4;
      }
    }

  if (view)
    {
    // Add simple panel (QFrame) for aesthetics
    QFrame *frame = new QFrame();
    frame->setFrameShadow(QFrame::Raised);
    frame->setFrameShape(QFrame::Panel);
    //frame->setStyleSheet("QFrame { background-color: pink; }");

    QVBoxLayout *frameLayout = new QVBoxLayout();
    frame->setLayout(frameLayout);
    uiManager->setSMTKView(view, frame, useInternalFileBrowser);
    widget->layout()->addWidget(frame);
    }

  widget->show();
//  uiManager->rootView()->showAdvanceLevel(0);
  int retcode = app->exec();

  if (argc > 2)
    {
    char *outputPath = argv[2];
    std::cout << "Writing resulting simulation file: " << outputPath << std::endl;
    smtk::io::AttributeWriter writer;
    smtk::io::Logger outputLogger;
    bool  outputErr = writer.write(system, outputPath, outputLogger);
    if (outputErr)
      {
      std::cout << "Error writing simulation file -- exiting" << "\n"
                << outputLogger.convertToString() << std::endl;
      return -3;
      }
    }

  // Done
  return retcode;
}
