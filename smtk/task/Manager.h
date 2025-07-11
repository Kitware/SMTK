//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_task_Manager_h
#define smtk_task_Manager_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/common/Categories.h"
#include "smtk/common/Managers.h"
#include "smtk/common/TypeName.h"
#include "smtk/operation/Manager.h"
#include "smtk/string/Token.h"

#include "smtk/task/Active.h"
#include "smtk/task/Adaptor.h"
#include "smtk/task/Agent.h"
#include "smtk/task/Gallery.h"
#include "smtk/task/Instances.h"
#include "smtk/task/PortInstances.h"
#include "smtk/task/Task.h"
#include "smtk/task/adaptor/Instances.h"

#include "smtk/view/Configuration.h"

#include "nlohmann/json.hpp"

#include <array>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>

namespace smtk
{
namespace task
{

/// Indicate that a task is being managed or unmanaged.
///
/// Emitted during operation observation by the task manager.
using TaskManagerTaskObserver =
  std::function<void(smtk::common::InstanceEvent, const std::shared_ptr<Task>&)>;
using TaskManagerTaskObservers = smtk::common::Observers<TaskManagerTaskObserver>;

/// Indicate that an adaptor is being managed or unmanaged.
///
/// Emitted during operation observation by the task manager.
using TaskManagerAdaptorObserver =
  std::function<void(smtk::common::InstanceEvent, const std::shared_ptr<Adaptor>&)>;
using TaskManagerAdaptorObservers = smtk::common::Observers<TaskManagerAdaptorObserver>;

/// Indicate that a workflow is undergoing creation, destruction, or a topology change.
///
/// Emitted during operation observation by the task manager.
using TaskManagerWorkflowObserver =
  std::function<void(const std::set<Task*>&, WorkflowEvent, Task*)>;
using TaskManagerWorkflowObservers = smtk::common::Observers<TaskManagerWorkflowObserver>;

/// A task manager is responsible for creating new tasks.
///
/// Eventually, the task manager will also hold an inventory
/// of created tasks and be a clearinghouse for task state transitions.
class SMTKCORE_EXPORT Manager : smtkEnableSharedPtr(Manager)
{
public:
  smtkTypeMacroBase(smtk::task::Manager);
  smtkCreateMacro(smtk::task::Manager);

  Manager();
  Manager(smtk::resource::Resource* parent);
  virtual ~Manager();
  Manager(const Manager&) = delete;
  void operator=(const Manager&) = delete;

  /// A map of objects grouped by their parent resource.
  ///
  /// If a resource itself is intended for selection, one
  /// entry of its target set of objects will be the null pointer.
  using ResourceObjectMap =
    std::map<smtk::resource::Resource*, std::unordered_set<smtk::resource::PersistentObject*>>;

  /// Managed instances of Task objects (and a registry of Task classes).
  using TaskInstances = smtk::task::Instances;

  /// Return the set of managed task instances.
  ///
  /// This class also acts as a registrar for Task subclasses.
  TaskInstances& taskInstances() { return m_taskInstances; }
  const TaskInstances& taskInstances() const { return m_taskInstances; }

  /// Return a tracker for the active task.
  Active& active() { return m_active; }
  const Active& active() const { return m_active; }

  /// Managed instances of Adaptor objects (and a registry of Adaptor classes).
  using AdaptorInstances = smtk::task::adaptor::Instances;

  /// Return the set of managed adaptor instances.
  ///
  /// This class also acts as a registrar for Adaptor subclasses.
  AdaptorInstances& adaptorInstances() { return m_adaptorInstances; }
  const AdaptorInstances& adaptorInstances() const { return m_adaptorInstances; }

  /// Managed instances of Port objects (and a registry od Port subclasses).
  using PortInstances = smtk::task::PortInstances;

  /// Return the set of managed task-port instances.
  ///
  /// This class also acts as a registrar for Port subclasses.
  PortInstances& portInstances() { return m_portInstances; }
  const PortInstances& portInstances() const { return m_portInstances; }

  /// The signatures the agent factory uses to construct agents.
  using AgentFactory = smtk::common::Factory<smtk::task::Agent, Task*>;

  /// Return a factory to create agents for tasks.
  const AgentFactory& agentFactory() const { return m_agentFactory; }
  AgentFactory& agentFactory() { return m_agentFactory; }

  /// Return the managers instance that contains this manager, if it exists.
  smtk::common::Managers::Ptr managers() const { return m_managers.lock(); }
  void setManagers(const smtk::common::Managers::Ptr& managers);

  /// Given a style key, return a style config.
  nlohmann::json getStyle(const smtk::string::Token& styleClass) const;
  nlohmann::json getStyles() const { return m_styles; };
  void setStyles(const nlohmann::json& styles) { m_styles = styles; }

  /// If this manager is owned by a resource (typically a project), return it.
  smtk::resource::Resource* resource() const;

  /// Given a filter \a spec, return a set of objects grouped by their parent resources.
  ///
  /// The objects are drawn – according to the \a spec – from either ports of the active
  /// task or resources owned by the parent of this task manager (assuming it is a project).
  /// If a resource itself is intended to be part of the result, one of the object pointers
  /// in that resource's target set will be the null pointer.
  ///
  /// The variant that accepts a JSON \a spec is for use inside task-manager style.
  /// The variant that accepts a Configuration::Component \a spec is for use inside
  /// view-configuration XML.
  /// The two forms of \a spec are similar but not identical in order to improve
  /// readability and reduce maintenance overhead.
  ResourceObjectMap workflowObjects(const nlohmann::json& spec, Task* task = nullptr);
  ResourceObjectMap workflowObjects(
    const smtk::view::Configuration::Component& spec,
    Task* task = nullptr);

  /// Return true if the \a resource matches the filter \a spec provided.
  bool isResourceRelevant(
    const std::shared_ptr<smtk::resource::Resource>& resource,
    const nlohmann::json& spec);
  bool isResourceRelevant(smtk::resource::Resource* resource, const nlohmann::json& spec);
  bool isResourceRelevant(
    const std::shared_ptr<smtk::resource::Resource>& resource,
    const smtk::view::Configuration::Component& spec);
  bool isResourceRelevant(
    smtk::resource::Resource* resource,
    const smtk::view::Configuration::Component& spec);

  /// Return a gallery of Task Worklets
  Gallery& gallery() { return m_gallery; }
  const Gallery& gallery() const { return m_gallery; }

  /// Return the category expression constraint for placing worklets at the top-level
  /// of the workflow
  smtk::common::Categories::Expression& toplevelExpression() { return m_expression; }
  const smtk::common::Categories::Expression& toplevelExpression() const { return m_expression; }

  /// Return the set of observers of task events (so you can insert/remove an observer).
  TaskManagerTaskObservers& taskObservers() { return m_taskEvents; }
  /// Return the set of observers of adaptor events (so you can insert/remove an observer).
  TaskManagerAdaptorObservers& adaptorObservers() { return m_adaptorEvents; }
  /// Return the set of observers of workflow events (so you can insert/remove an observer).
  TaskManagerWorkflowObservers& workflowObservers() { return m_workflowEvents; }

  /// Change the ID of a Port
  ///
  /// This method is used to update internal data structures. Please use task::Port::setId
  /// to change the name of the Port - it will call this method with an appropriate lambda.
  bool changePortId(Port* port, std::function<bool()> fp);

  /// Change the Name of a Port
  ///
  /// This method is used to update internal data structures. Please use task::Port::setName
  /// to change the name of the Port - it will call this method with an appropriate lambda.
  bool changePortName(Port* port, const std::string& newName, std::function<bool()> fp);

  /// Use an elevated priority for the task-manager's observation of operations.
  ///
  /// This is done so that task-related events are emitted before other ordinary-priority
  /// operation observations (especially hint-processing).
  static constexpr smtk::operation::Observers::Priority operationObserverPriority()
  {
    return 0x100;
  }

private:
  /// A method invoked when the \a m_manager's operation manager runs an operation.
  int handleOperation(
    const smtk::operation::Operation& op,
    smtk::operation::EventType event,
    smtk::operation::Operation::Result result);

  /// If a port directly references \a obj as a connection, remove \a obj
  /// and potentially notify the consuming port's task.
  bool handlePortDataReferences(smtk::resource::PersistentObject* obj);
  /// Remove an input \a port from upstream output ports.
  bool removeUpstreamMentions(const std::shared_ptr<smtk::task::Port>& port);
  /// Remove an output \a port from all its downstream input ports.
  ///
  /// This may invoke the downstream-task's portDataUpdated() method
  /// (if there are any remaining inputs).
  bool removePortFromDownstream(const std::shared_ptr<smtk::task::Port>& port);

  /// Invoke methods on upstream/downstream objects when \a port is created/modified.
  void handleCreatedOrModifiedPort(const std::shared_ptr<smtk::task::Port>& port);

  TaskInstances m_taskInstances;
  AdaptorInstances m_adaptorInstances;
  PortInstances m_portInstances;
  AgentFactory m_agentFactory;
  Active m_active;
  std::weak_ptr<smtk::common::Managers> m_managers;
  smtk::resource::Resource* m_parent = nullptr;
  nlohmann::json m_styles;
  Gallery m_gallery;
  using PortSet = std::unordered_set<smtk::task::Port*>;
  using PortsForObject = std::unordered_map<smtk::resource::PersistentObject*, PortSet>;
  // Keep track of non-port objects used as inputs to ports so that
  // when one is modified we can signal the consuming port's task.
  PortsForObject m_portData;

  /// Monitor operation results and, if any involve tasks, invoke task observers.
  smtk::operation::Observers::Key m_taskEventObserver;
  /// Observers to notify when a task is managed/unmanaged by an operation.
  TaskManagerTaskObservers m_taskEvents;
  /// Observers to notify when an adaptor is managed/unmanaged by an operation.
  TaskManagerAdaptorObservers m_adaptorEvents;
  /// Observers to notify when a workflow is created/destroyed/modified by an operation.
  TaskManagerWorkflowObservers m_workflowEvents;
  // Expression constraint for placing worklets at the toplevel of a workflow.
  smtk::common::Categories::Expression m_expression;
};
} // namespace task
} // namespace smtk

#endif // smtk_task_Manager_h
