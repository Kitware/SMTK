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
  , m_workflowObservers([this](WorkflowObserver& observer) {
    // Initialize an observer with all the extant workflow head-tasks.
    // Basically, iterate over all tasks computing the set of all workflow heads
    // and signal them with "WorkflowEvent::Resume".
    std::set<Task*> workflows;
    std::set<Task*> visited;
    this->visit([&workflows, &visited](const std::shared_ptr<Task>& task) {
      smtk::task::workflowsOfTask(task.get(), workflows, visited);
      return smtk::common::Visit::Continue;
    });
    observer(workflows, WorkflowEvent::Resuming, nullptr);
  })
{
}

bool Instances::pauseWorkflowNotifications(bool doPause)
{
  if (doPause == m_workflowNotificationsPaused)
  {
    return false;
  }
  m_workflowNotificationsPaused = doPause;
  if (!m_workflowNotificationsPaused)
  {
    // Notify of changes at end of pause...
    // Basically, iterate over all tasks computing the set of all workflow heads
    // and signal them with "WorkflowEvent::Resume".
    // This avoids having to keep and collapse diffs of all changes during a pause.
    std::set<Task*> workflows;
    std::set<Task*> visited;
    this->visit([&workflows, &visited](const std::shared_ptr<Task>& task) {
      smtk::task::workflowsOfTask(task.get(), workflows, visited);
      return smtk::common::Visit::Continue;
    });
    this->workflowEvent(workflows, WorkflowEvent::Resuming, nullptr);
  }
  else
  {
    m_needNotification = true;
  }
  return true;
}

bool Instances::workflowEvent(const std::set<Task*>& workflows, WorkflowEvent event, Task* subject)
{
  if (m_workflowNotificationsPaused)
  {
    m_needNotification = true;
    return false;
  }
  m_needNotification = false;
  m_workflowObservers(workflows, event, subject);
  return true;
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

} // namespace task
} // namespace smtk
