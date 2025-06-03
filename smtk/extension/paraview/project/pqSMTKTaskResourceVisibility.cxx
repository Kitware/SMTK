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

using namespace smtk::string::literals;

namespace
{
pqSMTKTaskResourceVisibility* gInstance = nullptr;

bool resourceMatch(
  const std::string& filter,
  smtk::resource::PersistentObject* object,
  smtk::resource::Resource*& rsrc)
{
  if (auto* resource = dynamic_cast<smtk::resource::Resource*>(object))
  {
    rsrc = resource;
    if (filter == "*" || rsrc->matchesType(filter))
    {
      return true;
    }
  }
  else if (auto* comp = dynamic_cast<smtk::resource::Component*>(object))
  {
    if ((rsrc = comp->parentResource()))
    {
      if (filter == "*" || rsrc->matchesType(filter))
      {
        return true;
      }
    }
  }
  return false;
}

bool componentMatch(
  const std::string& filter,
  smtk::resource::PersistentObject* object,
  smtk::resource::Resource* rsrc)
{
  // An empty filter string indicates only resources are allowed.
  if (filter.empty())
  {
    return (object == rsrc);
  }
  // "*" allows any component:
  else if (filter == "*")
  {
    // Assume that if object is not a pointer to the parent resource,
    // it must be a component:
    return (object != rsrc);
  }
  // We have a non-trivial filter string; we must have a component.
  if (auto* comp = dynamic_cast<smtk::resource::Component*>(object))
  {
    auto query = rsrc->queryOperation(filter);
    return query ? query(*comp) : false;
  }
  return false;
}

bool filterObject(const nlohmann::json::array_t& filter, smtk::resource::PersistentObject* object)
{
  for (const auto& filterTuple : filter)
  {
    try
    {
      if (filterTuple.is_array() && filterTuple.size() == 2)
      {
        smtk::resource::Resource* rsrc{ nullptr };
        if (resourceMatch(filterTuple[0].get<std::string>(), object, rsrc))
        {
          auto compSpec = filterTuple[1].is_null() ? "" : filterTuple[1].get<std::string>();
          if (componentMatch(compSpec, object, rsrc))
          {
            return true;
          }
        }
      }
    }
    catch (nlohmann::json::exception& e)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Cannot process filter \"" << filterTuple.dump() << "\";" << e.what());
      continue;
    }
  }
  return false;
}

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
    std::cout << "Process \"deactivated\" event for \"" << m_currentTask->name() << "\".\n";
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
      if (auto* panel = dynamic_cast<pqSMTKDiagramPanel*>(pqCore->manager("smtk task diagram")))
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
  std::cout << "Process \"activated\" event for \""
            << (m_currentTask ? m_currentTask->name() : "(default)") << "\".\n";
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
  auto representations = this->relevantRepresentations(spec);
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
    auto representations = this->relevantRepresentations(spec);
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

std::map<pqRepresentation*, std::unordered_set<smtk::resource::PersistentObject*>>
pqSMTKTaskResourceVisibility::relevantRepresentations(const nlohmann::json& spec)
{
  // Filter representations by the \a spec.
  std::map<pqRepresentation*, std::unordered_set<smtk::resource::PersistentObject*>>
    representations;

  nlohmann::json source;
  nlohmann::json filter;
  if (spec.contains("source"))
  {
    source = spec.at("source");
    if (!source.is_object() || !source.contains("type"))
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Spec source must be a dictionary with 'type' key, "
        "got \""
          << source.dump() << "\"");
      return representations;
    }
  }
  else
  {
    source = { { "type", "project resources" } };
  }

  if (spec.contains("filter"))
  {
    filter = spec.at("filter");
    if (!filter.is_array())
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Spec filter must be an array, got \"" << filter.dump() << "\".");
      return representations;
    }
  }
  else
  {
    // Accept any resource or component:
    filter = nlohmann::json::array_t({ { "*", "*" }, { "*", nullptr } });
  }

  auto* behavior = pqSMTKBehavior::instance();
  auto sourceType = source["type"].get<smtk::string::Token>();
  std::unordered_set<smtk::resource::PersistentObject*> objects;
  switch (sourceType.id())
  {
    default:
      smtkErrorMacro(
        smtk::io::Logger::instance(), "Unknown source type \"" << sourceType.data() << "\".");
      // fall through
    case "project resources"_hash:
    {
      if (!m_currentTaskManager)
      {
        return representations;
      }

      auto* project = dynamic_cast<smtk::project::Project*>(m_currentTaskManager->resource());
      if (!project)
      {
        return representations;
      }

      for (const auto& resource : project->resources())
      {
        if (filterObject(filter, resource.get()))
        {
          objects.insert(resource.get());
        }
      }
    }
    break;
    case "active task port"_hash:
    {
      if (!m_currentTask || !source.contains("port"))
      {
        smtkErrorMacro(smtk::io::Logger::instance(), "No active task or no \"port\" specified.");
        return representations;
      }

      // We are asked to find objects on a port of the (now) active task.
      // Determine whether we are filtering on role or not.
      smtk::string::Token sourceRole;
      if (source.contains("role"))
      {
        sourceRole = source.at("role").get<smtk::string::Token>();
      }
      // Find the correct port:
      const auto& taskPortMap = m_currentTask->ports();
      auto it = taskPortMap.find(source["port"]);
      if (it == taskPortMap.end())
      {
        return representations;
      }
      // Fetch data from the port. We only understand ObjectsInRoles at this point:
      auto portData = it->second->parent()->portData(it->second);
      if (portData)
      {
        if (auto objectsInRoles = std::dynamic_pointer_cast<smtk::task::ObjectsInRoles>(portData))
        {
          for (const auto& entry : objectsInRoles->data())
          {
            if (sourceRole.valid() && entry.first != sourceRole)
            { // Skip objects in unrequested roles.
              continue;
            }
            // Filter objects as requested.
            for (const auto& object : entry.second)
            {
              if (filterObject(filter, object))
              {
                objects.insert(object);
              }
            }
          }
        }
        else
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Unhandled port data type \"" << portData->typeName() << "\".");
        }
      }
    }
    break;
  }

  for (const auto& object : objects)
  {
    smtk::resource::Resource* resource{ nullptr };
    QPointer<pqSMTKResource> pvrsrc;
    if ((resource = dynamic_cast<smtk::resource::Resource*>(object)))
    {
      pvrsrc = behavior->getPVResource(resource->shared_from_this());
    }
    else if (auto* comp = dynamic_cast<smtk::resource::Component*>(object))
    {
      resource = comp->resource().get();
      pvrsrc = behavior->getPVResource(comp->resource());
    }
    if (!pvrsrc || !this->isResourceRelevant(resource->shared_from_this(), filter))
    {
      continue;
    }

    for (const auto& view : pvrsrc->getViews())
    {
      for (const auto& representation : pvrsrc->getRepresentations(view))
      {
        representations[representation].insert(object);
      }
    }
  }

  return representations;
}

bool pqSMTKTaskResourceVisibility::isResourceRelevant(
  const std::shared_ptr<smtk::resource::Resource>& resource,
  const nlohmann::json& filter)
{
  smtk::resource::Resource* rsrc = resource.get();
  return filterObject(filter, rsrc);
}
