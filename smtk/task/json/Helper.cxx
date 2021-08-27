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
#include <vector>

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

Helper& Helper::pushInstance(smtk::task::Task* parent)
{
  std::shared_ptr<smtk::common::Managers> managers;
  if (!g_instanceStack.empty())
  {
    managers = g_instanceStack.back()->managers();
  }
  g_instanceStack.emplace_back(new Helper);
  g_instanceStack.back()->setManagers(managers);
  g_instanceStack.back()->tasks().swizzleId(parent);
  g_instanceStack.back()->m_topLevel = false;
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
    }
    deps.insert(ptr->shared_from_this());
  }
  return deps;
}

void Helper::setAdaptorTaskIds(SwizzleId fromId, SwizzleId toId)
{
  m_adaptorFromId = fromId;
  m_adaptorToId = toId;
}

void Helper::clearAdaptorTaskIds()
{
  m_adaptorFromId = ~static_cast<SwizzleId>(0);
  m_adaptorToId = ~static_cast<SwizzleId>(0);
}

std::pair<Task*, Task*> Helper::getAdaptorTasks()
{
  Task::Ptr fromPtr;
  auto* from = m_tasks.unswizzle(m_adaptorFromId);
  auto* to = m_tasks.unswizzle(m_adaptorToId);
  return std::make_pair(from, to);
}

} // namespace json
} // namespace task
} // namespace smtk
