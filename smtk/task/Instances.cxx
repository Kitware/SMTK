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

namespace smtk
{
namespace task
{

Instances::Instances()
  : m_workflowObservers([this](WorkflowObserver& observer) {
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

} // namespace task
} // namespace smtk
