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
#include "smtk/attribute/ResourceItem.h"

#include "smtk/resource/Resource.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace task
{

constexpr const char* const Manager::type_name;

Manager::Manager()
  : m_taskInstances(*this)
  , m_portInstances(*this)
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
  , m_portInstances(*this)
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
  auto expungedResources = result->findResource("resourcesToExpunge");
  bool postWorkflows = false;
  // First, clean up m_portData
  for (const auto& comp : *expunged)
  {
    if (auto port = std::dynamic_pointer_cast<smtk::task::Port>(comp))
    {
      if (port->direction() == Port::Direction::In)
      {
        // Deleting an input port; simply remove the matching
        // upstream connection. There is no notification that
        // needs to happen.
        this->removeUpstreamMentions(port);
      }
      else
      {
        // Deleting an output port. All downstream ports
        // connected to this port that have multiple
        // inputs need to be told to reconfigure (without the
        // data from this port).
        this->removePortFromDownstream(port);
      }
      // Now that we've traversed connections, clear them.
      // This is defensive programming; we don't want stale pointers
      // in memory that is about to be freed.
      port->connections().clear();
    }
    else
    {
      this->handlePortDataReferences(comp.get());
    }
  }
  if (expungedResources)
  {
    // See if any removed resources were used as port inputs.
    for (const auto& resource : *expungedResources)
    {
      this->handlePortDataReferences(resource.get());
    }
  }
  // TODO: Handle workflows
  for (const auto& comp : *created)
  {
    if (auto port = std::dynamic_pointer_cast<smtk::task::Port>(comp))
    {
      this->handleCreatedOrModifiedPort(port);
    }
    else if (auto task = std::dynamic_pointer_cast<smtk::task::Task>(comp))
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
    if (auto port = std::dynamic_pointer_cast<smtk::task::Port>(comp))
    {
      this->handleCreatedOrModifiedPort(port);
    }
    else if (auto task = std::dynamic_pointer_cast<smtk::task::Task>(comp))
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

bool Manager::handlePortDataReferences(smtk::resource::PersistentObject* obj)
{
  // Any non-Port object may be an input to a Port.
  // Look in the m_portData map to see if this is the case.
  auto it = m_portData.find(obj);
  if (it != m_portData.end())
  {
    // The object is connected to downstream input ports.
    // Remove the connection and – if there are any remaining
    // connections – notify the port to update.
    for (auto* downstreamPort : it->second)
    {
      if (downstreamPort->connections().erase(obj) > 0 && !downstreamPort->connections().empty())
      {
        auto* downstreamTask = downstreamPort->parent();
        if (downstreamTask)
        {
          downstreamTask->portDataUpdated(downstreamPort);
        }
      }
    }
    // Remove the object from our lookup table:
    m_portData.erase(it);
    return true;
  }
  return false;
}

bool Manager::removeUpstreamMentions(const std::shared_ptr<smtk::task::Port>& port)
{
  bool didRemove = false;
  if (port->direction() != Port::Direction::In)
  {
    return didRemove;
  }
  for (auto* upstreamObject : port->connections())
  {
    if (auto* upstreamPort = dynamic_cast<smtk::task::Port*>(upstreamObject))
    {
      didRemove |= upstreamPort->connections().erase(port.get()) > 0;
    }
  }
  return didRemove;
}

bool Manager::removePortFromDownstream(const std::shared_ptr<smtk::task::Port>& port)
{
  bool didRemove = false;
  if (port->direction() != Port::Direction::Out)
  {
    return didRemove;
  }
  for (auto* downstreamObject : port->connections())
  {
    if (auto* downstreamPort = dynamic_cast<smtk::task::Port*>(downstreamObject))
    {
      bool removed = downstreamPort->connections().erase(port.get()) > 0;
      if (removed)
      {
        didRemove = true;
        if (!downstreamPort->connections().empty())
        {
          auto* downstreamTask = downstreamPort->parent();
          if (downstreamTask)
          {
            downstreamTask->portDataUpdated(downstreamPort);
          }
        }
      }
    }
    // NB: We do not currently allow non-Port objects to be downstream of a port.
    //     If that changes, there may be more work to do here.
  }
  return didRemove;
}

void Manager::handleCreatedOrModifiedPort(const std::shared_ptr<smtk::task::Port>& port)
{
  if (port->direction() == Port::Direction::Out)
  {
    // If an output port was created/modified, inform any downstream tasks
    // it has so they can update their state.
    for (auto* conn : port->connections())
    {
      if (auto* downstreamPort = dynamic_cast<smtk::task::Port*>(conn))
      {
        auto* downstreamTask = downstreamPort->parent();
        if (downstreamTask)
        {
          downstreamTask->portDataUpdated(downstreamPort);
        }
      }
    }
  }
  else
  {
    // If an input port was created/modified, index any non-Port connections
    // so that we can inform the port when the object is modified
    // or expunged.
    for (auto* conn : port->connections())
    {
      auto* connPort = dynamic_cast<smtk::task::Port*>(conn);
      if (!connPort)
      {
        // If (1) an input port was modified and (2) any connections are
        // non-port objects and (3) the objects were not already being
        // monitored for the port; notify the input port to process its data.
        // Note that if "conn" is already being monitored, it will only
        // notify the task if "conn" is marked as modified in the operation's
        // result.
        if (m_portData[conn].insert(port.get()).second)
        {
          port->parent()->portDataUpdated(port.get());
        }
      }
    }
  }
}

bool Manager::changePortId(Port* port, std::function<bool()> fp)
{
  (void)port;
  // Currently there are no internal data structures that need to be called so just call
  // the function passed in
  return fp();
}

void Manager::changePortName(Port* port, std::function<void()> fp)
{
  (void)port;
  // Currently there are no internal data structures that need to be called so just call
  // the function passed in
  fp();
}

} // namespace task
} // namespace smtk
