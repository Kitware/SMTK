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
#include "smtk/Qt/qtUIManager.h"
#include "smtk/util/AttributeReader.h"
#include "smtk/util/Logger.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include <iostream>

int main(int argc, char *argv[])
{
  if (argc < 2)
    {
    std::cout << "\n"
              << "Simple program to load attribute manager and display corresponding editor panel" << "\n"
              << "Usage: qtViewTest attribute_filename" << "\n"
              << std::endl;
    return -1;
    }

  // Instantiate and load attribute manager
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
  QApplication *app = new QApplication(argc, argv);

  // Instantiate smtk's qtUIManager
  smtk::attribute::qtUIManager *uiManager =
    new smtk::attribute::qtUIManager(manager);

  // Instantiate empty widget as containter for qtUIManager
  QWidget *widget = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout();
  widget->setLayout(layout);

  uiManager->initializeUI(widget);
  widget->show();
  int retcode = app->exec();

  // Done
  return retcode;
}
