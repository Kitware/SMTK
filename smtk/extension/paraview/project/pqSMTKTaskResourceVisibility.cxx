//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/project/pqSMTKTaskResourceVisibility.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKDiagramPanel.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/extension/paraview/project/Utility.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtTaskEditor.h"
#include "smtk/extension/qt/diagram/qtTaskPath.h"

#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"

#include "smtk/task/Active.h"
#include "smtk/task/Manager.h"
#include "smtk/task/ObjectsInRoles.h"
#include "smtk/task/Port.h"
#include "smtk/task/Task.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqRepresentation.h"

#include "vtkSMPropertyHelper.h"
#include "vtkSMRepresentationProxy.h"

#include <QTimer>

// Change "#undef" to "#define" to print messages as this behavior
// responds to changes in tasks with 3-d view style.
#undef SMTK_DBG_3D_STYLE

using namespace smtk::string::literals;

namespace
{

pqSMTKTaskResourceVisibility* gInstance = nullptr;

} // anonymous namespace

pqSMTKTaskResourceVisibility::pqSMTKTaskResourceVisibility(QObject* parent)
  : QObject(parent)
{
  auto* behavior = pqSMTKBehavior::instance();
  QObject::connect(
    behavior,
    SIGNAL(addedManagerOnServer(pqSMTKWrapper*, pqServer*)),
    this,
    SLOT(observeProjectsOnServer(pqSMTKWrapper*, pqServer*)));
  QObject::connect(
    behavior,
    SIGNAL(removingManagerFromServer(pqSMTKWrapper*, pqServer*)),
    this,
    SLOT(unobserveProjectsOnServer(pqSMTKWrapper*, pqServer*)));

  // Now update current state to take current server into account (if any).
  behavior->visitResourceManagersOnServers([this](pqSMTKWrapper* wrapper, pqServer* server) {
    this->observeProjectsOnServer(wrapper, server);
    return false; // terminate early
  });

  if (m_currentTaskManager && !m_currentTask)
  {
    this->processTaskEvent(nullptr, "activated");
  }

  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->registerManager("smtk task resource visibility behavior", this);
  }
}

pqSMTKTaskResourceVisibility::~pqSMTKTaskResourceVisibility()
{
  if (this == gInstance)
  {
    gInstance = nullptr;
  }
}

pqSMTKTaskResourceVisibility* pqSMTKTaskResourceVisibility::instance(QObject* parent)
{
  if (!gInstance)
  {
    gInstance = new pqSMTKTaskResourceVisibility(parent);
  }
  return gInstance;
}

void pqSMTKTaskResourceVisibility::observeProjectsOnServer(pqSMTKWrapper* mgr, pqServer* server)
{
  (void)server;
  if (!mgr)
  {
    return;
  }
  auto projectManager = mgr->smtkProjectManager();
  if (!projectManager)
  {
    return;
  }

  QPointer<pqSMTKTaskResourceVisibility> self(this);
  auto observerKey = projectManager->observers().insert(
    [self](const smtk::project::Project& project, smtk::project::EventType event) {
      if (self)
      {
        self->handleProjectEvent(project, event);
      }
    },
    0,    // assign a neutral priority
    true, // immediatelyNotify
    "pqSMTKTaskResourceVisibility: Control resource visibility.");
  m_projectManagerObservers[projectManager] = std::move(observerKey);

  // m_taskWatcherKey = xx
}

void pqSMTKTaskResourceVisibility::unobserveProjectsOnServer(pqSMTKWrapper* mgr, pqServer* server)
{
  (void)server;
  if (!mgr)
  {
    return;
  }
  auto projectManager = mgr->smtkProjectManager();
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

void pqSMTKTaskResourceVisibility::handleProjectEvent(
  const smtk::project::Project& project,
  smtk::project::EventType event)
{
  auto* taskManager = const_cast<smtk::task::Manager*>(&project.taskManager());
  std::weak_ptr<smtk::task::Manager> weakTaskManager = taskManager->shared_from_this();
  QPointer<pqSMTKTaskResourceVisibility> self(this);
  switch (event)
  {
    case smtk::project::EventType::ADDED:
      m_currentTaskManager = taskManager;
      // observe the active task
      // Use QTimer to wait until the event queue is emptied before trying this;
      // that gives operations time to complete. Blech.
      QTimer::singleShot(0, [this, taskManager, self]() {
        if (!self)
        {
          return;
        }
        auto& activeTracker = taskManager->active();
        m_activeTaskObserver = activeTracker.observers().insert(
          [this, self](smtk::task::Task* prevTask, smtk::task::Task* nextTask) {
            if (!self)
            {
              return;
            }
            this->handleTaskEvent(prevTask, nextTask);
          },
          /* priority */ 0,
          /* initialize */ true,
          "Task tracking for visibility control.");
      });
      break;
    case smtk::project::EventType::REMOVED:
      // stop observing active-task tracker and any active task.
      if (m_currentTask)
      {
        m_currentTask->observers().erase(m_currentTaskObserver);
      }
      m_currentTask = nullptr;
      m_activeTaskObserver.release();
      // Remove our observer key later.
      QTimer::singleShot(0, [this, weakTaskManager, self]() {
        auto taskManager = weakTaskManager.lock();
        if (!(self && taskManager))
        {
          return;
        }
        auto& activeTracker = taskManager->active();
        activeTracker.observers().erase(m_activeTaskObserver);
      });
      if (m_currentTaskManager == taskManager)
      {
        m_currentTaskManager = nullptr;
      }
      break;
    case smtk::project::EventType::MODIFIED:
    default:
      // Do nothing.
      break;
  }
}

void pqSMTKTaskResourceVisibility::handleTaskEvent(
  smtk::task::Task* prevTask,
  smtk::task::Task* nextTask)
{
  // Stop observing the prior task (if any).
  m_currentTaskObserver.release();

  (void)prevTask;
  if (m_currentTask)
  {
    m_currentTask->observers().erase(m_currentTaskObserver);
    this->processTaskEvent(m_currentTask, "deactivated"_token);
    // self->displayResource(nullptr);
  }
  m_currentTask = nextTask;
  if (m_currentTask)
  {
    // self->displayTaskAttribute(nextTask);
  }
  else
  {
    // For tasks with children, if we deactivate a task without activating
    // any other, we should examine the diagram's task-path (breadcrumb) and
    // apply the trailing task's visual style so that resources for that task
    // are not hidden by "accident."
    auto* pqCore = pqApplicationCore::instance();
    if (pqCore)
    {
      if (auto* panel = dynamic_cast<pqSMTKDiagramPanel*>(pqCore->manager("smtk task panel")))
      {
        for (const auto& generatorEntry : panel->diagram()->generators())
        {
          if (
            const auto& taskEditor =
              std::dynamic_pointer_cast<smtk::extension::qtTaskEditor>(generatorEntry.second))
          {
            const auto* taskPath = taskEditor->taskPath();
            m_currentTask = taskPath->lastTask();
          }
        }
      }
    }
  }
#ifdef SMTK_DBG_3D_STYLE
  std::cout << "Process \"activated\" event for \""
            << (m_currentTask ? m_currentTask->name() : "(default)") << "\".\n";
#endif
  this->processTaskEvent(m_currentTask, "activated"_token);
}

void pqSMTKTaskResourceVisibility::processTaskEvent(
  smtk::task::Task* task,
  smtk::string::Token event)
{
  std::unordered_set<smtk::string::Token> styleSet;
  if (task)
  {
    styleSet = task->style();
  }
  else
  {
    styleSet.insert("default"_token);
  }
  if (m_currentTaskManager)
  {
    for (const auto& styleTag : styleSet)
    {
      auto styleSpec = m_currentTaskManager->getStyle(styleTag);
      if (!styleSpec.contains("3d-view"))
      {
        continue;
      }
      auto viewSpec = styleSpec.at("3d-view");
      for (const auto& entry : viewSpec.items())
      {
        smtk::string::Token directive(entry.key());
        switch (directive.id())
        {
            // clang-format off
        case "color-by"_hash: this->applyColorBy(entry.value(), task, event); break;
        case "hide"_hash:     this->applyShowObjects(entry.value(), false, task, event); break;
        case "show"_hash:     this->applyShowObjects(entry.value(), true,  task, event); break;
        default:
          smtkWarningMacro(smtk::io::Logger::instance(),
            "Unknown directive \"" << directive.data()
            << "\" for style \"" << styleTag.data() << "\". Skipping.");
          break;
            // clang-format on
        }
      }
    }
  }
}

void pqSMTKTaskResourceVisibility::applyColorBy(
  const nlohmann::json& spec,
  smtk::task::Task* task,
  smtk::string::Token event)
{
  (void)task;
  (void)event;

  if (!spec.is_object() || !spec.contains("mode"))
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Specification is not a dictionary or has no mode:\n"
        << spec.dump(2) << "\n");
    return;
  }
  auto representations =
    smtk::paraview::relevantRepresentations(spec, m_currentTaskManager, m_currentTask);
  auto mode(spec.at("mode").get<smtk::string::Token>());
  bool needsRender = false;
  for (const auto& entry : representations)
  {
    auto* representation = entry.first;
    // auto* proxy = vtkSMTKResourceRepresentation::SafeDownCast(representation.getProxy());
    auto* proxy = representation->getProxy();
    if (!proxy)
    {
      continue;
    }

    switch (mode.id())
    {
      case "none"_hash:
        vtkSMPropertyHelper(proxy, "ColorBy").Set("None");
        needsRender = true;
        break;
      default:
      case "attribute-association"_hash:
        smtkErrorMacro(smtk::io::Logger::instance(), "Unsupported color mode.");
        break;
      case "entity"_hash:
        vtkSMPropertyHelper(proxy, "ColorBy").Set("Entity");
        needsRender = true;
        break;
    }
  }
  if (needsRender)
  {
    pqActiveObjects::instance().activeView()->render();
  }
}

void pqSMTKTaskResourceVisibility::applyShowObjects(
  const nlohmann::json& specArray,
  bool show,
  smtk::task::Task* task,
  smtk::string::Token event)
{
  (void)specArray;
  (void)show;
  (void)task;
  (void)event;

  if (!specArray.is_array())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Specification is not an array:\n"
        << specArray.dump(2) << "\n");
    return;
  }
  bool needsRender = false;
  for (const auto& spec : specArray)
  {
    if (!spec.is_object())
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Specification is not a dictionary or has no mode:\n"
          << spec.dump(2) << "\n");
      return;
    }
    auto representations =
      smtk::paraview::relevantRepresentations(spec, m_currentTaskManager, m_currentTask);
    for (const auto& entry : representations)
    {
      auto* representation = entry.first;
      // auto* proxy = vtkSMTKResourceRepresentation::SafeDownCast(representation.getProxy());

      if (representation->isVisible() != show)
      {
        representation->setVisible(show);
        needsRender = true;
      }
    }
  }
  if (needsRender)
  {
    pqActiveObjects::instance().activeView()->render();
  }
}
