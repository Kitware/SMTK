//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/plugin-core/pqSMTKNewResourceBehavior.h"

// Client side
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqFileDialog.h"
#include "pqObjectBuilder.h"
#include "pqPropertiesPanel.h"
#include "pqServer.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/json/jsonResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKRenderResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
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
  const std::string& operationName,
  QAction* parentObject)
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
  createDialog->setWindowTitle(
    QString::fromStdString(createOp->parameters()->definition()->label()));
  createDialog->setLayout(new QVBoxLayout(createDialog.data()));

  // Create a new UI for the dialog.
  QSharedPointer<smtk::extension::qtUIManager> uiManager =
    QSharedPointer<smtk::extension::qtUIManager>(new smtk::extension::qtUIManager(
      createOp, wrapper->smtkResourceManager(), wrapper->smtkViewManager()));

  // Create an operation view for the operation.
  smtk::view::ConfigurationPtr view = uiManager->findOrCreateOperationView();

  // Currently, creation operators all fallow the pattern of optionally creating
  // an entity within an extant resource. Since this functionality doesn't make
  // sense for a "New Resource" menu option, we flag the input items involved
  // with that choice as advanced and disable the advanced items here. Since the
  // choice of filtering by advance level is persistent for the operation, we
  // unset the flag after the operation window returns.
  std::string originalFilterByAdvanceLevel;
  if (view->details().attribute("FilterByAdvanceLevel", originalFilterByAdvanceLevel))
  {
    view->details().setAttribute("FilterByAdvanceLevel", "false");
  }
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

  if (!typeName.empty())
  {
    auto* pqCore = pqApplicationCore::instance();
    auto* builder = pqCore->getObjectBuilder();

    pqSMTKResource* src =
      static_cast<pqSMTKResource*>(builder->createSource("sources", "SMTKResourceCreator", server));
    vtkSMPropertyHelper(src->getProxy(), "TypeName").Set(typeName.c_str());
    vtkSMPropertyHelper(src->getProxy(), "Parameters").Set(parameters.c_str());

    pqSMTKRenderResourceBehavior::instance()->renderPipelineSource(src);
  }

  // Restore the original choice for filtering by advance level so it will be
  // present when the operation is called from another code path.
  if (view->details().attribute("FilterByAdvanceLevel"))
  {
    view->details().setAttribute("FilterByAdvanceLevel", originalFilterByAdvanceLevel);
  }
}

namespace
{
QAction* findSaveResourceAction(QMenu* menu)
{
  Q_FOREACH (QAction* action, menu->actions())
  {
    if (action->text().contains("save resource", Qt::CaseInsensitive))
    {
      return action;
    }
  }
  return nullptr;
}
} // namespace

static pqSMTKNewResourceBehavior* g_instance = nullptr;

pqSMTKNewResourceBehavior::pqSMTKNewResourceBehavior(QObject* parent)
  : Superclass(parent)
{
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    QObject::connect(pqCore, &pqApplicationCore::clientEnvironmentDone, [this]() {
      QMenu* fileMenu = this->fileMenu();

      // If no main window exists, then there would be no
      // file menu. Stop here if this is the case.
      if (fileMenu == nullptr)
      {
        return;
      }

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
      QObject::connect(
        activeObjects, SIGNAL(serverChanged(pqServer*)), this, SLOT(updateNewMenu()));

      pqServer* server = pqActiveObjects::instance().activeServer();
      pqSMTKWrapper* wrapper = pqSMTKBehavior::instance()->resourceManagerForServer(server);
      if (wrapper != nullptr)
      {
        m_key = wrapper->smtkOperationManager()->groupObservers().insert(
          [](
            const smtk::operation::Operation::Index& /*unused*/,
            const std::string& groupName,
            bool /*unused*/) {
            if (g_instance != nullptr && groupName == smtk::operation::CreatorGroup::type_name)
            {
              g_instance->updateNewMenu();
            }
          });
        // Access the creator group.
        auto creatorGroup = smtk::operation::CreatorGroup(wrapper->smtkOperationManager());
      }
    });
  }
}

QMenu* pqSMTKNewResourceBehavior::fileMenu()
{
  QMainWindow* mainWindow = qobject_cast<QMainWindow*>(pqCoreUtilities::mainWidget());

  // Stop here if main window does not exist
  // This is typically the case for certain unit tests.
  if (!mainWindow)
  {
    return nullptr;
  }

  QList<QAction*> menuBarActions = mainWindow->menuBar()->actions();

  QMenu* menu = nullptr;
  Q_FOREACH (QAction* existingMenuAction, menuBarActions)
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
    m_newMenu->setObjectName("newResourceMenu");

    bool visible = false;

    pqServer* server = pqActiveObjects::instance().activeServer();
    if (!server)
    {
      return;
    }
    pqSMTKWrapper* wrapper = pqSMTKBehavior::instance()->resourceManagerForServer(server);
    if (wrapper != nullptr)
    {
      // Access the creator group.
      auto creatorGroup = smtk::operation::CreatorGroup(wrapper->smtkOperationManager());

      // Access all resources that can be created by operations in the creator
      // group.
      auto resourceNames = creatorGroup.supportedResources();
      visible = !resourceNames.empty();

      for (const auto& resourceName : resourceNames)
      {
        // To acquire a human-readable resource name, we split on the c++
        // double-colon separator and select the penultimate token as the
        // short-form for the resource name. We then capitalize the name.
        // This is hack-ish, but it should serve as a stopgap until a
        // standardized convention for assigning human-readable labels to
        // resources is in place.
        QString label = QString::fromStdString(resourceName);
        QStringList splitLabel = label.split("::");
        label = *(++splitLabel.rbegin());
        label[0] = label[0].toUpper();

        auto operationIndices = creatorGroup.operationsForResource(resourceName);

        // If there is only one Create operation associated with this resource,
        // then there is no need to display the Create operation's label.
        if (operationIndices.size() == 1)
        {
          auto index = *operationIndices.begin();
          std::string oplabel = creatorGroup.operationLabel(index);
          if (!oplabel.empty())
          {
            label = oplabel.c_str();
          }
          QAction* newResourceAction = new QAction(label, m_newMenu);
          newResourceAction->setObjectName(QString::fromStdString(resourceName));
          m_newMenu->addAction(newResourceAction);

          std::string operationName = creatorGroup.operationName(*operationIndices.begin());
          new pqNewResourceReaction(operationName, newResourceAction);
        }
        else
        {
          // If there is more than one Create operation associated with this
          // resource, then we promote the resource from an action to a menu and
          // populate the menu with the operations that can create the resource.
          QMenu* resourceMenu = new QMenu(label, m_newMenu);
          m_newMenu->addMenu(resourceMenu);
          resourceMenu->setObjectName(label);

          for (const auto& index : operationIndices)
          {
            std::string operationName = creatorGroup.operationName(index);
            label = QString::fromStdString(creatorGroup.operationLabel(index));

            QAction* newResourceAction = new QAction(label, resourceMenu);
            newResourceAction->setObjectName(QString::fromStdString(operationName));
            resourceMenu->addAction(newResourceAction);

            new pqNewResourceReaction(operationName, newResourceAction);
          }
        }
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
    pqServer* server = pqActiveObjects::instance().activeServer();
    pqSMTKWrapper* wrapper = pqSMTKBehavior::instance()->resourceManagerForServer(server);
    if (wrapper != nullptr)
    {
      wrapper->smtkOperationManager()->groupObservers().erase(m_key);
    }

    g_instance = nullptr;
  }

  QObject::disconnect(this);
}
