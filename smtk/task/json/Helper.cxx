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

#include <thread>

namespace
{
std::mutex g_types;
thread_local std::unique_ptr<smtk::task::json::Helper> g_instance;
} // anonymous namespace

namespace smtk
{
namespace task
{
namespace json
{

Helper::Helper()
  : m_tasks(this)
  , m_adaptors(this)
{
}

Helper::~Helper() = default;

Helper& Helper::instance()
{
  if (!g_instance)
  {
    g_instance = std::unique_ptr<Helper>(new Helper);
  }
  return *g_instance;
}

Configurator<Task>& Helper::tasks()
{
  return m_tasks;
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

Helper::json Helper::swizzleDependencies(const Task::PassedDependencies& deps)
{
  auto ids = Helper::json::array({});
  for (const auto& dep : deps)
  {
    if (dep)
    {
      std::size_t id = m_tasks.swizzleId(const_cast<Task*>(dep.get()));
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
    auto taskId = id.get<std::size_t>();
    auto* ptr = m_tasks.unswizzle(id);
    if (!ptr)
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(), "No task or null task for ID " << taskId << ". Skipping.");
    }
    deps.insert(ptr->shared_from_this());
  }
  return deps;
}

void Helper::setAdaptorTaskIds(std::size_t fromId, std::size_t toId)
{
  m_adaptorFromId = fromId;
  m_adaptorToId = toId;
}

void Helper::clearAdaptorTaskIds()
{
  m_adaptorFromId = ~0;
  m_adaptorToId = ~0;
}

std::pair<Task::Ptr, Task::Ptr> Helper::getAdaptorTasks()
{
  Task::Ptr fromPtr;
  auto* from = m_tasks.unswizzle(m_adaptorFromId);
  if (from)
  {
    fromPtr = from->shared_from_this();
  }

  Task::Ptr toPtr;
  auto* to = m_tasks.unswizzle(m_adaptorToId);
  if (to)
  {
    toPtr = to->shared_from_this();
  }

  return std::make_pair(fromPtr, toPtr);
}

} // namespace json
} // namespace task
} // namespace smtk
