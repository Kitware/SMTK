//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/task/Manager.h"

#include "smtk/operation/Operation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/resource/Resource.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace task
{

constexpr const char* const Manager::type_name;

Manager::Manager()
  : m_taskInstances(*this)
  , m_active(&m_taskInstances)
  , m_gallery(this)
  , m_taskEvents([this](TaskManagerTaskObserver& observer) {
    this->taskInstances().visit([&observer](const Task::Ptr& task) {
      observer(smtk::common::InstanceEvent::Managed, task);
      return smtk::common::Visit::Continue;
    });
  })
  , m_adaptorEvents([this](TaskManagerAdaptorObserver& observer) {
    this->adaptorInstances().visit([&observer](const Adaptor::Ptr& adaptor) {
      observer(smtk::common::InstanceEvent::Managed, adaptor);
      return smtk::common::Visit::Continue;
    });
  })
  , m_workflowEvents([this](TaskManagerWorkflowObserver& observer) {
    // Initialize an observer with all the extant workflow head-tasks.
    // Basically, iterate over all tasks computing the set of all workflow heads
    // and signal them with "WorkflowEvent::Resume".
    std::set<Task*> workflows;
    std::set<Task*> visited;
    this->taskInstances().visit([&workflows, &visited](const std::shared_ptr<Task>& task) {
      smtk::task::workflowsOfTask(task.get(), workflows, visited);
      return smtk::common::Visit::Continue;
    });
    observer(workflows, WorkflowEvent::Resuming, nullptr);
  })
{
}

Manager::Manager(smtk::resource::Resource* parent)
  : m_taskInstances(*this)
  , m_active(&m_taskInstances)
  , m_parent(parent)
  , m_gallery(this)
  , m_taskEvents([this](TaskManagerTaskObserver& observer) {
    this->taskInstances().visit([&observer](const Task::Ptr& task) {
      observer(smtk::common::InstanceEvent::Managed, task);
      return smtk::common::Visit::Continue;
    });
  })
  , m_adaptorEvents([this](TaskManagerAdaptorObserver& observer) {
    this->adaptorInstances().visit([&observer](const Adaptor::Ptr& adaptor) {
      observer(smtk::common::InstanceEvent::Managed, adaptor);
      return smtk::common::Visit::Continue;
    });
  })
  , m_workflowEvents([this](TaskManagerWorkflowObserver& observer) {
    // Initialize an observer with all the extant workflow head-tasks.
    // Basically, iterate over all tasks computing the set of all workflow heads
    // and signal them with "WorkflowEvent::Resume".
    std::set<Task*> workflows;
    std::set<Task*> visited;
    this->taskInstances().visit([&workflows, &visited](const std::shared_ptr<Task>& task) {
      smtk::task::workflowsOfTask(task.get(), workflows, visited);
      return smtk::common::Visit::Continue;
    });
    observer(workflows, WorkflowEvent::Resuming, nullptr);
  })
{
}

Manager::~Manager() = default;

void Manager::setManagers(const smtk::common::Managers::Ptr& managers)
{
  smtk::operation::Manager::Ptr opMgr;
  auto localManagers = m_managers.lock();
  if (localManagers == managers)
  {
    return;
  }
  // Release the old operation-manager observer:
  if (localManagers)
  {
    opMgr = localManagers->get<smtk::operation::Manager::Ptr>();
    if (opMgr)
    {
      opMgr->observers().erase(m_taskEventObserver);
    }
  }
  m_managers = managers;
  if (managers)
  {
    opMgr = managers->get<smtk::operation::Manager::Ptr>();
    if (opMgr)
    {
      m_taskEventObserver = opMgr->observers().insert(
        [this](
          const smtk::operation::Operation& op,
          smtk::operation::EventType event,
          smtk::operation::Operation::Result result) {
          return this->handleOperation(op, event, result);
        },
        Manager::operationObserverPriority(),
        false,
        "Operation to task observer adaptor.");
    }
  }
}

nlohmann::json Manager::getStyle(const smtk::string::Token& styleClass) const
{
  if (this->m_styles.contains(styleClass.data()))
  {
    return this->m_styles.at(styleClass.data());
  }
  return nlohmann::json();
}

smtk::resource::Resource* Manager::resource() const
{
  return m_parent;
}

int Manager::handleOperation(
  const smtk::operation::Operation& op,
  smtk::operation::EventType event,
  smtk::operation::Operation::Result result)
{
  (void)op;
  switch (event)
  {
    default:
    case smtk::operation::EventType::WILL_OPERATE:
      return 0;
    case smtk::operation::EventType::DID_OPERATE:
      break;
  }
  auto created = result->findComponent("created");
  auto modified = result->findComponent("modified");
  auto expunged = result->findComponent("expunged");
  bool postWorkflows = false;
  // TODO: Handle workflows
  for (const auto& comp : *created)
  {
    if (auto task = std::dynamic_pointer_cast<smtk::task::Task>(comp))
    {
      postWorkflows = true;
      m_taskEvents(smtk::common::InstanceEvent::Managed, task);
    }
    else if (auto adaptor = std::dynamic_pointer_cast<smtk::task::Adaptor>(comp))
    {
      m_adaptorEvents(smtk::common::InstanceEvent::Managed, adaptor);
    }
  }
  for (const auto& comp : *modified)
  {
    if (auto task = std::dynamic_pointer_cast<smtk::task::Task>(comp))
    {
      postWorkflows = true;
      m_taskEvents(smtk::common::InstanceEvent::Modified, task);
    }
    else if (auto adaptor = std::dynamic_pointer_cast<smtk::task::Adaptor>(comp))
    {
      m_adaptorEvents(smtk::common::InstanceEvent::Modified, adaptor);
    }
  }
  for (const auto& comp : *expunged)
  {
    if (auto task = std::dynamic_pointer_cast<smtk::task::Task>(comp))
    {
      postWorkflows = true;
      m_taskEvents(smtk::common::InstanceEvent::Unmanaged, task);
    }
    else if (auto adaptor = std::dynamic_pointer_cast<smtk::task::Adaptor>(comp))
    {
      m_adaptorEvents(smtk::common::InstanceEvent::Unmanaged, adaptor);
    }
  }
  if (postWorkflows)
  {
    std::set<Task*> workflows;
    std::set<Task*> visited;
    this->taskInstances().visit([&workflows, &visited](const std::shared_ptr<Task>& task) {
      smtk::task::workflowsOfTask(task.get(), workflows, visited);
      return smtk::common::Visit::Continue;
    });
    m_workflowEvents(workflows, WorkflowEvent::Resuming, nullptr);
  }

  return 0;
}

} // namespace task
} // namespace smtk
