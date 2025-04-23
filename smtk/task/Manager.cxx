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
#include "smtk/task/ObjectsInRoles.h"

#include "smtk/project/Project.h"

#include "smtk/operation/Operation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/resource/Resource.h"

#include "smtk/io/Logger.h"

using namespace smtk::string::literals;

namespace smtk
{
namespace task
{
namespace // anonymous
{

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

nlohmann::json::array_t filtersForSpec(const smtk::view::Configuration::Component& spec)
{
  nlohmann::json::array_t filters;
  for (const auto& filter : spec.children())
  {
    if (filter.name() != "Filter")
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(), "Unhandled child \"" << filter.name() << "\".");
      continue;
    }
    std::string rsrcMatch;
    if (!filter.attribute("Resource", rsrcMatch))
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(), "No resource in \"" << filter.name() << "\". Skipping.");
      continue;
    }
    std::string compMatch;
    filter.attribute("Component", compMatch);
    nlohmann::json::array_t fspec{ rsrcMatch };
    if (!compMatch.empty())
    {
      fspec.emplace_back(compMatch);
    }
    else
    {
      fspec.emplace_back(std::nullptr_t());
    }
    filters.emplace_back(fspec);
  }
  return filters;
}

} // anonymous namespace

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
  // By default allow all worklets to be placed at the top level
  m_expression.setAllPass();
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
  // By default allow all worklets to be placed at the top level
  m_expression.setAllPass();
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

Manager::ResourceObjectMap Manager::workflowObjects(const nlohmann::json& spec, Task* task)
{
  // Filter objects by the \a spec.
  ResourceObjectMap objectMap;

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
      return objectMap;
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
      return objectMap;
    }
  }
  else
  {
    // Accept any resource or component:
    filter = nlohmann::json::array_t({ { "*", "*" }, { "*", nullptr } });
  }

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
      auto* project = dynamic_cast<smtk::project::Project*>(this->resource());
      if (!project)
      {
        return objectMap;
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
      auto* currentTask = task ? task : this->active().task();
      if (!currentTask || !source.contains("port"))
      {
        smtkErrorMacro(smtk::io::Logger::instance(), "No active task or no \"port\" specified.");
        return objectMap;
      }

      // We are asked to find objects on a port of the (now) active task.
      // Determine whether we are filtering on role or not.
      smtk::string::Token sourceRole;
      if (source.contains("role"))
      {
        sourceRole = source.at("role").get<smtk::string::Token>();
      }
      // Find the correct port:
      const auto& taskPortMap = currentTask->ports();
      auto it = taskPortMap.find(source["port"]);
      if (it == taskPortMap.end())
      {
        return objectMap;
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
    smtk::resource::Resource* resource = dynamic_cast<smtk::resource::Resource*>(object);
    if (auto* comp = dynamic_cast<smtk::resource::Component*>(object))
    {
      resource = comp->parentResource();
    }
    if (!resource)
    {
      continue;
    }
    objectMap[resource].insert(object == resource ? nullptr : object);
  }

  return objectMap;
}

Manager::ResourceObjectMap Manager::workflowObjects(
  const smtk::view::Configuration::Component& spec,
  Task* task)
{
  // To avoid dueling implementations, we'll convert \a spec into JSON and pass
  // it to the variant above.
  nlohmann::json jsonSpec;
  nlohmann::json::array_t controls;
  for (const auto& entry : spec.children())
  {
    smtk::string::Token tagName = entry.name();
    switch (tagName.id())
    {
      case "ActiveTaskPort"_hash:
      {
        auto portName = entry.attributeAsString("Port");
        auto roleName = entry.attributeAsString("Role");
        if (portName.empty())
        {
          smtkErrorMacro(smtk::io::Logger::instance(), "Missing a port name.");
          continue;
        }
        nlohmann::json sourceSpec{ { "type", "active task port" }, { "port", portName } };
        if (!roleName.empty())
        {
          sourceSpec["role"] = roleName;
        }
        auto filters = filtersForSpec(entry);
        if (!filters.empty())
        {
          jsonSpec["filter"] = filters;
        }
        jsonSpec["source"] = sourceSpec;
      }
      break;
      case "ProjectResources"_hash:
      {
        nlohmann::json sourceSpec{ { "type", "project resources" } };
        auto filters = filtersForSpec(entry);
        if (!filters.empty())
        {
          jsonSpec["filter"] = filters;
        }
        jsonSpec["source"] = sourceSpec;
      }
      break;
      case "Control"_hash:
      {
        auto controlType = entry.attributeAsString("Type");
        if (controlType.empty())
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(), "Control tag must provide a Type attribute.");
          continue;
        }
        controls.emplace_back(controlType);
      }
      break;
      default:
      {
        smtkWarningMacro(
          smtk::io::Logger::instance(),
          "Unhandled specification tag <" << tagName.data() << ">. Skipping.");
      }
    }
  }
  if (!controls.empty())
  {
    jsonSpec["controls"] = controls;
  }
  auto objMap = this->workflowObjects(jsonSpec, task);
  return objMap;
}

bool Manager::isResourceRelevant(
  const std::shared_ptr<smtk::resource::Resource>& resource,
  const nlohmann::json& filter)
{
  return filterObject(filter, resource.get());
}

bool Manager::isResourceRelevant(smtk::resource::Resource* resource, const nlohmann::json& filter)
{
  return filterObject(filter, resource);
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
    bool didUpdate = false;
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
          didUpdate = true;
        }
      }
    }
    // Notify agents that the port data may have changed if they have not
    // already been notified due to a direct (non-port) connection above.
    if (!didUpdate)
    {
      port->parent()->portDataUpdated(port.get());
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

bool Manager::changePortName(Port* port, const std::string& newName, std::function<bool()> fp)
{
  (void)port;
  (void)newName;
  // Currently there are no internal data structures that need to be called so just call
  // the function passed in
  return fp();
}

} // namespace task
} // namespace smtk
