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
#include "smtk/io/Logger.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/groups/CreatorGroup.h"
#include "smtk/operation/operators/RemoveResource.h"
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
    // Remove it from its manager. Use an operation to avoid observer conflicts.
    pqServer* server = pqActiveObjects::instance().activeServer();
    pqSMTKWrapper* wrapper = pqSMTKBehavior::instance()->resourceManagerForServer(server);
    if (server && wrapper)
    {
      smtk::operation::RemoveResource::Ptr removeOp =
        wrapper->smtkOperationManager()->create<smtk::operation::RemoveResource>();

      removeOp->parameters()->associate(resource);

      smtk::operation::Operation::Result removeOpResult = removeOp->operate();
      if (
        removeOpResult->findInt("outcome")->value() !=
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(), "RemoveResource operation failed while closing a resource");
        // resource is still managed (belongs to a project), avoid debug exception below.
        ret = QMessageBox::Cancel;
      }
    }
    // Remove it from the active selection
    {
      smtk::view::Selection::SelectionMap& selections =
        const_cast<smtk::view::Selection::SelectionMap&>(
          smtk::view::Selection::instance()->currentSelection());
      selections.erase(resource);
    }
  }

  // For debugging, print the use count of the resource to indicate
  // how many shared pointers are referencing it. This will generally
  // be 1 just before the resource is released, but since this now
  // happens on a different thread it may be 2 at the time this
  // message is printed. Larger numbers indicate problems with
  // code holding on to the resource beyond its expected life.
  if (ret != QMessageBox::Cancel && resource && resource.use_count() != 1)
  {
    smtkInfoMacro(
      smtk::io::Logger::instance(),
      "Unexpected use count (" << resource.use_count() << ") "
                               << "for resource " << resource << " (" << resource->name() << ", "
                               << resource->typeName() << ", " << resource->location()
                               << ") being released.");
  }
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
        QAction* insertBeforeAction = findSaveStateAction(menu);

        menu->insertSeparator(insertBeforeAction);
        menu->insertAction(insertBeforeAction, closeResourceAction);
        menu->insertSeparator(insertBeforeAction);
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
