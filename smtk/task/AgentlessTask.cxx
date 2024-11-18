//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/AgentlessTask.h"

namespace smtk
{
namespace task
{

AgentlessTask::AgentlessTask(
  const Configuration& config,
  Manager& taskManager,
  const std::shared_ptr<smtk::common::Managers>& managers)
  : Task(config, taskManager, managers)
{
}

AgentlessTask::AgentlessTask(
  const Configuration& config,
  const PassedDependencies& dependencies,
  Manager& taskManager,
  const std::shared_ptr<smtk::common::Managers>& managers)
  : Task(config, dependencies, taskManager, managers)
{
}

bool AgentlessTask::updateAgentState(const Agent* agent, State prev, State next, bool signal)
{
  (void)agent; // Not needed by this class
  (void)prev;  // Not needed by this class
  if (m_agentState == next)
  {
    return false;
  }

  State newAgentState = this->computeAgentState();
  if (m_agentState == newAgentState)
  {
    return false; // no change in agent state
  }

  State currentTaskState = this->state();
  m_agentState = newAgentState;
  State newTaskState = this->state();

  if (currentTaskState == newTaskState || !signal)
  {
    return false;
  }

  this->changeState(currentTaskState, newTaskState);
  return true;
}

State AgentlessTask::computeAgentState() const
{
  return this->computeInternalState();
}

State AgentlessTask::computeInternalState() const
{
  return State::Unavailable;
}

bool AgentlessTask::internalStateChanged(State next)
{
  return this->updateAgentState(nullptr, State::Irrelevant, next, true);
}

State AgentlessTask::internalState() const
{
  return this->agentState();
}
} // namespace task
} // namespace smtk
