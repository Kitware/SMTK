//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKSaveResourceBehavior.h"

// Client side
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqFileDialog.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqServer.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"

#include "smtk/common/Paths.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/resource/Manager.h"

#include <QAction>
#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QObject>

void initSaveResourceBehaviorResources()
{
  Q_INIT_RESOURCE(pqSMTKSaveResourceBehavior);
}

//-----------------------------------------------------------------------------
pqSaveResourceReaction::pqSaveResourceReaction(QAction* parentObject)
  : Superclass(parentObject)
{
  pqActiveObjects* activeObjects = &pqActiveObjects::instance();
  QObject::connect(
    activeObjects, SIGNAL(serverChanged(pqServer*)), this, SLOT(updateEnableState()));
  QObject::connect(
    activeObjects, SIGNAL(sourceChanged(pqPipelineSource*)), this, SLOT(updateEnableState()));
  this->updateEnableState();
}

//-----------------------------------------------------------------------------
void pqSaveResourceReaction::updateEnableState()
{
  pqActiveObjects& activeObjects = pqActiveObjects::instance();
  // TODO: also if there's a pending accept.
  bool enable_state =
    (activeObjects.activeServer() != NULL && activeObjects.activeSource() != NULL &&
      dynamic_cast<pqSMTKResource*>(activeObjects.activeSource()) != NULL);
  this->parentAction()->setEnabled(enable_state);
}

//-----------------------------------------------------------------------------
pqSaveResourceReaction::State pqSaveResourceReaction::saveResource()
{
  pqActiveObjects& activeObjects = pqActiveObjects::instance();
  pqSMTKResource* smtkResource = dynamic_cast<pqSMTKResource*>(activeObjects.activeSource());
  smtk::resource::ResourcePtr resource = smtkResource->getResource();

  // If there is no associated file name with this resource, call
  // pqSaveResourceAsReaction's saveResourceAs static method instead.
  if (resource->location().empty())
  {
    return pqSaveResourceAsReaction::saveResourceAs();
  }

  // Ensure the resource suffix is .smtk so readers can subsequently read the
  // file.
  std::string filename = resource->location();
  if (smtk::common::Paths::extension(resource->location()) != ".smtk")
  {
    filename = smtk::common::Paths::directory(filename) + "/" +
      smtk::common::Paths::stem(filename) + ".smtk";

    // If a file already exists with this name, append a distinguishing string
    // to the name.
    if (smtk::common::Paths::fileExists(filename))
    {
      std::string id = resource->id().toString();
      filename = smtk::common::Paths::directory(filename) + "/" +
        smtk::common::Paths::stem(filename) + "_" + id.substr(0, 8) + ".smtk";
    }

    resource->setLocation(filename);
  }

  if (smtk::resource::Manager::Ptr manager = resource->manager())
  {
    // The resource manager returns true on success
    return manager->write(resource) ? pqSaveResourceReaction::State::Succeeded
                                    : pqSaveResourceReaction::State::Failed;
  }

  return pqSaveResourceReaction::State::Failed;
}

//-----------------------------------------------------------------------------
pqSaveResourceAsReaction::pqSaveResourceAsReaction(QAction* parentObject)
  : Superclass(parentObject)
{
  pqActiveObjects* activeObjects = &pqActiveObjects::instance();
  QObject::connect(
    activeObjects, SIGNAL(serverChanged(pqServer*)), this, SLOT(updateEnableState()));
  QObject::connect(
    activeObjects, SIGNAL(sourceChanged(pqPipelineSource*)), this, SLOT(updateEnableState()));
  this->updateEnableState();
}

//-----------------------------------------------------------------------------
void pqSaveResourceAsReaction::updateEnableState()
{
  pqActiveObjects& activeObjects = pqActiveObjects::instance();
  // TODO: also is there's a pending accept.
  bool enable_state =
    (activeObjects.activeServer() != NULL && activeObjects.activeSource() != NULL &&
      dynamic_cast<pqSMTKResource*>(activeObjects.activeSource()) != NULL);
  this->parentAction()->setEnabled(enable_state);
}

//-----------------------------------------------------------------------------
pqSaveResourceReaction::State pqSaveResourceAsReaction::saveResourceAs()
{
  pqServer* server = pqActiveObjects::instance().activeServer();
  pqActiveObjects& activeObjects = pqActiveObjects::instance();
  pqSMTKResource* smtkResource = dynamic_cast<pqSMTKResource*>(activeObjects.activeSource());
  smtk::resource::ResourcePtr resource = smtkResource->getResource();

  QString filters("Simulation Modeling Toolkit Resource files (*.smtk)");

  pqFileDialog fileDialog(
    server, pqCoreUtilities::mainWidget(), tr("Save File:"),
    (resource->location().empty()
        ? QString()
        : QFileInfo(QString::fromStdString(resource->location())).absoluteDir().absolutePath()),
    filters);
  fileDialog.setObjectName("FileSaveDialog");
  fileDialog.setFileMode(pqFileDialog::AnyFile);

  if (fileDialog.exec() == QDialog::Accepted)
  {
    QString fname = fileDialog.getSelectedFiles()[0];

    if (smtk::resource::Manager::Ptr manager = resource->manager())
    {
      return manager->write(resource, fname.toStdString())
        ? pqSaveResourceReaction::State::Succeeded
        : pqSaveResourceReaction::State::Failed;
    }
    else
    {
      return pqSaveResourceReaction::State::Failed;
    }
  }
  else
  {
    return pqSaveResourceReaction::State::Aborted;
  }
}

namespace
{
QAction* findSaveAction(QMenu* menu)
{
  foreach (QAction* action, menu->actions())
  {
    if (action->text().contains("save data", Qt::CaseInsensitive))
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

static pqSMTKSaveResourceBehavior* g_instance = nullptr;

pqSMTKSaveResourceBehavior::pqSMTKSaveResourceBehavior(QObject* parent)
  : Superclass(parent)
{
  initSaveResourceBehaviorResources();

  // Wait until the event loop starts, ensuring that the main window will be
  // accessible.
  QTimer::singleShot(0, this, [this]() {
    // Blech: pqApplicationCore doesn't have the selection manager yet,
    // so wait until we hear that the server is ready to make the connection.
    // We can't have a selection before the first connection, anyway.
    auto pqCore = pqApplicationCore::instance();
    if (pqCore)
    {
      QAction* saveResourceAction =
        new QAction(QPixmap(":/SaveResourceBehavior/Save24.png"), tr("&Save Resource"), this);
      saveResourceAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
      QAction* saveResourceAsAction =
        new QAction(QPixmap(":/SaveResourceBehavior/Save24.png"), tr("&Save Resource As..."), this);
      saveResourceAsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));

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
          QAction* saveAction = findSaveAction(menu);

          menu->insertAction(saveAction, saveResourceAsAction);
          menu->insertAction(saveResourceAsAction, saveResourceAction);

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
        menu->addAction(saveResourceAsAction);
        menu->addAction(saveResourceAction);
        // insert new menus before the Help menu is possible.
        mainWindow->menuBar()->insertMenu(::findHelpMenuAction(mainWindow->menuBar()), menu);
      }
      new pqSaveResourceReaction(saveResourceAction);
      new pqSaveResourceAsReaction(saveResourceAsAction);
    }
  });
}

pqSMTKSaveResourceBehavior* pqSMTKSaveResourceBehavior::instance(QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKSaveResourceBehavior(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

pqSMTKSaveResourceBehavior::~pqSMTKSaveResourceBehavior()
{
  if (g_instance == this)
  {
    g_instance = nullptr;
  }

  QObject::disconnect(this);
}
