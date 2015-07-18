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
              << "  [view_title]" << "\n"
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

  // Check for view to display
  const std::map<std::string, smtk::common::ViewPtr>& views = system.views();
  smtk::common::ViewPtr root;
  // Check for input "view" argument
  if (argc > 3)
    {
    std::string input = argv[3];
    root = system.findView(input);
    if (!root)
      {
      std::cout << "ERROR: View \"" << input << "\" not found" << std::endl;
      return -4;
      }
    }

  // If there is a single view, display that
  else if (views.size() == 1)
    {
    std::map<std::string, smtk::common::ViewPtr>::const_iterator iter = views.begin();
    root = iter->second;
    }

  // Else look for a RootView
  else
    {
    root = system.findViewByType("Root");
    }

  if (!root && views.size() > 1)
    {
    std::cout << "ERROR: Cannot figure out which view to display."
              << "\n" << "Use 3rd argument to specify 1 view by its title."
              << std::endl;
    return -5;
    }

  if (!root && views.size() == 0)
    {
    if (system.hasAttributes())
      {
      std::cout
        << "ERROR: Input file has attributes but no View elements"
        << " - cannot display." << std::endl;
      return -3;
      }

    // (else) create default instanced view
    std::cout << "No views, so creating instanced view" << std::endl;
    root = smtk::common::View::New("Instanced", "Default");
    smtk::common::View::Component &instancedComp =
      root->details().addChild("InstancedAttributes");

    std::vector<smtk::attribute::DefinitionPtr> defs;
    std::vector<smtk::attribute::DefinitionPtr> baseDefinitions;
    system.findBaseDefinitions(baseDefinitions);
    std::vector<smtk::attribute::DefinitionPtr>::const_iterator baseIter;
    for (baseIter = baseDefinitions.begin();
         baseIter != baseDefinitions.end();
         baseIter++)
      {
      std::vector<smtk::attribute::DefinitionPtr> derivedDefs;
      system.findAllDerivedDefinitions(*baseIter, true, derivedDefs);
      defs.insert(defs.end(), derivedDefs.begin(), derivedDefs.end());
      }

    // Add view component for each concrete definition
    std::vector<smtk::attribute::DefinitionPtr>::const_iterator defIter;
    for (defIter = defs.begin(); defIter != defs.end(); defIter++)
      {
      smtk::attribute::DefinitionPtr def = *defIter;
      smtk::common::View::Component& attComp =instancedComp.addChild("Att");
      attComp.setAttribute("Name", def->type());
      attComp.setAttribute("Type", def->type());
      }  // for (defIter)

    system.addView(root);
    }  // if

  smtk::model::ManagerPtr modelManager = system.refModelManager();

  // Instantiate Qt application
  QApplication *app = new QApplication(argc, argv);

  // Instantiate smtk's qtUIManager
  smtk::attribute::qtUIManager *uiManager =
    new smtk::attribute::qtUIManager(system, root->title());

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
    uiManager->initializeUI(widget, useInternalFileBrowser);
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
    uiManager->initializeView(frame, view, useInternalFileBrowser);
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
