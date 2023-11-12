//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
/*! \file */
#ifndef smtk_task_Instances_h
#define smtk_task_Instances_h

#include "smtk/common/Instances.h"
#include "smtk/common/Managers.h"
#include "smtk/common/TypeName.h"

#include "smtk/string/Token.h"

#include "smtk/task/Task.h"

namespace smtk
{
namespace task
{
class Manager;

/// An enum for events in the lifecycle of a workflow (tree of tasks).
enum class WorkflowEvent
{
  Created,     //!< A workflow has been created.
  TaskAdded,   //!< A task has been added to the workflow.
  TaskRemoved, //!< A task has been removed from the workflow.
  Destroyed, //!< The tasks in the workflow have become unmanaged or dependent on another workflow.
  Resuming   //!< Notifications had been paused and now are not.
             //!< The reported workflow heads are all extant workflows current.
  //!< This event is also used to initialize observers added after tasks have been created.
};

using TaskInstancesBase = smtk::common::Instances<
  smtk::task::Task,
  void,
  std::tuple<
    smtk::task::Task::Configuration&,
    smtk::task::Manager&,
    std::shared_ptr<smtk::common::Managers>>,
  std::tuple<
    smtk::task::Task::Configuration&,
    smtk::task::Task::PassedDependencies,
    smtk::task::Manager&,
    std::shared_ptr<smtk::common::Managers>>>;

/// Track smtk::task::Task objects with smtk::common::Instances.
///
/// This class adds methods to create tasks by name and will
/// eventually index tasks by UUID and name.
class SMTKCORE_EXPORT Instances : public TaskInstancesBase
{
public:
  smtkTypeMacroBase(smtk::task::Instances);
  smtkSuperclassMacro(smtk::task::TaskInstancesBase);

  Instances(Manager& taskManager);
  Instances(const Instances&) = delete;
  void operator=(const Instances&) = delete;
  virtual ~Instances() = default;

  ///@{
  /// Create a task given its class name and optionally more configuration data.
  ///
  /// These override the base factory methods by supplying the task manager.
  Task::Ptr createFromName(const std::string& taskType);
  Task::Ptr createFromName(
    const std::string& taskType,
    Task::Configuration& configuration,
    std::shared_ptr<smtk::common::Managers> managers);
  Task::Ptr createFromName(
    const std::string& taskType,
    smtk::task::Task::Configuration& configuration,
    smtk::task::Task::PassedDependencies& dependencies,
    std::shared_ptr<smtk::common::Managers> managers);
  ///@}

  /// Returns the tasks with the given name
  std::set<smtk::task::Task::Ptr> findByName(const std::string& name) const;
  SMTK_DEPRECATED_IN_23_11("Use findByName() instead.")
  std::set<smtk::task::Task::Ptr> findByTitle(const std::string& name) const
  {
    return this->findByName(name);
  }

  /// Returns the task with the given ID.
  smtk::task::Task::Ptr findById(const smtk::common::UUID& taskId) const;

protected:
  Manager& m_taskManager;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_Instances_h
