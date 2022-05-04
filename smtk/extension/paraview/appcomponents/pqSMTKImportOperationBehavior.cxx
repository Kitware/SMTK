//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKImportOperationBehavior.h"

// Client side
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqFileDialog.h"
#include "pqServer.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/operators/ImportPythonOperation.h"

#include <QAction>
#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QObject>

void initImportOperationBehaviorResources()
{
  Q_INIT_RESOURCE(pqSMTKImportOperationBehavior);
}

pqImportOperationReaction::pqImportOperationReaction(QAction* parentObject)
  : Superclass(parentObject)
{
  pqActiveObjects* activeObjects = &pqActiveObjects::instance();
  QObject::connect(
    activeObjects, SIGNAL(serverChanged(pqServer*)), this, SLOT(updateEnableState()));
  this->updateEnableState();
}

void pqImportOperationReaction::updateEnableState()
{
  pqServer* server = pqActiveObjects::instance().activeServer();
  bool enable_state =
    (server != nullptr && pqSMTKBehavior::instance()->resourceManagerForServer(server) != nullptr);
  this->parentAction()->setEnabled(enable_state);
}

void pqImportOperationReaction::importOperation()
{
  // Access the active server
  pqServer* server = pqActiveObjects::instance().activeServer();

  // Construct a file dialog for importing python operations
  pqFileDialog fileDialog(
    server, pqCoreUtilities::mainWidget(), tr("SMTK operation File:"), "", "Python files (*.py)");
  fileDialog.setObjectName("OperationImportDialog");
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
    if (
      result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "\"import python operation\" operation failed\n";
    }
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

static pqSMTKImportOperationBehavior* g_instance = nullptr;

pqSMTKImportOperationBehavior::pqSMTKImportOperationBehavior(QObject* parent)
  : Superclass(parent)
{
  initImportOperationBehaviorResources();

  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    QObject::connect(pqCore, &pqApplicationCore::clientEnvironmentDone, [this]() {
      QAction* importOperationAction = new QAction(
        QPixmap(":/ImportOperationBehavior/python-28x28.png"), tr("&Import Operation..."), this);

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

          if (exitAction == nullptr)
          {
            menu->addSeparator();
          }

          menu->insertAction(exitAction, importOperationAction);

          if (exitAction != nullptr)
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
        menu->addAction(importOperationAction);
        // insert new menus before the Help menu is possible.
        mainWindow->menuBar()->insertMenu(::findHelpMenuAction(mainWindow->menuBar()), menu);
      }
      new pqImportOperationReaction(importOperationAction);
    });
  }
}

pqSMTKImportOperationBehavior* pqSMTKImportOperationBehavior::instance(QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKImportOperationBehavior(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

pqSMTKImportOperationBehavior::~pqSMTKImportOperationBehavior()
{
  if (g_instance == this)
  {
    g_instance = nullptr;
  }

  QObject::disconnect(this);
}
