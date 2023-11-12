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

#include "smtk/common/Managers.h"
#include "smtk/common/TypeName.h"
#include "smtk/operation/Manager.h"
#include "smtk/string/Token.h"

#include "smtk/task/Active.h"
#include "smtk/task/Adaptor.h"
#include "smtk/task/Gallery.h"
#include "smtk/task/Instances.h"
#include "smtk/task/Task.h"
#include "smtk/task/adaptor/Instances.h"

#include "nlohmann/json.hpp"

#include <array>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>

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

  /// Return the managers instance that contains this manager, if it exists.
  smtk::common::Managers::Ptr managers() const { return m_managers.lock(); }
  void setManagers(const smtk::common::Managers::Ptr& managers);

  /// Given a style key, return a style config.
  nlohmann::json getStyle(const smtk::string::Token& styleClass) const;
  nlohmann::json getStyles() const { return m_styles; };
  void setStyles(const nlohmann::json& styles) { m_styles = styles; }

  /// If this manager is owned by a resource (typically a project), return it.
  smtk::resource::Resource* resource() const;

  /// Return a gallery of Task Worklets
  Gallery& gallery() { return m_gallery; }
  const Gallery& gallery() const { return m_gallery; }

  /// Return the set of observers of task events (so you can insert/remove an observer).
  TaskManagerTaskObservers& taskObservers() { return m_taskEvents; }
  /// Return the set of observers of adaptor events (so you can insert/remove an observer).
  TaskManagerAdaptorObservers& adaptorObservers() { return m_adaptorEvents; }
  /// Return the set of observers of workflow events (so you can insert/remove an observer).
  TaskManagerWorkflowObservers& workflowObservers() { return m_workflowEvents; }

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

  TaskInstances m_taskInstances;
  AdaptorInstances m_adaptorInstances;
  Active m_active;
  std::weak_ptr<smtk::common::Managers> m_managers;
  smtk::resource::Resource* m_parent = nullptr;
  nlohmann::json m_styles;
  Gallery m_gallery;

  /// Monitor operation results and, if any involve tasks, invoke task observers.
  smtk::operation::Observers::Key m_taskEventObserver;
  /// Observers to notify when a task is managed/unmanaged by an operation.
  TaskManagerTaskObservers m_taskEvents;
  /// Observers to notify when an adaptor is managed/unmanaged by an operation.
  TaskManagerAdaptorObservers m_adaptorEvents;
  /// Observers to notify when a workflow is created/destroyed/modified by an operation.
  TaskManagerWorkflowObservers m_workflowEvents;
};
} // namespace task
} // namespace smtk

#endif // smtk_task_Manager_h
