//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKNewResourceBehavior.h"

// Client side
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqFileDialog.h"
#include "pqObjectBuilder.h"
#include "pqServer.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/json/jsonResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKOperationPanel.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/groups/CreatorGroup.h"

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QPushButton>
#include <QSharedPointer>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

pqNewResourceReaction::pqNewResourceReaction(
  const std::string& operationName, QAction* parentObject)
  : Superclass(parentObject)
  , m_operationName(operationName)
{
}

void pqNewResourceReaction::newResource()
{
  // Access the active server
  pqServer* server = pqActiveObjects::instance().activeServer();
  pqSMTKWrapper* wrapper = pqSMTKBehavior::instance()->resourceManagerForServer(server);

  // Access the unique name associated with the operation.
  auto createOp = wrapper->smtkOperationManager()->create(m_operationName);

  // Construct a modal dialog for the operation.
  QSharedPointer<QDialog> createDialog = QSharedPointer<QDialog>(new QDialog());
  createDialog->setObjectName("CreateResourceDialog");
  createDialog->setWindowTitle("Create Resource Dialog");
  createDialog->setLayout(new QVBoxLayout(createDialog.data()));

  // Create a new UI for the dialog.
  QSharedPointer<smtk::extension::qtUIManager> uiManager =
    QSharedPointer<smtk::extension::qtUIManager>(new smtk::extension::qtUIManager(createOp));

  // Create an operation view for the operation.
  smtk::view::ViewPtr view = uiManager->findOrCreateOperationView();
  smtk::extension::qtOperationView* opView = dynamic_cast<smtk::extension::qtOperationView*>(
    uiManager->setSMTKView(view, createDialog.data()));

  // Remove all connections from the operation view's apply button. We are going
  // to intercept the client-side operation parameters and execute them on the
  // server.
  opView->applyButton()->disconnect();

  // Retrieve the operation parameters and close the modal dialog when the
  // apply button is clicked.
  std::string typeName;
  std::string parameters;
  QObject::connect(opView->applyButton(), &QPushButton::clicked, [&]() {
    typeName = opView->operation()->typeName();
    json j;
    smtk::attribute::to_json(j, opView->operation()->parameters());
    parameters = j.dump();
    createDialog->done(QDialog::Accepted);
  });

  // Launch the modal dialog and wait for the operation to succeed.
  createDialog->exec();

  auto pqCore = pqApplicationCore::instance();
  auto builder = pqCore->getObjectBuilder();

  pqPipelineSource* src = builder->createSource("sources", "SMTKModelCreator", server);
  vtkSMPropertyHelper(src->getProxy(), "TypeName").Set(typeName.c_str());
  vtkSMPropertyHelper(src->getProxy(), "Parameters").Set(parameters.c_str());
  src->getProxy()->UpdateVTKObjects();
  src->updatePipeline();
}

namespace
{
QAction* findSaveResourceAction(QMenu* menu)
{
  foreach (QAction* action, menu->actions())
  {
    if (action->text().contains("save resource", Qt::CaseInsensitive))
    {
      return action;
    }
  }
  return NULL;
}

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

static pqSMTKNewResourceBehavior* g_instance = nullptr;

pqSMTKNewResourceBehavior::pqSMTKNewResourceBehavior(QObject* parent)
  : Superclass(parent)
  , m_newMenu(nullptr)
{
  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    QMenu* fileMenu = this->fileMenu();

    // We want to defer the creation of the menu actions as much as possible
    // so the File menu will already be populated by the time we add our
    // custom actions. If our actions are inserted first, there is no way to
    // control where in the list of actions they go, and they end up awkwardly
    // sitting at the top of the menu. By using a single-shot connection to
    // load our actions, we ensure that extant Save methods are in place; we
    // key off of their location to make the menu look better.
    QMetaObject::Connection* connection = new QMetaObject::Connection;
    *connection = QObject::connect(fileMenu, &QMenu::aboutToShow, [this, connection, fileMenu]() {
      QAction* saveAction = findSaveResourceAction(fileMenu);

      this->setNewMenu(qobject_cast<QMenu*>(
        fileMenu->insertMenu(saveAction, new QMenu("New Resource"))->parent()));
      this->updateNewMenu();

      // Remove this connection.
      QObject::disconnect(*connection);
      delete connection;
    });

    pqActiveObjects* activeObjects = &pqActiveObjects::instance();
    QObject::connect(activeObjects, SIGNAL(serverChanged(pqServer*)), this, SLOT(updateNewMenu()));
  }
}

QMenu* pqSMTKNewResourceBehavior::fileMenu()
{
  QMainWindow* mainWindow = qobject_cast<QMainWindow*>(pqCoreUtilities::mainWidget());

  QList<QAction*> menuBarActions = mainWindow->menuBar()->actions();

  QMenu* menu = NULL;
  foreach (QAction* existingMenuAction, menuBarActions)
  {
    QString menuName = existingMenuAction->text();
    menuName.remove('&');
    if (menuName == "File")
    {
      return existingMenuAction->menu();
    }
  }

  if (menu == nullptr)
  {
    // Create new menu.
    menu = new QMenu("File", mainWindow);
    menu->setObjectName("File");
  }

  return menu;
}

void pqSMTKNewResourceBehavior::setNewMenu(QMenu* menu)
{
  this->m_newMenu = menu;
}

void pqSMTKNewResourceBehavior::updateNewMenu()
{
  if (m_newMenu != nullptr)
  {
    m_newMenu->clear();

    bool visible = false;

    pqServer* server = pqActiveObjects::instance().activeServer();
    pqSMTKWrapper* wrapper = pqSMTKBehavior::instance()->resourceManagerForServer(server);
    if (wrapper != nullptr)
    {
      auto creatorGroup = smtk::operation::CreatorGroup(wrapper->smtkOperationManager());
      auto operationNames = creatorGroup.operationNames();
      visible = !operationNames.empty();

      for (auto& operationName : operationNames)
      {
        // We start with the unique operation name. We use it as a key to
        // acquire the unique resource name. We then split on the c++
        // double-colon separator and select the penultimate token as the
        // short-form for the resource name. Finally, we capitalize the name.
        // This is hack-ish, but it should serve as a stopgap until a
        // standardized convention for assigning human-readable labels to
        // resources is in place.
        QString label = QString::fromStdString(creatorGroup.resourceForOperation(operationName));
        QStringList splitLabel = label.split("::");
        label = *(++splitLabel.rbegin());
        label[0] = label[0].toUpper();
        QAction* newResourceAction = new QAction(label, m_newMenu);
        m_newMenu->addAction(newResourceAction);

        new pqNewResourceReaction(operationName, newResourceAction);
      }
    }
    m_newMenu->menuAction()->setVisible(visible);
  }
}

pqSMTKNewResourceBehavior* pqSMTKNewResourceBehavior::instance(QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKNewResourceBehavior(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

pqSMTKNewResourceBehavior::~pqSMTKNewResourceBehavior()
{
  if (g_instance == this)
  {
    g_instance = nullptr;
  }

  QObject::disconnect(this);
}
