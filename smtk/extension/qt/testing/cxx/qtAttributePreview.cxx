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

#include "smtk/view/Base.h"
#include "smtk/view/Instanced.h"
#include "smtk/view/Root.h"

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
  if (system.rootView()->numberOfSubViews() == 0)
    {
    // Generate list of all concrete definitions in the system
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
      smtk::view::InstancedPtr view =
          smtk::view::Instanced::New((*defIter)->type());
      system.rootView()->addSubView(view);
      smtk::attribute::AttributePtr instance =
        system.createAttribute((*defIter)->type());
      view->addInstance(instance);
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

  smtk::view::RootPtr rootView = system.rootView();
  smtk::view::BasePtr view;

  // Check for input "view" argument
  if (argc <= 3)
    {
    if (rootView->numberOfSubViews() > 0)
      {
      // Generate tab group with all views (standard)
      uiManager->initializeUI(widget, useInternalFileBrowser);
      }
    else
      {
      // If rootView is empty, generate 1 instanced view w/all attdefs
      std::cout << "Creating default view" << std::endl;
      smtk::view::InstancedPtr instanced = smtk::view::Instanced::New("Default");

      std::vector<smtk::attribute::DefinitionPtr> defList;
      system.findBaseDefinitions(defList);
      for (size_t i=0; i<defList.size(); ++i)
        {
        smtk::attribute::DefinitionPtr defn = defList[i];
        if (defn->isAbstract())
          {
          // For abstract definitions, retrieve all derived & concrete defs
          std::vector<smtk::attribute::DefinitionPtr> derivedList;
          system.findAllDerivedDefinitions(defn, true, derivedList);
          for (size_t j=0; j<derivedList.size(); ++j)
            {
            std::string attType = derivedList[j]->type();
            smtk::attribute::AttributePtr att = system.createAttribute(attType);
            instanced->addInstance(att);
            }
          }
        else
          {
          std::string attType = defn->type();
          smtk::attribute::AttributePtr att = system.createAttribute(attType);
          instanced->addInstance(att);
          }
        }
      std::cout << "Number of atts added to default view: "
                << instanced->numberOfInstances() << std::endl;

      view = instanced;
      rootView->addSubView(view);
      }
    }
  else
    {
    // Render one view (experimental)
    // First check if argv[3] is index or name
    std::string input = argv[3];
    try
      {
      int index = boost::lexical_cast<int>(input);
      view = rootView->subView(index);
      }
    catch (const boost::bad_lexical_cast &)
      {
      // If input not an int, check for view with matching title
      for (std::size_t i=0; i<rootView->numberOfSubViews(); ++i)
        {
        if (rootView->subView(i)->title() == input)
          {
          view = rootView->subView(i);
          break;
          }
        }
      }

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
