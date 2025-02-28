//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKGroupComponentsBehavior.h"

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
#include "smtk/extension/paraview/server/vtkSMTKSettings.h"
#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/groups/GroupingGroup.h"
#include "smtk/operation/groups/UngroupingGroup.h"
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

pqGroupComponentsReaction::pqGroupComponentsReaction(QAction* parentObject, bool isGrouping)
  : Superclass(parentObject)
  , m_isGrouping(isGrouping)
{
  // Monitor behavior for change to active wrapper/server, have it monitor
  // for change to active selection and call updateEnableState when the
  // selection changes.
  auto& activeObjects = pqActiveObjects::instance();
  QObject::connect(
    &activeObjects,
    &pqActiveObjects::serverChanged,
    this,
    &pqGroupComponentsReaction::updateSelectionObserver);
  this->updateSelectionObserver(activeObjects.activeServer());
}

void pqGroupComponentsReaction::updateEnableState()
{
  auto* bb = pqSMTKBehavior::instance();
  auto* ww = bb ? bb->builtinOrActiveWrapper() : nullptr;
  auto selection = ww ? ww->smtkSelection() : nullptr;
  if (!selection)
  {
    // Skip during destruction.
    return;
  }
  auto selected =
    selection->currentSelectionByValueAs<std::set<smtk::resource::PersistentObject::Ptr>>(
      "selected", /*exactMatch*/ false);

  // NB: We could be more thorough by using more of the logic in groupSelectedComponents() below.
  if (m_isGrouping)
  { // Grouping requires at least two things (refuse to create an empty or single-object group).
    this->parentAction()->setEnabled(selected.size() > 1);
  }
  else
  { // Ungrouping requires only a single (group) to be selected.
    this->parentAction()->setEnabled(!selected.empty());
  }
}

void pqGroupComponentsReaction::updateSelectionObserver(pqServer* server)
{
  auto* b = pqSMTKBehavior::instance();
  auto* wrapper = b->resourceManagerForServer(server);
  auto selection = wrapper ? wrapper->smtkSelection() : nullptr;
  if (selection)
  {
    m_observerKey = selection->observers().insert(
      [this](const std::string&, std::shared_ptr<smtk::view::Selection> const&) {
        this->updateEnableState();
      },
      std::numeric_limits<smtk::view::SelectionObservers::Priority>::lowest(),
      /* initialize immediately */ true,
      m_isGrouping ? "update grouping action" : "update ungrouping action");
  }
  else
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(), "Active ParaView server " << server << " has no selection.");
    m_observerKey.release();
  }
}

void pqGroupComponentsReaction::groupSelectedComponents()
{
  auto* b = pqSMTKBehavior::instance();
  auto opMgr = b->builtinOrActiveWrapper()->smtkOperationManager();
  smtk::operation::GroupingGroup groupingGroup(opMgr);
  auto selection = b->builtinOrActiveWrapper()->smtkSelection();
  auto selected =
    selection->currentSelectionByValueAs<std::set<smtk::resource::PersistentObject::Ptr>>(
      "selected", /*exactMatch*/ false);
  if (selected.size() < 2)
  {
    return;
  }
  // NB: We could be more thorough by ensuring an operation that can associate
  //     *all* of the selected objects exists in the GroupingGroup.
  auto opIndex = groupingGroup.matchingOperation(**selected.begin());
  auto groupOp = opMgr->create(opIndex);
  if (!groupOp)
  {
    return;
  }
  auto assoc = groupOp->parameters()->associations();
  std::size_t numAssoc = 0;
  for (const auto& member : selected)
  {
    numAssoc += assoc->appendValue(member) ? 1 : 0;
  }
  if (numAssoc < 2)
  {
    return;
  }
  opMgr->launchers()(groupOp);
}

void pqGroupComponentsReaction::ungroupSelectedComponents()
{
  auto* b = pqSMTKBehavior::instance();
  auto opMgr = b->builtinOrActiveWrapper()->smtkOperationManager();
  smtk::operation::UngroupingGroup ungroupingGroup(opMgr);
  auto selection = b->builtinOrActiveWrapper()->smtkSelection();
  auto selected =
    selection->currentSelectionByValueAs<std::set<smtk::resource::PersistentObject::Ptr>>(
      "selected", /*exactMatch*/ false);
  if (selected.empty())
  {
    return;
  }
  // NB: We could be more thorough by ensuring an operation that can associate
  //     *all* of the selected objects exists in the UngroupingGroup.
  auto opIndex = ungroupingGroup.matchingOperation(**selected.begin());
  auto ungroupOp = opMgr->create(opIndex);
  if (!ungroupOp)
  {
    return;
  }
  auto assoc = ungroupOp->parameters()->associations();
  std::size_t numAssoc = 0;
  for (const auto& member : selected)
  {
    numAssoc += assoc->appendValue(member) ? 1 : 0;
  }
  if (numAssoc == 0)
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(), "You can currently only ungroup groups. Select a group.");
    return;
  }
  opMgr->launchers()(ungroupOp);
}

namespace
{
QAction* findInsertBeforeAction(QMenu* menu)
{
  Q_FOREACH (QAction* action, menu->actions())
  {
    if (action->text().contains("copy screenshot to clipboard", Qt::CaseInsensitive))
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

static pqSMTKGroupComponentsBehavior* g_instance = nullptr;

pqSMTKGroupComponentsBehavior::pqSMTKGroupComponentsBehavior(QObject* parent)
  : Superclass(parent)
{
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    QObject::connect(pqCore, &pqApplicationCore::clientEnvironmentDone, [this]() {
      auto* groupSelectedComponentsAction = new QAction(tr("&Group"), this);
      groupSelectedComponentsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
      groupSelectedComponentsAction->setShortcutContext(Qt::ApplicationShortcut);
      groupSelectedComponentsAction->setObjectName("groupSelectedComponents");

      auto* ungroupSelectedComponentsAction = new QAction(tr("&Ungroup"), this);
      ungroupSelectedComponentsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_G));
      ungroupSelectedComponentsAction->setShortcutContext(Qt::ApplicationShortcut);
      ungroupSelectedComponentsAction->setObjectName("ungroupSelectedComponents");

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
        if (menuName == "Edit")
        {
          menu = existingMenuAction->menu();
          break;
        }
      }

      if (menu)
      {
        QAction* insertBeforeAction = findInsertBeforeAction(menu);

        menu->insertSeparator(insertBeforeAction);
        menu->insertAction(insertBeforeAction, ungroupSelectedComponentsAction);
        menu->insertAction(ungroupSelectedComponentsAction, groupSelectedComponentsAction);
        menu->insertSeparator(insertBeforeAction);
      }
      else
      {
        smtkWarningMacro(
          smtk::io::Logger::instance(), "Unable to find Edit menu… creating a new one.");
        // If the Edit menu doesn't already exist, I don't think the following
        // logic works. It is taken from pqPluginActionGroupBehavior, which
        // is designed to accomplish pretty much the same task, though.

        // Create new menu.
        menu = new QMenu("Edit", mainWindow);
        menu->setObjectName("Edit");
        menu->addAction(groupSelectedComponentsAction);
        menu->addAction(ungroupSelectedComponentsAction);
        // insert new menus before the Delete menu if possible.
        mainWindow->menuBar()->insertMenu(::findHelpMenuAction(mainWindow->menuBar()), menu);
      }
      new pqGroupComponentsReaction(groupSelectedComponentsAction, true);
      new pqGroupComponentsReaction(ungroupSelectedComponentsAction, false);
      // groupSelectedComponentsAction->setEnabled(true);
    });
  }
}

pqSMTKGroupComponentsBehavior* pqSMTKGroupComponentsBehavior::instance(QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKGroupComponentsBehavior(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

pqSMTKGroupComponentsBehavior::~pqSMTKGroupComponentsBehavior()
{
  if (g_instance == this)
  {
    g_instance = nullptr;
  }

  QObject::disconnect(this);
}
