//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtOperationPreview - Standalone app for testing smtk::extensions::qtOperationDialog

#include "smtk/attribute/Registrar.h"
#include "smtk/extension/qt/qtOperationDialog.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtViewRegistrar.h"
#include "smtk/model/Registrar.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/resource/Manager.h"
#include "smtk/view/Manager.h"

// Qt includes
#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QtGlobal>

#include <set>
#include <string>

void longDescription()
{
  qInfo().nospace()
    << "Example program to execute qtOperationDialog for display purposes only.\n"
    << "Pass in one argument as the name of the operation to display.\n"
    << "A list of available operations will be dumped to the console for reference.\n"
    << "Note that this application is for **display** purposes only! Clicking the \n"
    << "\"Apply\" button will result in undefined behavior, which could include\n"
    << "the program crashing.\n";
}

void listAvailableOps(smtk::operation::ManagerPtr opManager)
{
  std::set<std::string> ops = opManager->availableOperations();
  qInfo() << "Available Operations:";
  for (const auto& opName : ops)
  {
    qInfo() << "  " << opName.c_str();
  }
}

int main(int argc, char* argv[])
{
  // Instantiate Qt application
  QApplication app(argc, argv);

  // Initialize smtk managers
  auto resManager = smtk::resource::Manager::create();
  smtk::attribute::Registrar::registerTo(resManager);
  smtk::model::Registrar::registerTo(resManager);

  auto opManager = smtk::operation::Manager::create();
  opManager->registerResourceManager(resManager);
  smtk::attribute::Registrar::registerTo(opManager);
  smtk::model::Registrar::registerTo(opManager);

  // Process command line
  QCommandLineParser parser;
  parser.setApplicationDescription("Load operation template and display editor dialog");
  parser.addHelpOption();
  parser.addPositionalArgument("operation_name", "e.g., smtk::model::GroupAuxilliaryGeometry");
  parser.process(app);

  if (!parser.parse(QCoreApplication::arguments()))
  {
    qDebug() << parser.errorText();
    return 1;
  }

  const QStringList positionalArguments = parser.positionalArguments();
  if (positionalArguments.isEmpty())
  {
    qWarning() << "\nERROR: Argument 'operation_name' missing.\n";
    longDescription();
    qInfo();
    listAvailableOps(opManager);
    qInfo() << "";
    parser.showHelp();
    return 1;
  }
  if (positionalArguments.size() > 1)
  {
    qWarning("\nERROR: More than one positional argument specified.\n");
    longDescription();
    qInfo();
    listAvailableOps(opManager);
    qInfo();
    parser.showHelp();
    return 1;
  }

  // Instantiate the operation
  const QString& opName = positionalArguments.first();
  auto op = opManager->create(opName.toStdString());
  if (op == nullptr)
  {
    qInfo() << "Unable to create operation with name" << opName;
    listAvailableOps(opManager);
    qInfo();
    return -1;
  }

  // Initialize and register managers
  auto viewManager = smtk::view::Manager::create();
  smtk::view::Registrar::registerTo(viewManager);
  smtk::extension::qtViewRegistrar::registerTo(viewManager);

  auto uiManager = QSharedPointer<smtk::extension::qtUIManager>(
    new smtk::extension::qtUIManager(op, resManager, viewManager));

  // Initialize dialog
  smtk::extension::qtOperationDialog dialog(op, uiManager, nullptr);
  int retcode = dialog.exec();
  return retcode;
}
