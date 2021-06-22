//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/Task.h"

#include <stdexcept>

namespace smtk
{
namespace task
{

Task::Task() = default;

Task::Task(const Configuration& config, const std::shared_ptr<smtk::common::Managers>& managers)
{
  (void)managers;
  this->configure(config);
}

Task::Task(
  const Configuration& config,
  const PassedDependencies& dependencies,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  (void)managers;
  this->configure(config);
  for (const auto& dependency : dependencies)
  {
    m_dependencies.insert(std::make_pair(
      (const std::weak_ptr<Task>)(dependency),
      dependency->observers().insert([this](Task& dependency, State prev, State next) {
        bool didChange = this->updateState(dependency, prev, next);
        (void)didChange;
      })));
  }
}

void Task::configure(const Configuration& config)
{
  if (!config.is_object())
  {
    throw std::logic_error("Invalid configuration passed to Task constructor.");
  }
  if (config.contains("title"))
  {
    m_title = config.at("title").get<std::string>();
  }
  if (config.contains("completed"))
  {
    m_completed = config.at("completed").get<bool>();
  }
}

void Task::setTitle(const std::string& title)
{
  m_title = title;
}

State Task::state() const
{
  State s = m_internalState;
  for (const auto& dd : m_dependencies)
  {
    const auto& dependency(dd.first.lock());
    switch (dependency->state())
    {
      case State::Unavailable:
      case State::Incomplete:
        s = State::Unavailable;
        break;
      case State::Completable:
      case State::Completed:
        break;
    }
    if (s == State::Unavailable)
    {
      break;
    }
  }
  if (s == State::Completable && m_completed)
  {
    s = State::Completed;
  }
  return s;
}

bool Task::markCompleted(bool completed)
{
  switch (this->state())
  {
    case State::Unavailable: // fall through
    case State::Incomplete:
      return false;
    case State::Completable:
      if (!completed)
      {
        return false;
      }
      this->changeState(State::Completable, State::Completed);
      break;
    case State::Completed:
      if (completed)
      {
        return false;
      }
      this->changeState(State::Completed, State::Completable);
  }
  return true;
}

Task::PassedDependencies Task::dependencies() const
{
  PassedDependencies result;
  for (const auto& dependency : m_dependencies)
  {
    if (auto task = dependency.first.lock())
    {
      result.insert(task);
    }
  }
  return result;
}

bool Task::addDependency(const std::shared_ptr<Task>& dependency)
{
  if (!dependency)
  {
    return false;
  }
  auto it = m_dependencies.find(dependency);
  if (it != m_dependencies.end())
  {
    return false;
  }
  State prev = this->state();
  m_dependencies.insert(std::make_pair(
    (const std::weak_ptr<Task>)(dependency),
    dependency->observers().insert([this](Task& dependency, State prev, State next) {
      bool didChange = this->updateState(dependency, prev, next);
      (void)didChange;
    })));
  // Now determine if this dependency changed the state.
  State next = this->state();
  if (prev != next)
  {
    bool didChange = this->changeState(prev, next);
    (void)didChange;
  }
  return true;
}

bool Task::removeDependency(const std::shared_ptr<Task>& dependency)
{
  State prev = this->state();
  bool didRemove = m_dependencies.erase(dependency) > 0;
  if (didRemove)
  {
    State next = this->state();
    if (prev != next)
    {
      this->changeState(prev, next);
    }
    return true;
  }
  return false;
}

bool Task::changeState(State previous, State next)
{
  if (previous == next)
  {
    return false;
  }
  m_completed = next == State::Completed;
  m_observers(*this, previous, next);
  return true;
}

bool Task::updateState(Task& dependency, State prev, State next)
{
  // If a dependent task becomes blocking or non-blocking,
  // check other tasks and see if we should change our state
  bool dependencyNowUnblocked = (prev < State::Completable && next > State::Incomplete);
  bool dependencyNowBlocking = (prev > State::Incomplete && next < State::Completable);

  // No significant change to our dependency.
  if (!dependencyNowUnblocked && !dependencyNowBlocking)
  {
    return false;
  }

  if (dependencyNowUnblocked && dependencyNowBlocking)
  {
    throw std::logic_error("Impossible state.");
  }

  bool remainingDepsUnblocked = true;
  for (const auto& entry : m_dependencies)
  {
    if (auto remainingDep = entry.first.lock())
    {
      if (remainingDep.get() == &dependency)
      {
        continue;
      }
      remainingDepsUnblocked &= remainingDep->state() > State::Incomplete;
    }
  }
  if (!remainingDepsUnblocked)
  {
    // The dependent task would have caused a change, but other dependencies
    // keep us in our current state.
    return false;
  }
  if (dependencyNowUnblocked)
  {
    // All other tasks are also unblocked, we changed from Unavailable to Completable or Complete.
    this->changeState(State::Unavailable, m_completed ? State::Completed : State::Completable);
  }
  else if (dependencyNowBlocking)
  {
    // All other tasks are also unblocked, we changed from Completable or Complete to Unavailable.
    this->changeState(m_completed ? State::Completed : State::Completable, State::Unavailable);
  }
  return true;
}

bool Task::internalStateChanged(State next)
{
  if (m_internalState == next)
  {
    return false;
  }
  State previousFinalState = this->state();
  m_internalState = next;
  State nextFinalState = this->state();
  if (previousFinalState != nextFinalState)
  {
    this->changeState(previousFinalState, nextFinalState);
    return true;
  }
  return false;
}

} // namespace task
} // namespace smtk
