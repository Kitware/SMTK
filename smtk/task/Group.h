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
  Group(const Configuration& config, const smtk::common::Managers::Ptr& managers = nullptr);
  Group(
    const Configuration& config,
    const PassedDependencies& dependencies,
    const smtk::common::Managers::Ptr& managers = nullptr);

  ~Group() override = default;

  void configure(const Configuration& config);

  std::vector<Task::Ptr> children() const;

protected:
  /// Check m_resourcesByRole to see if all requirements are met.
  State computeInternalState() const;

  smtk::common::Managers::Ptr m_managers;
  std::map<Task::Ptr, Task::Observers::Key> m_children;
};
} // namespace task
} // namespace smtk

#endif // smtk_task_Group_h
