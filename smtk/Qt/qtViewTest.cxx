/*=========================================================================

Copyright (c) 1998-2013 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME qtViewTest - Standalone test program for qtRootView instances
// .SECTION Description
// .SECTION See Also

#include "smtk/attribute/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/Qt/qtRootView.h"
#include "smtk/Qt/qtUIManager.h"
#include "smtk/util/AttributeReader.h"
#include "smtk/util/AttributeWriter.h"
#include "smtk/util/Logger.h"
#include "smtk/view/Base.h"
#include "smtk/view/Root.h"

#include <QApplication>
#include <QFrame>
#include <QVBoxLayout>
#include <QWidget>

#include <boost/lexical_cast.hpp>

#include <iostream>

int main(int argc, char *argv[])
{
  if (argc < 2)
    {
    std::cout << "\n"
              << "Simple program to load attribute manager and display corresponding editor panel" << "\n"
              << "Usage: qtViewTest attribute_filename  [output_filename]"
              << "  [view_name | view_number]" << "\n"
              << std::endl;
    return -1;
    }

  // Instantiate and load attribute manager
  smtk::attribute::Manager manager;
  char *inputPath = argv[1];
  std::cout << "Loading simulation file: " << inputPath << std::endl;
  smtk::util::AttributeReader reader;
  smtk::util::Logger inputLogger;
  bool  err = reader.read(manager, inputPath, inputLogger);
  if (err)
    {
    std::cout << "Error loading simulation file -- exiting" << "\n";
    std::cout << inputLogger.convertToString() << std::endl;
    return -2;
    }

  // Instantiate (empty) model
  smtk::model::ModelPtr model =
    smtk::model::ModelPtr(new smtk::model::Model());
  manager.setRefModel(model);

  // Instantiate Qt application
  QApplication *app = new QApplication(argc, argv);

  // Instantiate smtk's qtUIManager
  smtk::attribute::qtUIManager *uiManager =
    new smtk::attribute::qtUIManager(manager);

  // Instantiate empty widget as containter for qtUIManager
  QWidget *widget = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout();
  widget->setLayout(layout);

  bool useInternalFileBrowser = true;

  // Check for input "view" argument
  if (argc <= 3)
    {
    // Generate tab group with all views (standard)
    uiManager->initializeUI(widget, useInternalFileBrowser);
    }
  else
    {
    // Render one view (experimental)
    smtk::view::RootPtr rootView = manager.rootView();
    smtk::view::BasePtr view;
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
  uiManager->rootView()->onShowAdvanced(0);
  int retcode = app->exec();

  if (argc > 2)
    {
    char *outputPath = argv[2];
    std::cout << "Writing resulting simulation file: " << outputPath << std::endl;
    smtk::util::AttributeWriter writer;
    smtk::util::Logger outputLogger;
    bool  outputErr = writer.write(manager, outputPath, outputLogger);
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
