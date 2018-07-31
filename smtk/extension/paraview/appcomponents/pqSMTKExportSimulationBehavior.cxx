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
#include "smtk/extension/paraview/appcomponents/pqSMTKOperationPanel.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/operators/ImportPythonOperation.h"

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QSharedPointer>

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

    // Execute the operation
    auto result = importPythonOp->operate();

    // Test the results for success
    if (result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "\"import python operation\" operation failed\n";
      return;
    }

    // Access the unique name associated with the operation.
    auto exportOp =
      wrapper->smtkOperationManager()->create(result->findString("unique_name")->value());

    // Construct a modal dialog for the operation.
    QSharedPointer<QDialog> exportDialog = QSharedPointer<QDialog>(new QDialog());
    exportDialog->setObjectName("SimulationExportDialog");
    exportDialog->setWindowTitle("Simulation Export Dialog");
    exportDialog->setLayout(new QVBoxLayout(exportDialog.data()));

    // Create a new UI for the dialog.
    QSharedPointer<smtk::extension::qtUIManager> uiManager =
      QSharedPointer<smtk::extension::qtUIManager>(new smtk::extension::qtUIManager(exportOp));

    // Create an operation view for the operation.
    smtk::view::ViewPtr view = uiManager->findOrCreateOperationView();
    smtk::extension::qtOperationView* opView = dynamic_cast<smtk::extension::qtOperationView*>(
      uiManager->setSMTKView(view, exportDialog.data()));

    // Close the modal dialog when the operation is executed.
    connect(opView, &smtk::extension::qtOperationView::operationRequested,
      [=]() { exportDialog->done(QDialog::Accepted); });

    // Launch the modal dialog and wait for the operation to succeed.
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
  foreach (QAction* action, menu->actions())
  {
    QString name = action->text().toLower();
    name.remove('&');
    if (name == "exit" || name == "quit")
    {
      return action;
    }
  }
  return NULL;
}

QAction* findHelpMenuAction(QMenuBar* menubar)
{
  QList<QAction*> menuBarActions = menubar->actions();
  foreach (QAction* existingMenuAction, menuBarActions)
  {
    QString menuName = existingMenuAction->text().toLower();
    menuName.remove('&');
    if (menuName == "help")
    {
      return existingMenuAction;
    }
  }
  return NULL;
}
}

static pqSMTKExportSimulationBehavior* g_instance = nullptr;

pqSMTKExportSimulationBehavior::pqSMTKExportSimulationBehavior(QObject* parent)
  : Superclass(parent)
{
  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    QAction* exportSimulationAction = new QAction(tr("&Export Simulation..."), this);

    QMainWindow* mainWindow = qobject_cast<QMainWindow*>(pqCoreUtilities::mainWidget());

    QList<QAction*> menuBarActions = mainWindow->menuBar()->actions();

    QMenu* menu = NULL;
    foreach (QAction* existingMenuAction, menuBarActions)
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
      // We want to defer the creation of the menu actions as much as possible
      // so the File menu will already be populated by the time we add our
      // custom actions. If our actions are inserted first, there is no way to
      // control where in the list of actions they go, and they end up awkwardly
      // sitting at the top of the menu. By using a single-shot connection to
      // load our actions, we ensure that extant actions are in place; we
      // key off of their locations to make the menu look better.
      QMetaObject::Connection* connection = new QMetaObject::Connection;
      *connection = QObject::connect(menu, &QMenu::aboutToShow, [=]() {
        QAction* exitAction = findExitAction(menu);

        if (exitAction == NULL)
        {
          menu->addSeparator();
        }

        menu->insertAction(exitAction, exportSimulationAction);

        if (exitAction != NULL)
        {
          menu->insertSeparator(exitAction);
        }

        // Remove this connection.
        QObject::disconnect(*connection);
        delete connection;
      });
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
