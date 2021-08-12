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
std::unordered_map<std::string, Helper::ConfigurationHelper> Helper::s_types;

Helper::Helper() = default;

Helper::~Helper() = default;

bool Helper::registerType(const std::string& typeName, ConfigurationHelper helper)
{
  std::cout << "Register " << typeName << "\n";
  std::lock_guard<std::mutex> lock(g_types);
  return s_types.insert({ typeName, helper }).second;
}

bool Helper::unregisterType(const std::string& typeName)
{
  std::lock_guard<std::mutex> lock(g_types);
  return s_types.erase(typeName) > 0;
}

Helper& Helper::instance()
{
  if (!g_instance)
  {
    g_instance = std::unique_ptr<Helper>(new Helper);
  }
  return *g_instance;
}

void Helper::setManagers(const smtk::common::Managers::Ptr& managers)
{
  m_managers = managers;
}

smtk::common::Managers::Ptr Helper::managers()
{
  return m_managers;
}

Task::Configuration Helper::configuration(const Task* task)
{
  Task::Configuration config;
  if (task)
  {
    auto typeName = task->typeName();
    ConfigurationHelper taskHelper = nullptr;
    HelperTypeMap::const_iterator it;
    {
      std::lock_guard<std::mutex> lock(g_types);
      it = s_types.find(typeName);
      if (it == s_types.end())
      {
        return config;
      }
      taskHelper = it->second;
    }
    this->swizzleId(task); // Assign a task ID as early as possible.
    config = taskHelper(task, *this);
  }
  return config;
}

void Helper::clear()
{
  m_swizzleFwd.clear();
  m_swizzleBck.clear();
  m_nextSwizzle = 1;
}

std::size_t Helper::swizzleId(const Task* task)
{
  if (!task)
  {
    return 0;
  }
  auto* nctask = const_cast<Task*>(task); // Need a non-const Task in some cases.
  const auto& it = m_swizzleFwd.find(nctask);
  if (it != m_swizzleFwd.end())
  {
    return it->second;
  }
  std::size_t id = m_nextSwizzle++;
  m_swizzleFwd[nctask] = id;
  m_swizzleBck[id] = nctask;
  return id;
}

Helper::json Helper::swizzleDependencies(const Task::PassedDependencies& deps)
{
  auto ids = Helper::json::array({});
  for (const auto& dep : deps)
  {
    if (dep)
    {
      std::size_t id = this->swizzleId(const_cast<Task*>(dep.get()));
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
    auto it = m_swizzleBck.find(taskId);
    if (it == m_swizzleBck.end() || !it->second)
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(), "No task or null task for ID " << taskId << ". Skipping.");
    }
    deps.insert(it->second->shared_from_this());
  }
  return deps;
}

} // namespace json
} // namespace task
} // namespace smtk
