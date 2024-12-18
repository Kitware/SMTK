//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/Instances.h"
#include "smtk/task/Manager.h"

namespace smtk
{
namespace task
{

Instances::Instances(Manager& taskManager)
  : m_taskManager(taskManager)
{
}

Task::Ptr Instances::createFromName(const std::string& taskType)
{
  return this->Superclass::createFromName(taskType);
}

Task::Ptr Instances::createFromName(
  const std::string& taskType,
  Task::Configuration& configuration,
  std::shared_ptr<smtk::common::Managers> managers)
{
  return this->Superclass::createFromName(taskType, configuration, m_taskManager, managers);
}

Task::Ptr Instances::createFromName(
  const std::string& taskType,
  smtk::task::Task::Configuration& configuration,
  smtk::task::Task::PassedDependencies& dependencies,
  std::shared_ptr<smtk::common::Managers> managers)
{
  return this->Superclass::createFromName(
    taskType, configuration, dependencies, m_taskManager, managers);
}

std::set<smtk::task::Task::Ptr> Instances::findByName(const std::string& name) const
{
  std::set<smtk::task::Task::Ptr> foundTasks;
  this->visit([&foundTasks, name](const std::shared_ptr<smtk::task::Task>& task) {
    if (task->name() == name)
    {
      foundTasks.insert(task);
    }
    return smtk::common::Visit::Continue;
  });
  return foundTasks;
}

smtk::task::Task::Ptr Instances::findById(const smtk::common::UUID& taskId) const
{
  smtk::task::Task::Ptr foundTask;
  this->visit([&foundTask, taskId](const std::shared_ptr<smtk::task::Task>& task) {
    if (task->id() == taskId)
    {
      foundTask = task;
      return smtk::common::Visit::Halt;
    }
    return smtk::common::Visit::Continue;
  });
  return foundTask;
}

std::unordered_set<smtk::task::Task*> Instances::topLevelTasks() const
{
  std::unordered_set<smtk::task::Task*> topTasks;
  this->visit([&topTasks](const std::shared_ptr<smtk::task::Task>& task) {
    if (task->parent() == nullptr)
    {
      topTasks.insert(task.get());
    }
    return smtk::common::Visit::Continue;
  });
  return topTasks;
}

} // namespace task
} // namespace smtk
