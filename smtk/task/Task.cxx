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
#include "smtk/task/Manager.h"

#include "smtk/string/json/jsonToken.h"
#include "smtk/task/json/Helper.h"

#include "smtk/io/Logger.h"

#include <stdexcept>

namespace smtk
{
namespace task
{

void workflowsOfTask(
  Task* task,
  std::set<smtk::task::Task*>& workflows,
  std::set<smtk::task::Task*>& visited)
{
  if (visited.find(task) != visited.end())
  {
    return;
  }
  visited.insert(task);
  if (task->m_dependencies.empty())
  {
    workflows.insert(task);
  }
  else
  {
    for (const auto& weakDependency : task->m_dependencies)
    {
      if (auto dependency = weakDependency.first.lock())
      {
        workflowsOfTask(dependency.get(), workflows, visited);
      }
    }
  }
}

std::set<smtk::task::Task*> workflowsOfTask(Task& task)
{
  std::set<smtk::task::Task*> result;
  std::set<smtk::task::Task*> visited;
  workflowsOfTask(&task, result, visited);
  return result;
}

constexpr const char* const Task::type_name;

Task::Task()
{
  this->setId();
}

Task::Task(const Configuration& config, const std::shared_ptr<smtk::common::Managers>& managers)
{
  static bool once = false;
  if (!once)
  {
    once = true;
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "Note to developers; call a constructor that takes a task::Manager&.");
  }
  this->setId();
  if (managers && managers->contains<smtk::task::Manager::Ptr>())
  {
    m_manager = managers->get<smtk::task::Manager::Ptr>();
  }
  this->configure(config);
}

Task::Task(
  const Configuration& config,
  Manager& taskManager,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  this->setId();
  (void)managers;
  m_manager = taskManager.shared_from_this();
  this->configure(config);
}

Task::Task(
  const Configuration& config,
  const PassedDependencies& dependencies,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  static bool once = false;
  if (!once)
  {
    once = true;
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "Note to developers; call a constructor that takes a task::Manager&.");
  }
  this->setId();
  if (managers->contains<smtk::task::Manager::Ptr>())
  {
    m_manager = managers->get<smtk::task::Manager::Ptr>();
  }
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

Task::Task(
  const Configuration& config,
  const PassedDependencies& dependencies,
  Manager& taskManager,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  this->setId();
  (void)managers;
  m_manager = taskManager.shared_from_this();
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
  if (config.contains("style"))
  {
    try
    {
      m_style = config.at("style").get<std::set<smtk::string::Token>>();
    }
    catch (std::exception&)
    {
    }
  }
  if (config.contains("strict-dependencies"))
  {
    m_strictDependencies = config.at("strict-dependencies").get<bool>();
  }
  if (config.contains("state"))
  {
    bool valid;
    m_internalState = stateEnum(config.at("state").get<std::string>(), &valid);
    if (valid)
    {
      m_completed = (m_internalState == State::Completed);
    }
  }
  auto& helper = smtk::task::json::Helper::instance();
  if (!helper.topLevel())
  {
    m_parent = helper.tasks().unswizzle(1);
  }
}

void Task::setTitle(const std::string& title)
{
  m_title = title;
}

bool Task::addStyle(const smtk::string::Token& styleClass)
{
  if (styleClass.id() == smtk::string::Manager::Invalid)
  {
    return false;
  }
  return m_style.insert(styleClass).second;
}

bool Task::removeStyle(const smtk::string::Token& styleClass)
{
  return m_style.erase(styleClass) > 0;
}

bool Task::clearStyle()
{
  bool didModify = !m_style.empty();
  m_style.clear();
  return didModify;
}

bool Task::getViewData(smtk::common::TypeContainer& configuration) const
{
  (void)configuration;
  return false;
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
      case State::Irrelevant:
      case State::Completable:
        if (m_strictDependencies)
        {
          s = State::Unavailable;
        }
        break;
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

bool Task::editableCompletion() const
{
  switch (this->state())
  {
    default:
    case State::Irrelevant:
    case State::Unavailable:
    case State::Incomplete:
      return false;
    case State::Completable:
    case State::Completed:
      return true;
  }
}

bool Task::markCompleted(bool completed)
{
  switch (this->state())
  {
    case State::Irrelevant:
    case State::Unavailable:
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
  // Was this task previously without dependencies? If so,
  // it used to be a workflow head and we must notify
  // the task-manager's instances object that a head task
  // is being removed.
  bool wasHead = m_dependencies.empty();
  if (wasHead)
  {
    if (auto taskManager = m_manager.lock())
    {
      taskManager->taskInstances().workflowEvent({ this }, WorkflowEvent::Destroyed, nullptr);
    }
  }
  State prev = this->state();
  m_dependencies.insert(std::make_pair(
    (const std::weak_ptr<Task>)(dependency),
    dependency->observers().insert([this](Task& dependency, State prev, State next) {
      bool didChange = this->updateState(dependency, prev, next);
      (void)didChange;
    })));
  dependency->m_dependents.insert(this->shared_from_this());
  if (wasHead)
  {
    if (auto taskManager = m_manager.lock())
    {
      taskManager->taskInstances().workflowEvent(
        smtk::task::workflowsOfTask(*this), WorkflowEvent::TaskAdded, this);
    }
  }
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
  bool willBeHead =
    m_dependencies.size() == 1 && m_dependencies.begin()->first.lock() == dependency;
  if (willBeHead)
  {
    if (auto taskManager = m_manager.lock())
    {
      taskManager->taskInstances().workflowEvent(
        smtk::task::workflowsOfTask(*this), WorkflowEvent::TaskRemoved, nullptr);
    }
  }
  bool didRemove = m_dependencies.erase(dependency) > 0;
  if (didRemove)
  {
    if (willBeHead)
    {
      if (auto taskManager = m_manager.lock())
      {
        taskManager->taskInstances().workflowEvent({ this }, WorkflowEvent::Created, nullptr);
      }
    }
    State next = this->state();
    if (prev != next)
    {
      this->changeState(prev, next);
    }
    return true;
  }
  return false;
}

smtk::common::Visit Task::visit(RelatedTasks relation, Visitor visitor) const
{
  smtk::common::Visit status = smtk::common::Visit::Continue;
  switch (relation)
  {
    case RelatedTasks::Depend:
      for (const auto& entry : m_dependencies)
      {
        auto dep = entry.first.lock();
        if (dep)
        {
          if (visitor(*dep) == smtk::common::Visit::Halt)
          {
            status = smtk::common::Visit::Halt;
            break;
          }
        }
      }
      break;
    case RelatedTasks::Child:
      break;
  }
  return status;
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
  State limit = m_strictDependencies ? State::Completed : State::Completable;
  bool dependencyNowUnblocked = (prev < limit && next >= limit);
  bool dependencyNowBlocking = (prev >= limit && next < limit);

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

  State taskState = m_internalState;
  // If the internal state is Completable and m_completed is true then externally the task state
  // is Completed
  if (m_completed && (taskState == State::Completable))
  {
    taskState = State::Completed;
  }

  if (dependencyNowUnblocked)
  {
    // All other tasks are also unblocked, we changed from Unavailable to the task state
    this->changeState(State::Unavailable, taskState);
  }
  else if (dependencyNowBlocking)
  {
    // All other tasks are also unblocked, we changed from task state to Unavailable.
    this->changeState(taskState, State::Unavailable);
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

void Task::setId()
{
  // For now, create a string with the address of the task and hash it.
  std::ostringstream uid;
  uid << std::hex << this;
  m_id = uid.str();
}

} // namespace task
} // namespace smtk
