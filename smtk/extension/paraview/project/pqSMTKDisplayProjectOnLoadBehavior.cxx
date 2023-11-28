//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/project/pqSMTKDisplayProjectOnLoadBehavior.h"

// SMTK
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKTaskPanel.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"

#include "smtk/view/Selection.h"

#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"

#include "smtk/io/Logger.h"

// Client side
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqLiveInsituManager.h"
#include "pqPipelineSource.h"
#include "pqSelectionManager.h"
#include "pqServerManagerModel.h"

// Qt
#include <QMainWindow>
#include <QTimer>
#include <QWidget>

using namespace smtk;

static pqSMTKDisplayProjectOnLoadBehavior* g_displayOnLoad = nullptr;

pqSMTKDisplayProjectOnLoadBehavior::pqSMTKDisplayProjectOnLoadBehavior(QObject* parent)
  : Superclass(parent)
{
  if (!g_displayOnLoad)
  {
    g_displayOnLoad = this;
  }

  // Track server connects/disconnects
  auto* projectBehavior = pqSMTKBehavior::instance();
  QObject::connect(
    projectBehavior,
    SIGNAL(addedManagerOnServer(vtkSMSMTKWrapperProxy*, pqServer*)),
    this,
    SLOT(observeProjectsOnServer(vtkSMSMTKWrapperProxy*, pqServer*)));
  QObject::connect(
    projectBehavior,
    SIGNAL(removingManagerFromServer(vtkSMSMTKWrapperProxy*, pqServer*)),
    this,
    SLOT(unobserveProjectsOnServer(vtkSMSMTKWrapperProxy*, pqServer*)));
}

pqSMTKDisplayProjectOnLoadBehavior::~pqSMTKDisplayProjectOnLoadBehavior()
{
  if (g_displayOnLoad == this)
  {
    g_displayOnLoad = nullptr;
  }
}

pqSMTKDisplayProjectOnLoadBehavior* pqSMTKDisplayProjectOnLoadBehavior::instance(QObject* parent)
{
  if (!g_displayOnLoad)
  {
    g_displayOnLoad = new pqSMTKDisplayProjectOnLoadBehavior(parent);
  }

  return g_displayOnLoad;
}

void pqSMTKDisplayProjectOnLoadBehavior::observeProjectsOnServer(
  vtkSMSMTKWrapperProxy* mgr,
  pqServer* server)
{
  (void)server;
  if (!mgr)
  {
    vtkGenericWarningMacro("No wrapper.");
    return;
  }
  auto projectManager = mgr->GetProjectManager();
  if (!projectManager)
  {
    vtkGenericWarningMacro("No project manager to observe.");
    return;
  }

  auto observerKey = projectManager->observers().insert(
    [this](const smtk::project::Project& project, smtk::project::EventType event) {
      this->handleProjectEvent(project, event);
    },
    0,    // assign a neutral priority
    true, // immediatelyNotify
    "pqSMTKDisplayProjectOnLoadBehavior: Display new attribute project in panel.");
  m_projectManagerObservers[projectManager] = std::move(observerKey);
}

void pqSMTKDisplayProjectOnLoadBehavior::unobserveProjectsOnServer(
  vtkSMSMTKWrapperProxy* mgr,
  pqServer* server)
{
  (void)server;
  if (!mgr)
  {
    return;
  }
  auto projectManager = mgr->GetProjectManager();
  if (!projectManager)
  {
    return;
  }

  auto entry = m_projectManagerObservers.find(projectManager);
  if (entry != m_projectManagerObservers.end())
  {
    projectManager->observers().erase(entry->second);
    m_projectManagerObservers.erase(entry);
  }
}

void pqSMTKDisplayProjectOnLoadBehavior::handleProjectEvent(
  const smtk::project::Project& project,
  smtk::project::EventType event)
{
  auto* taskManager = const_cast<smtk::task::Manager*>(&project.taskManager());
  switch (event)
  {
    case smtk::project::EventType::ADDED:
      this->focusTaskPanel(taskManager);
      break;
    case smtk::project::EventType::REMOVED:
      this->focusTaskPanel(nullptr);
      break;
    case smtk::project::EventType::MODIFIED:
    default:
      // Do nothing.
      break;
  }
}

void pqSMTKDisplayProjectOnLoadBehavior::focusTaskPanel(smtk::task::Manager* taskManager)
{
  auto* core = pqApplicationCore::instance();
  auto* panel = dynamic_cast<pqSMTKTaskPanel*>(core->manager("smtk task panel"));
  if (panel)
  {
    panel->taskEditor()->displayTaskManager(taskManager);
    if (auto* parent = dynamic_cast<QWidget*>(panel->parent()))
    {
      parent->raise(); // Make sure the widget is visible and raised.
    }
  }
}
