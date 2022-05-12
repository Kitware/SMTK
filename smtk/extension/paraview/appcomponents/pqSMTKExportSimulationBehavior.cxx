//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKExportSimulationBehavior.h"

// Client side
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqFileDialog.h"
#include "pqServer.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/extension/qt/qtOperationDialog.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/operators/ImportPythonOperation.h"

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QGridLayout>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QPushButton>
#include <QSharedPointer>
#include <QSpacerItem>
#include <QWindow>

pqExportSimulationReaction::pqExportSimulationReaction(QAction* parentObject)
  : Superclass(parentObject)
{
  // In order to export a simulation, there must at least be an active server.
  pqActiveObjects* activeObjects = &pqActiveObjects::instance();
  QObject::connect(
    activeObjects, SIGNAL(serverChanged(pqServer*)), this, SLOT(updateEnableState()));
  this->updateEnableState();
}

void pqExportSimulationReaction::updateEnableState()
{
  pqServer* server = pqActiveObjects::instance().activeServer();
  bool enable_state =
    (server != nullptr && pqSMTKBehavior::instance()->resourceManagerForServer(server) != nullptr);
  this->parentAction()->setEnabled(enable_state);
}

void pqExportSimulationReaction::exportSimulation()
{
  // Access the active server
  pqServer* server = pqActiveObjects::instance().activeServer();

  // Construct a file dialog for importing python operations
  pqFileDialog fileDialog(
    server, pqCoreUtilities::mainWidget(), tr("SMTK export File:"), "", "Python files (*.py)");
  fileDialog.setObjectName("SimulationExportDialog");
  fileDialog.setFileMode(pqFileDialog::ExistingFile);

  if (fileDialog.exec() == QDialog::Accepted)
  {
    QString fname = fileDialog.getSelectedFiles()[0];

    // Access the server's operation manager (held by pqSMTKWrapper)
    pqSMTKWrapper* wrapper = pqSMTKBehavior::instance()->resourceManagerForServer(server);

    // Construct an operation for importing Python operations
    auto importPythonOp =
      wrapper->smtkOperationManager()->create<smtk::operation::ImportPythonOperation>();

    if (importPythonOp == nullptr)
    {
      std::cerr << "Could not create \"import python operation\"\n";
      return;
    }

    // Set the input python operation file name
    importPythonOp->parameters()->findFile("filename")->setValue(fname.toStdString());

    smtk::operation::Operation::Result result;
    try
    {
      // Execute the operation
      result = importPythonOp->operate();
    }
    catch (std::exception& e)
    {
      smtkErrorMacro(smtk::io::Logger::instance(), e.what());
      return;
    }

    // Test the results for success
    if (
      result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(), "\"import python operation\" operation failed\n");
      return;
    }

    // Access the unique name associated with the operation.
    auto exportOp =
      wrapper->smtkOperationManager()->create(result->findString("unique_name")->value());

    // Construct a modal dialog for the operation spec
    auto exportDialog =
      QSharedPointer<smtk::extension::qtOperationDialog>(new smtk::extension::qtOperationDialog(
        exportOp,
        wrapper->smtkResourceManager(),
        wrapper->smtkViewManager(),
        pqCoreUtilities::mainWidget()));
    exportDialog->setObjectName("SimulationExportDialog");
    exportDialog->setWindowTitle("Simulation Export Dialog");

    // Alert the user if the operation fails. Close the dialog if the operation
    // succeeds.
    QObject::connect(
      exportDialog.get(),
      &smtk::extension::qtOperationDialog::operationExecuted,
      [=](const smtk::operation::Operation::Result& result) {
        if (
          result->findInt("outcome")->value() !=
          static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
        {
          QMessageBox msgBox;
          msgBox.setStandardButtons(QMessageBox::Ok);
          // Create a spacer so it doesn't look weird
          QSpacerItem* horizontalSpacer =
            new QSpacerItem(300, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
          msgBox.setText("Export failed. Please see output log for more details.");
          QGridLayout* layout = (QGridLayout*)msgBox.layout();
          layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
          msgBox.exec();
        }
      });
    exportDialog->exec();

    // Remove the export operation from the operation manager.
    wrapper->smtkOperationManager()->unregisterOperation(
      result->findString("unique_name")->value());
  }
}

namespace
{
QAction* findExitAction(QMenu* menu)
{
  Q_FOREACH (QAction* action, menu->actions())
  {
    QString name = action->text().toLower();
    name.remove('&');
    if (name == "exit" || name == "quit")
    {
      return action;
    }
  }
  return nullptr;
}

QAction* findHelpMenuAction(QMenuBar* menubar)
{
  QList<QAction*> menuBarActions = menubar->actions();
  Q_FOREACH (QAction* existingMenuAction, menuBarActions)
  {
    QString menuName = existingMenuAction->text().toLower();
    menuName.remove('&');
    if (menuName == "help")
    {
      return existingMenuAction;
    }
  }
  return nullptr;
}
} // namespace

static pqSMTKExportSimulationBehavior* g_instance = nullptr;

pqSMTKExportSimulationBehavior::pqSMTKExportSimulationBehavior(QObject* parent)
  : Superclass(parent)
{
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    QObject::connect(pqCore, &pqApplicationCore::clientEnvironmentDone, [this]() {
      QAction* exportSimulationAction = new QAction(tr("&Export Simulation..."), this);

      QMainWindow* mainWindow = qobject_cast<QMainWindow*>(pqCoreUtilities::mainWidget());

      // Stop here if main window does not exist
      // This is typically the case for certain unit tests.
      if (!mainWindow)
      {
        return;
      }

      QList<QAction*> menuBarActions = mainWindow->menuBar()->actions();

      QMenu* menu = nullptr;
      Q_FOREACH (QAction* existingMenuAction, menuBarActions)
      {
        QString menuName = existingMenuAction->text();
        menuName.remove('&');
        if (menuName == "File")
        {
          menu = existingMenuAction->menu();
          break;
        }
      }

      if (menu)
      {
        QAction* exitAction = findExitAction(menu);

        if (exitAction == nullptr)
        {
          menu->addSeparator();
        }

        menu->insertAction(exitAction, exportSimulationAction);

        if (exitAction != nullptr)
        {
          menu->insertSeparator(exitAction);
        }
      }
      else
      {
        // If the File menu doesn't already exist, I don't think the following
        // logic works. It is taken from pqPluginActionGroupBehavior, which
        // is designed to accomplish pretty much the same task, though.

        // Create new menu.
        menu = new QMenu("File", mainWindow);
        menu->setObjectName("File");
        menu->addAction(exportSimulationAction);
        // insert new menus before the Help menu is possible.
        mainWindow->menuBar()->insertMenu(::findHelpMenuAction(mainWindow->menuBar()), menu);
      }
      new pqExportSimulationReaction(exportSimulationAction);
    });
  }
}

pqSMTKExportSimulationBehavior* pqSMTKExportSimulationBehavior::instance(QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKExportSimulationBehavior(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

pqSMTKExportSimulationBehavior::~pqSMTKExportSimulationBehavior()
{
  if (g_instance == this)
  {
    g_instance = nullptr;
  }

  QObject::disconnect(this);
}
