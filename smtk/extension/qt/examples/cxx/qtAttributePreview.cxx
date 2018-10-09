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

#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include "smtk/view/View.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Resource.h"

#include <QApplication>
#include <QFrame>
#include <QVBoxLayout>
#include <QWidget>

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/lexical_cast.hpp>
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

#include <iostream>

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cout << "\n"
              << "Simple program to load attribute resource and display corresponding editor panel"
              << "\n"
              << "Usage: qtAttributePreview attribute_filename  [output_filename]"
              << "  [view_name | view_number]"
              << "\n"
              << std::endl;
    return -1;
  }

  // Instantiate and load attribute resource
  smtk::attribute::ResourcePtr resource = smtk::attribute::Resource::create();
  char* inputPath = argv[1];
  std::cout << "Loading simulation file: " << inputPath << std::endl;
  smtk::io::AttributeReader reader;
  smtk::io::Logger inputLogger;
  bool err = reader.read(resource, inputPath, true, inputLogger);
  if (err)
  {
    std::cout << "Error loading simulation file -- exiting"
              << "\n";
    std::cout << inputLogger.convertToString() << std::endl;
    return -2;
  }

  // If resource contains no views, create InstancedView by default
  // assume there is at most one root type view
  smtk::view::ViewPtr root = resource->findTopLevelView();

  if (!root)
  {
    root = smtk::view::View::New("Group", "RootView");
    root->details().setAttribute("TopLevel", "true");
    resource->addView(root);
    smtk::view::View::Component& temp = root->details().addChild("Views");
    (void)temp;
    int viewsIndex = root->details().findChild("Views");

    //  Lets add instances of all
    // non-abstract attributE definitions
    smtk::view::View::Component& viewsComp = root->details().child(viewsIndex);
    std::vector<smtk::attribute::DefinitionPtr> defs;
    std::vector<smtk::attribute::DefinitionPtr> baseDefinitions;
    resource->findBaseDefinitions(baseDefinitions);
    std::vector<smtk::attribute::DefinitionPtr>::const_iterator baseIter;

    for (baseIter = baseDefinitions.begin(); baseIter != baseDefinitions.end(); baseIter++)
    {
      // Add def if not abstract
      if (!(*baseIter)->isAbstract())
      {
        //defs.push_back(*baseIter);
      }

      std::vector<smtk::attribute::DefinitionPtr> derivedDefs;
      resource->findAllDerivedDefinitions(*baseIter, true, derivedDefs);
      defs.insert(defs.end(), derivedDefs.begin(), derivedDefs.end());
    }

    // Instantiate attribute for each concrete definition
    std::vector<smtk::attribute::DefinitionPtr>::const_iterator defIter;
    for (defIter = defs.begin(); defIter != defs.end(); defIter++)
    {
      smtk::view::ViewPtr instanced = smtk::view::View::New("Instanced", (*defIter)->type());

      smtk::view::View::Component& comp =
        instanced->details().addChild("InstancedAttributes").addChild("Att");
      comp.setAttribute("Type", (*defIter)->type());
      comp.setAttribute("Name", (*defIter)->type());
      resource->addView(instanced);
      smtk::attribute::AttributePtr instance = resource->createAttribute((*defIter)->type());
      comp.setContents(instance->name());
      viewsComp.addChild("View").setAttribute("Title", (*defIter)->type());
    }
  }

  // Instantiate Qt application
  QApplication* app = new QApplication(argc, argv);

  // Instantiate smtk's qtUIManager
  smtk::extension::qtUIManager* uiManager = new smtk::extension::qtUIManager(resource);

  // Instantiate empty widget as containter for qtUIManager
  QWidget* widget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout();
  widget->setLayout(layout);

  bool useInternalFileBrowser = true;

  smtk::view::ViewPtr view;

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
    view = resource->findView(input);
    if (!view)
    {
      std::cout << "ERROR: View \"" << input << "\" not found" << std::endl;
      return -4;
    }
  }

  if (view)
  {
    // Add simple panel (QFrame) for aesthetics
    QFrame* frame = new QFrame();
    frame->setFrameShadow(QFrame::Raised);
    frame->setFrameShape(QFrame::Panel);
    //frame->setStyleSheet("QFrame { background-color: pink; }");

    QVBoxLayout* frameLayout = new QVBoxLayout();
    frame->setLayout(frameLayout);
    uiManager->setSMTKView(view, frame, useInternalFileBrowser);
    widget->layout()->addWidget(frame);
  }

  widget->show();
  //  uiManager->rootView()->showAdvanceLevel(0);
  int retcode = app->exec();
  QCoreApplication::processEvents();
  if (argc > 2)
  {
    char* outputPath = argv[2];
    std::cout << "Writing resulting simulation file: " << outputPath << std::endl;
    smtk::io::AttributeWriter writer;
    smtk::io::Logger outputLogger;
    bool outputErr = writer.write(resource, outputPath, outputLogger);
    if (outputErr)
    {
      std::cout << "Error writing simulation file -- exiting"
                << "\n"
                << outputLogger.convertToString() << std::endl;
      return -3;
    }
  }

  // Done
  return retcode;
}
