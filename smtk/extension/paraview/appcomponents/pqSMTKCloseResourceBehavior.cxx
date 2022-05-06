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
#include "smtk/extension/paraview/appcomponents/pqSMTKRenderResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKSaveOnCloseResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKSaveResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/extension/paraview/server/vtkSMTKSettings.h"
#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/groups/CreatorGroup.h"
#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"
#include "smtk/resource/Manager.h"
#include "smtk/view/Selection.h"

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>

#include <stdexcept>
#include <string>

namespace
{
class UnreleasedMemoryError : public std::exception
{
public:
  explicit UnreleasedMemoryError(
    const std::string& name,
    const std::string& typeName,
    long useCount)
    : message(
        "Resource \"" + name + "\" (Type \"" + typeName +
        "\") has not been released from memory. Use count is " + std::to_string(useCount))
  {
  }
  const char* what() const noexcept override { return message.c_str(); }

private:
  std::string message;
};
} // namespace

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
    (activeObjects.activeServer() != nullptr && activeObjects.activeSource() != nullptr &&
     dynamic_cast<pqSMTKResource*>(activeObjects.activeSource()) != nullptr);
  this->parentAction()->setEnabled(enable_state);
}

void pqCloseResourceReaction::closeResource()
{
  pqActiveObjects& activeObjects = pqActiveObjects::instance();
  pqSMTKResource* smtkResource = dynamic_cast<pqSMTKResource*>(activeObjects.activeSource());

  // Access the active resource
  smtk::resource::ResourcePtr resource = smtkResource->getResource();

  int ret = QMessageBox::Discard;

  if (resource && !resource->clean())
  {
    ret = pqSMTKSaveOnCloseResourceBehavior::showDialogWithPrefs(1, true);

    if (ret == QMessageBox::Save)
    {
      activeObjects.setActiveSource(smtkResource);
      pqSaveResourceReaction::State state = pqSaveResourceReaction::saveResource();
      if (state == pqSaveResourceReaction::State::Aborted)
      {
        // If user pref is DontShowAndSave, an Aborted save dialog must mean
        // Discard, otherwise there's no way to discard a modified resource.
        auto* settings = vtkSMTKSettings::GetInstance();
        int showSave = settings->GetShowSaveResourceOnClose();
        ret =
          showSave == vtkSMTKSettings::DontShowAndSave ? QMessageBox::Discard : QMessageBox::Cancel;
      }
    }
    if (ret == QMessageBox::Discard)
    {
      // Mark the resource as clean, even though it hasn't been saved. This way,
      // other listeners will not prompt the user to save the resource.
      resource->setClean(true);
    }
  }

  if (ret != QMessageBox::Cancel)
  {
    // Remove it from its manager
    if (smtk::resource::Manager::Ptr manager = resource->manager())
    {
      manager->remove(resource);
    }

    // Remove project instance from project manager
    auto project = std::dynamic_pointer_cast<smtk::project::Project>(resource);
    if (project)
    {
      pqServer* server = pqActiveObjects::instance().activeServer();
      pqSMTKWrapper* wrapper = pqSMTKBehavior::instance()->resourceManagerForServer(server);
      auto projectManager = wrapper->smtkProjectManager();
      projectManager->remove(project);
    }

    // Remove it from the active selection
    {
      smtk::view::Selection::SelectionMap& selections =
        const_cast<smtk::view::Selection::SelectionMap&>(
          smtk::view::Selection::instance()->currentSelection());
      selections.erase(resource);
    }
  }

// If we are not holding the last reference to the resource, then there is a
// memory leak.
#ifndef NDEBUG
  if (resource && resource.use_count() != 1)
  {
    throw UnreleasedMemoryError(resource->name(), resource->typeName(), resource.use_count());
  }
#endif
}

namespace
{
QAction* findSaveStateAction(QMenu* menu)
{
  Q_FOREACH (QAction* action, menu->actions())
  {
    if (action->text().contains("save state", Qt::CaseInsensitive))
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

static pqSMTKCloseResourceBehavior* g_instance = nullptr;

pqSMTKCloseResourceBehavior::pqSMTKCloseResourceBehavior(QObject* parent)
  : Superclass(parent)
{
  initCloseResourceBehaviorResources();

  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    QObject::connect(pqCore, &pqApplicationCore::clientEnvironmentDone, [this]() {
      QAction* closeResourceAction =
        new QAction(QPixmap(":/CloseResourceBehavior/Close22.png"), tr("&Close Resource"), this);
      closeResourceAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
      closeResourceAction->setObjectName("closeResource");

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
    });
  }
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
