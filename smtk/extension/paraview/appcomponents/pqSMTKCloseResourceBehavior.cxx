//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKCloseResourceBehavior.h"

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
#include "smtk/extension/paraview/appcomponents/pqSMTKOperationPanel.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKRenderResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/groups/CreatorGroup.h"
#include "smtk/resource/Manager.h"

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QPushButton>
#include <QSharedPointer>

void initCloseResourceBehaviorResources()
{
  Q_INIT_RESOURCE(pqSMTKCloseResourceBehavior);
}

pqCloseResourceReaction::pqCloseResourceReaction(QAction* parentObject)
  : Superclass(parentObject)
{
  pqActiveObjects* activeObjects = &pqActiveObjects::instance();
  QObject::connect(
    activeObjects, SIGNAL(serverChanged(pqServer*)), this, SLOT(updateEnableState()));
  QObject::connect(
    activeObjects, SIGNAL(sourceChanged(pqPipelineSource*)), this, SLOT(updateEnableState()));
  this->updateEnableState();
}

void pqCloseResourceReaction::updateEnableState()
{
  pqActiveObjects& activeObjects = pqActiveObjects::instance();
  // TODO: also is there's a pending accept.
  bool enable_state =
    (activeObjects.activeServer() != NULL && activeObjects.activeSource() != NULL &&
      dynamic_cast<pqSMTKResource*>(activeObjects.activeSource()) != NULL);
  this->parentAction()->setEnabled(enable_state);
}

void pqCloseResourceReaction::closeResource()
{
  pqActiveObjects& activeObjects = pqActiveObjects::instance();
  pqSMTKResource* smtkResource = dynamic_cast<pqSMTKResource*>(activeObjects.activeSource());

  // Access the active resource
  smtk::resource::ResourcePtr resource = smtkResource->getResource();

  // Remove it from its manager
  if (smtk::resource::Manager::Ptr manager = resource->manager())
  {
    manager->remove(resource);
  }

  // Destroy the active source associated with the active resource
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  builder->destroy(smtkResource);
}

namespace
{
QAction* findSaveStateAction(QMenu* menu)
{
  foreach (QAction* action, menu->actions())
  {
    if (action->text().contains("save state", Qt::CaseInsensitive))
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

static pqSMTKCloseResourceBehavior* g_instance = nullptr;

pqSMTKCloseResourceBehavior::pqSMTKCloseResourceBehavior(QObject* parent)
  : Superclass(parent)
  , m_newMenu(nullptr)
{
  initCloseResourceBehaviorResources();

  // Wait until the event loop starts, ensuring that the main window will be
  // accessible.
  QTimer::singleShot(0, this, [this]() {
    auto pqCore = pqApplicationCore::instance();
    if (pqCore)
    {
      QAction* closeResourceAction =
        new QAction(QPixmap(":/CloseResourceBehavior/Close22.png"), tr("&Close Resource"), this);
      closeResourceAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));

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
        // load our actions, we ensure that extant Save methods are in place; we
        // key off of their location to make the menu look better.
        QMetaObject::Connection* connection = new QMetaObject::Connection;
        *connection = QObject::connect(menu, &QMenu::aboutToShow, [=]() {
          QAction* saveAction = findSaveStateAction(menu);

          menu->insertSeparator(saveAction);
          menu->insertAction(saveAction, closeResourceAction);
          menu->insertSeparator(closeResourceAction);

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
        menu->addAction(closeResourceAction);
        // insert new menus before the Help menu is possible.
        mainWindow->menuBar()->insertMenu(::findHelpMenuAction(mainWindow->menuBar()), menu);
      }
      new pqCloseResourceReaction(closeResourceAction);
    }
  });
}

pqSMTKCloseResourceBehavior* pqSMTKCloseResourceBehavior::instance(QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKCloseResourceBehavior(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

pqSMTKCloseResourceBehavior::~pqSMTKCloseResourceBehavior()
{
  if (g_instance == this)
  {
    g_instance = nullptr;
  }

  QObject::disconnect(this);
}
