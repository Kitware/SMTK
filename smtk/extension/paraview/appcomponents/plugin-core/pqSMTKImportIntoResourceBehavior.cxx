//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/plugin-core/pqSMTKImportIntoResourceBehavior.h"

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
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/json/jsonResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKRenderResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/groups/ImporterGroup.h"

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

pqImportIntoResourceReaction::pqImportIntoResourceReaction(QAction* parentObject)
  : Superclass(parentObject)
{
  pqActiveObjects* activeObjects = &pqActiveObjects::instance();
  QObject::connect(
    activeObjects, SIGNAL(serverChanged(pqServer*)), this, SLOT(updateEnableState()));
  QObject::connect(
    activeObjects, SIGNAL(sourceChanged(pqPipelineSource*)), this, SLOT(updateEnableState()));
  this->updateEnableState();
}

void pqImportIntoResourceReaction::updateEnableState()
{
  this->parentAction()->setEnabled(false);

  pqActiveObjects& activeObjects = pqActiveObjects::instance();

  // TODO: also is there's a pending accept.
  bool enable_state =
    activeObjects.activeServer() != nullptr && activeObjects.activeSource() != nullptr;
  if (!enable_state)
  {
    return;
  }

  pqSMTKResource* smtkResource = dynamic_cast<pqSMTKResource*>(activeObjects.activeSource());
  if (smtkResource == nullptr)
  {
    return;
  }

  // TODO: there should be a check here that the active resource type has an
  // associated import operation, but when this is called the active resource is
  // not accessible.

  this->parentAction()->setEnabled(true);
}

void pqImportIntoResourceReaction::importIntoResource()
{
  // Access the active server
  pqServer* server = pqActiveObjects::instance().activeServer();
  pqSMTKWrapper* wrapper = pqSMTKBehavior::instance()->resourceManagerForServer(server);

  // Access the active resource
  pqActiveObjects& activeObjects = pqActiveObjects::instance();

  pqSMTKResource* smtkResource = dynamic_cast<pqSMTKResource*>(activeObjects.activeSource());
  smtk::resource::ResourcePtr resource = smtkResource->getResource();

  // Access the importer group.
  auto importerGroup = smtk::operation::ImporterGroup(wrapper->smtkOperationManager());

  // Access the operations that import into the active resource type.
  auto operationIndices = importerGroup.operationsForResource(resource->typeName());

  // TODO: handle the selection of import operations more intelligently.
  if (operationIndices.empty())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "No import operation registered to resource type <" << resource->typeName() << ">.");
    return;
  }
  auto operationIndex = *operationIndices.begin();

  // Construct an import operation.
  auto importIntoOp = wrapper->smtkOperationManager()->create(operationIndex);

  // Associate the active resource to the operation's parameters
  if (!importIntoOp->parameters()->associate(resource))
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Import operation for resource type <"
        << resource->typeName() << "> is not configured for importing into an extant resource.");
    return;
  }

  // Access the file item definition associated with the import operation.
  auto fileItemDef = importerGroup.fileItemDefinitionForOperation(operationIndex);

  // Construct a file dialog to let the user select the file to import.
  pqFileDialog fileDialog(
    server,
    pqCoreUtilities::mainWidget(),
    tr("Import File:"),
    QString(),
    QString::fromStdString(fileItemDef->getFileFilters()));
  fileDialog.setObjectName("FileSaveDialog");
  fileDialog.setFileMode(pqFileDialog::ExistingFiles);

  if (fileDialog.exec() == QDialog::Accepted)
  {
    QList<QStringList> files = fileDialog.getAllSelectedFiles();
    auto fileItem = importIntoOp->parameters()->findFile(fileItemDef->name());
    if (fileItemDef->isExtensible())
    {
      fileItem->setNumberOfValues(files.size());
      std::size_t i = 0;
      for (auto cit = files.constBegin(); cit != files.constEnd(); ++cit, ++i)
      {
        fileItem->setValue(i, (*cit)[0].toStdString());
      }
      // Execute the import operation for all files at once.
      auto result = importIntoOp->operate();
    }
    else
    {
      for (auto cit = files.constBegin(); cit != files.constEnd(); ++cit)
      {
        fileItem->setValue((*cit)[0].toStdString());
        // Execute the import operation one file at a time.
        auto result = importIntoOp->operate();
      }
    }

    // Render the resource (also resets "Apply")
    pqSMTKRenderResourceBehavior::instance()->renderPipelineSource(smtkResource);
  }
}

namespace
{
QAction* findSaveAction(QMenu* menu)
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

static pqSMTKImportIntoResourceBehavior* g_instance = nullptr;

pqSMTKImportIntoResourceBehavior::pqSMTKImportIntoResourceBehavior(QObject* parent)
  : Superclass(parent)
{
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    QObject::connect(pqCore, &pqApplicationCore::clientEnvironmentDone, [this]() {
      QAction* importIntoResourceAction = new QAction(tr("&Import Into Resource..."), this);
      importIntoResourceAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));

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
        // load our actions, we ensure that extant Save methods are in place; we
        // key off of their location to make the menu look better.
        QMetaObject::Connection* connection = new QMetaObject::Connection;
        *connection = QObject::connect(menu, &QMenu::aboutToShow, [=]() {
          QAction* saveAction = findSaveAction(menu);

          menu->insertAction(saveAction, importIntoResourceAction);
          menu->insertSeparator(saveAction);
          menu->insertSeparator(importIntoResourceAction);

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
        menu->addAction(importIntoResourceAction);
        // insert new menus before the Help menu is possible.
        mainWindow->menuBar()->insertMenu(::findHelpMenuAction(mainWindow->menuBar()), menu);
      }
      new pqImportIntoResourceReaction(importIntoResourceAction);
    });
  }
}

pqSMTKImportIntoResourceBehavior* pqSMTKImportIntoResourceBehavior::instance(QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKImportIntoResourceBehavior(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

pqSMTKImportIntoResourceBehavior::~pqSMTKImportIntoResourceBehavior()
{
  if (g_instance == this)
  {
    g_instance = nullptr;
  }

  QObject::disconnect(this);
}
