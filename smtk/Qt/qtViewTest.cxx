/*=========================================================================

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME qtViewTest - Standalone test program for qtRootView instances
// .SECTION Description
// .SECTION See Also

#include "smtk/attribute/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/Qt/qtUIManager.h"
#include "smtk/util/AttributeReader.h"
#include "smtk/util/Logger.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include <iostream>

int main(int argc, char *argv[])
{
  std::cout << "Enter" << std::endl;

  if (argc < 2)
    {
    std::cout << "\n"
              << "Usage qtViewTest attribute_filename"
              << "\n" << std::endl;
    return -1;
    }

  // Initialize attribute manager
  smtk::attribute::Manager manager;
  char *sbi_path = argv[1];
  std::cout << "Loading simulation file: " << sbi_path << std::endl;
  smtk::util::AttributeReader reader;
  smtk::util::Logger logger;
  bool  err = reader.read(manager, sbi_path, logger);
  if (err)
    {
    std::cout << "Error loading simulation file -- exiting" << std::endl;
    return -2;
    }

  // Instantiate (empty) model
  smtk::model::ModelPtr model =
    smtk::model::ModelPtr(new smtk::model::Model());
  manager.setRefModel(model);

  // Instantiate Qt application
  QApplication *qapp = new QApplication(argc, argv);

  // Instantiate smtk's qtUIManager
  smtk::attribute::qtUIManager *uiManager =
    new smtk::attribute::qtUIManager(manager);

  // Instantiate empty widget as containter for qtUIManager
  QWidget *widget = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout();
  widget->setLayout(layout);

  uiManager->initializeUI(widget);
  widget->show();
  int retcode = qApp->exec();

  // Done
  std::cout << "Exit " << retcode << std::endl;
  return retcode;
}
