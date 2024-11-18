//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_AgentlessTask_h
#define smtk_task_AgentlessTask_h

#include "smtk/task/Task.h"

namespace smtk
{
namespace task
{

class Port;
class PortData;
class Task;

///\brief AgentlessTask is a base class for all Tasks that do not use agents.
///
/// This class introduces the concept of an internal state that is mapped
/// into the base task class's agent state

class SMTKCORE_EXPORT AgentlessTask : public smtk::task::Task
{
public:
  smtkTypeMacro(smtk::task::AgentlessTask);
  smtkSuperclassMacro(smtk::task::Task);
  smtkCreateMacro(smtk::resource::PersistentObject);

  AgentlessTask() = default;
  AgentlessTask(
    const Configuration& config,
    Manager& taskManager,
    const std::shared_ptr<smtk::common::Managers>& managers = nullptr);
  AgentlessTask(
    const Configuration& config,
    const PassedDependencies& dependencies,
    Manager& taskManager,
    const std::shared_ptr<smtk::common::Managers>& managers = nullptr);

  ~AgentlessTask() override = default;

  bool updateAgentState(const Agent* agent, State prev, State next, bool signal) override;
  State internalState() const;

protected:
  State computeAgentState() const override;
  /// This method computes the internal of the task and needs to be overridden
  /// by derived classes.
  virtual State computeInternalState() const;
  bool internalStateChanged(State next);
};

} // namespace task
} // namespace smtk

#endif // smtk_task_Task_h
