//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/Configurator.txx"

#include "smtk/io/Logger.h"
#include "smtk/task/Manager.h"

#include "smtk/resource/Manager.h"

#include <thread>
#include <vector>

using namespace smtk::string::literals;

namespace
{
std::mutex g_types;
thread_local std::vector<std::unique_ptr<smtk::task::json::Helper>> g_instanceStack;
} // anonymous namespace

namespace smtk
{
namespace task
{
namespace json
{

Helper::Helper()
  : m_tasks(this)
  , m_ports(this)
  , m_adaptors(this)
{
}

Helper::Helper(Manager* taskManager)
  : m_taskManager(taskManager)
  , m_tasks(this)
  , m_ports(this)
  , m_adaptors(this)
{
}

Helper::~Helper() = default;

Helper& Helper::instance()
{
  if (g_instanceStack.empty())
  {
    g_instanceStack.emplace_back(new Helper);
  }
  return *(g_instanceStack.back());
}

Helper& Helper::pushInstance(
  smtk::task::Manager& taskManager,
  const smtk::common::Managers::Ptr& otherManagers)
{
  g_instanceStack.emplace_back(new Helper(&taskManager));
  g_instanceStack.back()->setManagers(otherManagers);
  return *(g_instanceStack.back());
}

Helper& Helper::pushInstance(smtk::task::Task* parent)
{
  std::shared_ptr<smtk::common::Managers> managers;
  smtk::task::Manager* taskManager = nullptr;
  if (!g_instanceStack.empty())
  {
    managers = g_instanceStack.back()->managers();
    taskManager = &g_instanceStack.back()->taskManager();
    g_instanceStack.emplace_back(new Helper(taskManager));
  }
  else
  {
    g_instanceStack.emplace_back(new Helper());
  }
  g_instanceStack.back()->setManagers(managers);
  if (parent)
  {
    g_instanceStack.back()->tasks().swizzleId(parent);
    g_instanceStack.back()->m_topLevel = false;
  }
  return *(g_instanceStack.back());
}

void Helper::popInstance()
{
  if (!g_instanceStack.empty())
  {
    g_instanceStack.pop_back();
  }
}

std::size_t Helper::nestingDepth()
{
  return g_instanceStack.size();
}

void Helper::setMapUUIDs(bool shouldMap)
{
  if (shouldMap == m_mapUUIDs)
  {
    return;
  }

  if (shouldMap)
  {
    if (!m_tasks.empty() || !m_ports.empty() || !m_adaptors.empty())
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(), "Turned mapping of UUIDs on with non-empty configurators.");
    }
  }
  m_mapUUIDs = shouldMap;
}

Configurator<Task>& Helper::tasks()
{
  return m_tasks;
}

Configurator<Port>& Helper::ports()
{
  return m_ports;
}

Configurator<Adaptor>& Helper::adaptors()
{
  return m_adaptors;
}

void Helper::setManagers(const smtk::common::Managers::Ptr& managers)
{
  m_managers = managers;
}

smtk::common::Managers::Ptr Helper::managers()
{
  return m_managers;
}

void Helper::clear()
{
  m_tasks.clear();
  m_adaptors.clear();
}

void Helper::currentTasks(std::vector<Task*>& tasks)
{
  m_tasks.currentObjects(tasks, this->topLevel() ? 1 : 2);
}

void Helper::currentPorts(std::vector<Port*>& ports)
{
  m_ports.currentObjects(ports, this->topLevel() ? 1 : 2);
}

void Helper::currentAdaptors(std::vector<Adaptor*>& adaptors)
{
  m_adaptors.currentObjects(adaptors, 1);
}

Helper::json Helper::swizzleDependencies(const Task::PassedDependencies& deps)
{
  auto ids = Helper::json::array({});
  for (const auto& dep : deps)
  {
    if (dep)
    {
      SwizzleId id = m_tasks.swizzleId(const_cast<Task*>(dep.get()));
      if (id)
      {
        ids.push_back(id);
      }
    }
  }
  return ids;
}

Task::PassedDependencies Helper::unswizzleDependencies(const json& ids) const
{
  Task::PassedDependencies deps;
  for (const auto& id : ids)
  {
    auto taskId = id.get<SwizzleId>();
    auto* ptr = m_tasks.unswizzle(id);
    if (!ptr)
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(), "No task or null task for ID " << taskId << ". Skipping.");
      continue;
    }
    deps.insert(std::static_pointer_cast<Task>(ptr->shared_from_this()));
  }
  return deps;
}

void Helper::setAdaptorTaskIds(SwizzleId fromId, SwizzleId toId)
{
  // Only allow one set of IDs to be active at a time
  m_adaptorFromUID = smtk::common::UUID::null();
  m_adaptorToUID = smtk::common::UUID::null();
  m_adaptorFromId = fromId;
  m_adaptorToId = toId;
}

void Helper::setAdaptorTaskIds(const smtk::common::UUID& fromId, const smtk::common::UUID& toId)
{
  // Only allow one set of IDs to be active at a time
  m_adaptorFromId = ~static_cast<SwizzleId>(0);
  m_adaptorToId = ~static_cast<SwizzleId>(0);
  m_adaptorFromUID = fromId;
  m_adaptorToUID = toId;
}

void Helper::clearAdaptorTaskIds()
{
  m_adaptorFromId = ~static_cast<SwizzleId>(0);
  m_adaptorToId = ~static_cast<SwizzleId>(0);
  m_adaptorFromUID = smtk::common::UUID::null();
  m_adaptorToUID = smtk::common::UUID::null();
}

std::pair<Task*, Task*> Helper::getAdaptorTasks()
{
  Task* from = nullptr;
  Task* to = nullptr;
  if (!m_adaptorFromUID.isNull() && !m_adaptorToUID.isNull())
  {
    if (m_taskManager)
    {
      from = m_taskManager->taskInstances().findById(m_adaptorFromUID).get();
      to = m_taskManager->taskInstances().findById(m_adaptorToUID).get();
    }
  }
  else
  {
    from = m_tasks.unswizzle(m_adaptorFromId);
    to = m_tasks.unswizzle(m_adaptorToId);
  }
  return std::make_pair(from, to);
}

void Helper::setActiveSerializedTask(Task* task)
{
  bool once = false;
  for (auto& otherHelper : g_instanceStack)
  {
    if (&otherHelper->taskManager() == m_taskManager)
    {
      if (otherHelper->m_activeSerializedTask && !once)
      {
        once = true;
        smtkWarningMacro(
          smtk::io::Logger::instance(), "Multiple active tasks deserialized. Last one wins.");
      }
      otherHelper->m_activeSerializedTask = task;
    }
  }
}

Task* Helper::activeSerializedTask() const
{
  return m_activeSerializedTask;
}

smtk::resource::PersistentObject* Helper::objectFromJSONSpec(
  const json& spec,
  smtk::string::Token objType)
{
  if (spec.is_number_integer())
  {
    switch (objType.id())
    {
      default:
      case "task"_hash:
        return this->tasks().unswizzle(spec.get<smtk::task::json::Configurator<Task>::SwizzleId>());
        break;
      case "port"_hash:
        return this->ports().unswizzle(spec.get<smtk::task::json::Configurator<Port>::SwizzleId>());
        break;
      case "adaptor"_hash:
        return this->adaptors().unswizzle(
          spec.get<smtk::task::json::Configurator<Adaptor>::SwizzleId>());
        break;
    }
  }
  else if (spec.is_array() && spec.size() == 2)
  {
    if (spec[0].is_string() && spec[1].is_number_integer())
    {
      // spec is an object type and swizzle ID
      return this->objectFromJSONSpec(spec[1], spec[0].get<smtk::string::Token>());
    }
    else
    {
      try
      {
        auto rsrcId = spec[0].get<smtk::common::UUID>();
        auto compId = spec[1].get<smtk::common::UUID>();
        if (!rsrcId && compId)
        {
          auto* mgrRsrc = this->taskManager().resource();
          auto* comp = mgrRsrc->component(compId);
          if (!comp && m_mapUUIDs)
          {
            // We are mapping UUIDs; the component may be mapped by
            // one of our configurators:
            if (auto* port = m_ports.get(compId))
            {
              comp = port;
            }
            else if (auto* task = m_ports.get(compId))
            {
              comp = task;
            }
            else if (auto* adaptor = m_adaptors.get(compId))
            {
              comp = adaptor;
            }
          }
          return comp;
        }
        else if (rsrcId && !compId)
        {
          auto mgrs = this->managers();
          auto rsrcMgr = (mgrs ? mgrs->get<smtk::resource::Manager::Ptr>() : nullptr);
          if (rsrcMgr)
          {
            auto rsrc = rsrcMgr->get(rsrcId);
            return rsrc.get();
          }
        }
        else if (rsrcId && compId)
        {
          auto mgrs = this->managers();
          auto rsrcMgr = (mgrs ? mgrs->get<smtk::resource::Manager::Ptr>() : nullptr);
          if (rsrcMgr)
          {
            if (auto rsrc = rsrcMgr->get(rsrcId))
            {
              auto* comp = rsrc->component(compId);
              return comp;
            }
          }
        }
      }
      catch (nlohmann::json::exception& e)
      {
        // Do nothing, which ultimately returns a nullptr.
      }
    }
  }
  else if (spec.is_string() && m_taskManager && m_taskManager->resource())
  {
    // It should be a UUID.
    auto uid = spec.get<smtk::common::UUID>();
    auto* obj = m_taskManager->resource()->component(uid);
    if (!obj && m_mapUUIDs)
    {
      // The object may be mapped by our configurator.
      switch (objType.id())
      {
        default:
        case "task"_hash:
          obj = m_tasks.get(uid);
          break;
        case "port"_hash:
          obj = m_ports.get(uid);
          break;
        case "adaptor"_hash:
          obj = m_adaptors.get(uid);
          break;
      }
    }
    return obj;
  }
  return nullptr;
}

} // namespace json
} // namespace task
} // namespace smtk
