//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_Group_h
#define smtk_task_Group_h

#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/resource/Resource.h"
#include "smtk/task/Task.h"

#include "smtk/common/Visit.h"

namespace smtk
{
namespace task
{
class Adaptor;

/**\brief Group is a task that owns children and draws its state from them.
  *
  * A task group exists to organize a set of tasks.
  *
  * The Group instance is responsible for configuring its children, including
  * creating dependencies among them. The Group's state and output are
  * dependent on its children.
  *
  * The group has a "mode," which describes how children are related to
  * one another: when the mode is parallel, children have no dependency on
  * one another and the group itself is dependent on all of its children.
  * When the mode is serial, children must be completed in the
  * order specified (i.e., each successive task is dependent on its
  * predecessor) and the group itself is dependent on the final child task.
  *
  */
class SMTKCORE_EXPORT Group : public Task
{
public:
  smtkTypeMacro(smtk::task::Group);
  smtkSuperclassMacro(smtk::task::Task);
  smtkCreateMacro(smtk::task::Task);

  Group();
  Group(
    const Configuration& config,
    Manager& taskManager,
    const smtk::common::Managers::Ptr& managers = nullptr);
  Group(
    const Configuration& config,
    const PassedDependencies& dependencies,
    Manager& taskManager,
    const smtk::common::Managers::Ptr& managers = nullptr);

  ~Group() override = default;

  void configure(const Configuration& config);

  std::vector<Task::Ptr> children() const;
  bool hasChildren() const override { return !m_children.empty(); }
  smtk::common::Visit visit(RelatedTasks relation, Visitor visitor) const override;

  const std::vector<std::weak_ptr<smtk::task::Adaptor>>& adaptors() const { return m_adaptors; }

  /// Set/get adaptor configuration data passed to/from child tasks.
  void setAdaptorData(const std::string& tagName, Task::Configuration& config);
  const Task::Configuration& adaptorData() const { return m_adaptorData; }
  Task::Configuration& adaptorData() { return m_adaptorData; }
  Task::Configuration adaptorData(const std::string& key) const;

  /// Return the managers used to configure this Group.
  smtk::common::Managers::Ptr managers() const { return m_managers; }

protected:
  /// Check m_resourcesByRole to see if all requirements are met.
  State computeInternalState() const;

  void childStateChanged(Task& child, State prev, State next);

  smtk::common::Managers::Ptr m_managers;
  std::map<Task::Ptr, Task::Observers::Key> m_children;
  std::vector<std::weak_ptr<smtk::task::Adaptor>> m_adaptors;
  Task::Configuration m_adaptorData;
};
} // namespace task
} // namespace smtk

#endif // smtk_task_Group_h
