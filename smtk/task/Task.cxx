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

#include "smtk/task/Agent.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Port.h"
#include "smtk/task/PortData.h"

#include "smtk/string/json/jsonToken.h"
#include "smtk/task/json/Helper.h"

#include "smtk/io/Logger.h"

#include "smtk/common/UUIDGenerator.h"

#include <stdexcept>

using namespace smtk::string::literals;

namespace smtk
{
namespace task
{

namespace
{

bool checkDependenciesRecursive(Task* check, Task* cycle, std::set<Task*>& visited)
{
  // If we have visited this task before, terminate recursion.
  if (visited.find(check) != visited.end())
  {
    return true;
  }

  visited.insert(check);
  if (check == cycle)
  {
    // Oops, we found the task about to be inserted; this would be a dependency cycle.
    return false;
  }

  for (const auto& dependency : check->dependencies())
  {
    if (!checkDependenciesRecursive(dependency.get(), cycle, visited))
    {
      // Oops, a child found the task about to be inserted; this would be a dependency cycle.
      return false;
    }
  }
  return true;
}

// Checks to see if a task or any of its descendants are present in a set of tasks.
// Returns true if neither the tasks nor its descendants are in the set.
bool checkDescendants(Task* task, const std::unordered_set<const Task*>& taskSet)
{
  // Is the task in the set?
  if (taskSet.find(task) != taskSet.end())
  {
    return false;
  }

  // Now check all of its children
  return (std::all_of(task->children().begin(), task->children().end(), [taskSet](Task* child) {
    return checkDescendants(child, taskSet);
  }));
}

} // anonymous namespace

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
  m_id = smtk::common::UUIDGenerator::instance().random();
}

Task::Task(
  const Configuration& config,
  Manager& taskManager,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  (void)managers;
  m_id = smtk::common::UUIDGenerator::instance().random();
  m_manager = taskManager.shared_from_this();
  this->configure(config);
}

Task::Task(
  const Configuration& config,
  const PassedDependencies& dependencies,
  Manager& taskManager,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  m_id = smtk::common::UUIDGenerator::instance().random();
  (void)managers;
  m_manager = taskManager.shared_from_this();
  this->configure(config);
  for (const auto& dependency : dependencies)
  {
    m_dependencies.insert(std::make_pair(
      (const std::weak_ptr<Task>)(dependency),
      dependency->observers().insert([this](Task& dependency, State prev, State next) {
        bool didChange = this->updateDependencyState(dependency, prev, next);
        (void)didChange;
      })));
  }
  m_dependencyState = this->computeDependencyState();
  // If we were told through the configuration info that the task was
  // completed - lets verify this based on the dependencies
  if (m_completed)
  {
    m_completed = (this->state() >= State::Completable);
  }
}

void Task::configure(const Configuration& config)
{
  if (!config.is_object())
  {
    throw std::logic_error("Invalid configuration passed to Task constructor.");
  }
  auto it = config.find("id");
  if (it == config.end() || it->is_number_integer())
  {
    // We are deserializing a task "template" which has no UUID. Make one up.
    this->setId(smtk::common::UUID::random());
  }
  else
  {
    this->setId(it->get<smtk::common::UUID>());
  }
  if (config.contains("name"))
  {
    this->setName(config.at("name").get<std::string>());
  }
  else if (config.contains("title"))
  {
    this->setName(config.at("title").get<std::string>());
  }
  if (config.contains("description"))
  {
    this->setDescription(config.at("description").get<std::string>());
  }
  if (config.contains("style"))
  {
    try
    {
      m_style = config.at("style").get<std::unordered_set<smtk::string::Token>>();
    }
    catch (nlohmann::json::exception&)
    {
    }
  }
  if (config.contains("strict-dependencies"))
  {
    m_strictDependencies = config.at("strict-dependencies").get<bool>();
  }
  if (config.contains("agentState"))
  {
    bool valid;
    m_agentState = stateEnum(config.at("agentState").get<std::string>(), &valid);
  }
  else if (config.contains("state")) // For Agent-less Tasks
  {
    bool valid;
    m_agentState = stateEnum(config.at("state").get<std::string>(), &valid);
  }
  if (config.contains("childrenState"))
  {
    bool valid;
    m_childrenState = stateEnum(config.at("childrenState").get<std::string>(), &valid);
  }
  if (config.contains("dependencyState"))
  {
    bool valid;
    m_dependencyState = stateEnum(config.at("dependencyState").get<std::string>(), &valid);
  }
  if (config.contains("completed"))
  {
    m_completed = config.at("completed").get<bool>();
  }
  auto& helper = smtk::task::json::Helper::instance();
  auto manager = m_manager.lock();
  if (config.contains("ports") && config.at("ports").is_object())
  {
    for (const auto& portEntry : config.at("ports").items())
    {
      const std::string& portName = portEntry.key();
      auto* port = helper.objectFromJSONSpecAs<smtk::task::Port>(portEntry.value(), "port"_token);
      if (port && !portName.empty())
      {
        port->setParent(this);
        m_ports[portName] = port;
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "Malformed port: key \"" << portName << "\" value " << portEntry.value().dump()
                                   << ". Skipping.");
      }
    }
  }
  if (config.contains("agents"))
  {
    for (const auto& agentConfig : config.at("agents"))
    {
      auto it = agentConfig.find("type");
      if (it != agentConfig.end())
      {
        auto agentTypeName = it->get<std::string>();
        auto agent = manager->agentFactory().createFromName(agentTypeName, this);
        if (agent)
        {
          try
          {
            // NB: We must emplace the agent before calling configure
            //     on it because most agents will call their parent task's
            //     updateAgentState() upon completion (which uses the
            //     task's agents to compute state).
            (*m_agents.emplace(std::move(agent)).first)->configure(agentConfig);
          }
          catch (std::exception& e)
          {
            smtkErrorMacro(
              smtk::io::Logger::instance(),
              "Could not configure agent: \"" << e.what() << "\"." << agentConfig.dump(2));
          }
        }
        else
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(), "Unable to construct agent \"" << agentTypeName << "\".");
        }
      }
    }
  }
}

bool Task::setId(const common::UUID& newId)
{
  if (newId == m_id)
  {
    return false;
  }
  if (auto rsrc = this->resource())
  {
    // TODO: FIXME: ask resource to update our ID and index us.
  }
  m_id = newId;
  return true;
}

void Task::setName(const std::string& name)
{
  if (name == m_name)
  {
    return;
  }
  if (auto rsrc = this->resource())
  {
    // TODO: FIXME: ask resource to update our name and index us.
  }
  m_name = name;
}

const std::shared_ptr<resource::Resource> Task::resource() const
{
  std::shared_ptr<resource::Resource> rsrc;
  if (auto manager = m_manager.lock())
  {
    if (auto* ptr = manager->resource())
    {
      rsrc = ptr->shared_from_this();
    }
  }
  return rsrc;
}

std::string Task::information(const InformationOptions& opts) const
{
  std::cout << "Rebuild tooltip " << this->name() << "\n";
  std::ostringstream result;
  if (opts.m_includeTitle)
  {
    result << "<h1>" << this->name() << "</h1>\n";
  }
  if (opts.m_includeDescription)
  {
    result << m_description;
  }

  if (opts.m_includeTroubleshooting)
  {
    std::string troubleshooting;
    for (const auto& agent : m_agents)
    {
      troubleshooting += agent->troubleshoot();
    }
    if (!troubleshooting.empty())
    {
      result << "<h2>Diagnostics</h2>\n<ul>\n" << troubleshooting << "\n</ul>";
    }
  }
  return result.str();
}

void Task::setDescription(const std::string& description)
{
  m_description = description;
}

const std::unordered_map<smtk::string::Token, Port*>& Task::ports() const
{
  return m_ports;
}

bool Task::changePortName(Port* port, const std::string& newName, std::function<bool()> fp)
{
  auto fp1 = [this, port, newName, fp]() {
    if (port->parent() != this)
    {
      return false; // Task does not own the port
    }
    // See if the new name is already being used
    auto it = m_ports.find(newName);
    if (it != m_ports.end())
    {
      return false; // new name is already used
    }
    // Ok - it's ok to change the port name
    m_ports.erase(it);
    m_ports[newName] = port;
    return fp();
  };

  if (auto manager = m_manager.lock())
  {
    return manager->changePortName(port, newName, fp1);
  }
  return fp1();
}

std::shared_ptr<PortData> Task::portData(const Port* port) const
{
  if (!port || port->parent() != this)
  {
    return nullptr;
  }

  if (port->direction() == Port::Direction::In)
  {
    return this->inputPortData(port);
  }
  return this->outputPortData(port);
}

void Task::portDataUpdated(const Port* port)
{
  if (!port)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Null port provided to Task::portDataUpdated()");
    return;
  }

  if (port->direction() == Port::Direction::In)
  {
    for (const auto& agent : m_agents)
    {
      agent->portDataUpdated(port);
    }
  }
  else // port->direction() == Port::Direction::Out
  {
    for (const auto& conn : port->connections())
    {
      if (auto* connPort = dynamic_cast<smtk::task::Port*>(conn))
      {
        if (auto* connParent = connPort->parent())
        {
          connParent->portDataUpdated(connPort);
        }
      }
    }
  }
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
  bool didAdd = false;
  for (const auto& agent : m_agents)
  {
    didAdd |= agent->getViewData(configuration);
  }
  return didAdd;
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

bool Task::canAddDependency(const std::shared_ptr<Task>& dependency)
{
  std::set<Task*> visited;
  if (!dependency)
  {
    return false;
  }
  // Are both the tasks involved children of the same parents?
  if (m_parent != dependency->m_parent)
  {
    return false;
  }
  bool ok = checkDependenciesRecursive(dependency.get(), this, visited);
  return ok;
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

  if (!this->canAddDependency(dependency))
  {

    // Ensure no dependency cycles exist.
    return false;
  }

  m_dependencies.insert(std::make_pair(
    (const std::weak_ptr<Task>)(dependency),
    dependency->observers().insert([this](Task& dependency, State prev, State next) {
      bool didChange = this->updateDependencyState(dependency, prev, next);
      (void)didChange;
    })));
  dependency->m_dependents.insert(std::dynamic_pointer_cast<Task>(this->shared_from_this()));
  this->updateDependencyState(*dependency, m_dependencyState, dependency->state());
  return true;
}

bool Task::removeDependency(const std::shared_ptr<Task>& dependency)
{
  State prev = this->state();
  bool didRemove = m_dependencies.erase(dependency) > 0;
  if (didRemove)
  {
    // Update the dependency state
    m_dependencyState = this->computeDependencyState();
    State next = this->state();
    if (prev != next)
    {
      this->changeState(prev, next);
    }
    return true;
  }
  return false;
}

State Task::state() const
{
  if (m_dependencyState == State::Unavailable)
  {
    return State::Unavailable;
  }

  if (
    ((m_childrenState > State::Unavailable) && (m_childrenState < State::Completed)) ||
    (m_agentState == State::Incomplete))
  {
    return State::Incomplete;
  }

  State internalState;

  if ((m_agentState == m_childrenState) || (m_agentState == State::Irrelevant))
  {
    internalState = m_childrenState;
  }
  else if (m_childrenState == State::Irrelevant)
  {
    internalState = m_agentState;
  }
  else if ((m_agentState == State::Completable) && (m_childrenState == State::Completed))
  {
    internalState = State::Completable;
  }
  else
  {
    // The states are not the same and at least one is less
    // than Completable
    return State::Incomplete;
  }
  // In the case where the children states are Completed and
  // there are no relevant agents, the task's internal state is
  // considered Completable instead of Completed
  if (internalState == State::Completed)
  {
    internalState = State::Completable;
  }
  if (internalState == State::Completable && m_completed)
  {
    return State::Completed;
  }
  return internalState;
}

std::unordered_set<const Task*> Task::ancestors() const
{
  std::unordered_set<const Task*> result;
  for (const Task* p = this; p != nullptr; p = p->m_parent)
  {
    result.insert(p);
  }
  return result;
}

std::vector<Task*> Task::lineage() const
{
  std::vector<Task*> result;
  for (Task* p = m_parent; p != nullptr; p = p->m_parent)
  {
    result.push_back(p);
  }
  std::reverse(result.begin(), result.end());
  return result;
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
      for (const auto& child : m_children)
      {
        if (child)
        {
          if (visitor(*child) == smtk::common::Visit::Halt)
          {
            return smtk::common::Visit::Halt;
          }
        }
      }
      break;
  }
  return status;
}

bool Task::canAddChild(const std::shared_ptr<Task>& child) const
{
  if (child->m_parent != nullptr)
  {
    // task is already a child to a task
    return false;
  }
  auto taskSet = this->ancestors();
  return checkDescendants(child.get(), taskSet);
}

bool Task::addChild(const std::shared_ptr<Task>& child)
{
  auto taskSet = this->ancestors();
  return this->addChild(child, taskSet);
}

bool Task::removeChild(const std::shared_ptr<Task>& child)
{
  if (child && (child->m_parent == this))
  {
    auto it = m_children.find(child.get());
    if (it != m_children.end())
    {
      m_children.erase(it);
      child->m_parent = nullptr;
      // Compute new task child state if needed
      this->updateChildrenState(child.get(), child->state(), State::Irrelevant);
      return true;
    }
  }
  return false;
}

std::unordered_set<Agent*> Task::agents() const
{
  std::unordered_set<Agent*> aset;
  for (const auto& agent : m_agents)
  {
    aset.insert(agent.get());
  }
  return aset;
}

bool Task::updateAgentState(const Agent* agent, State prev, State next, bool signal)
{
  (void)prev; // Not needed by the base class
  if ((m_agentState == next) || (agent->parent() != this))
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

std::shared_ptr<smtk::common::Managers> Task::managers() const
{
  if (auto* taskMgr = this->manager())
  {
    return taskMgr->managers();
  }
  return nullptr;
}

bool Task::changeState(State previous, State next)
{
  if (previous == next)
  {
    return false;
  }

  State statePriorToAgentNotification = next;
  // Notify the agents that the Task state has changed
  for (const auto& agent : m_agents)
  {
    agent->taskStateChanged(previous, next);
    // Any agent may alter "next".
    //
    // For example, SubmitOperationAgent will downgrade Completable to Incomplete
    // when a task transitions away from Completed (since now the operation must
    // re-run). If this occurs, we should not continue since the agent will call
    // Task::updateAgentState() which calles changeState() before returning here.
    if (next != statePriorToAgentNotification)
    {
      return false;
    }
  }

  m_completed = next == State::Completed;

  // Let the task's parent know about the change
  if (m_parent)
  {
    m_parent->updateChildrenState(this, previous, next);
  }
  m_observers(*this, previous, next);
  return true;
}

bool Task::addChild(
  const std::shared_ptr<Task>& child,
  const std::unordered_set<const Task*>& taskSet)
{
  // Is this child already parented?
  if (child->m_parent != nullptr)
  {
    return false;
  }
  if (child && checkDescendants(child.get(), taskSet))
  {
    m_children.insert(child.get());
    child->m_parent = this;
    // Compute new task child state if needed
    this->updateChildrenState(child.get(), State::Irrelevant, child->state());
    return true;
  }
  return false;
}

bool Task::updateDependencyState(Task& dependency, State prev, State next)
{
  (void)dependency; // the base class doesn't need it.

  // If the new state is the same as the current dependency state then
  // then the task state will not change
  if (next == m_dependencyState)
  {
    return false;
  }
  // If a dependent task becomes blocking or non-blocking,
  // check other tasks and see if we should change our state
  State limit = m_strictDependencies ? State::Completed : State::Completable;
  bool dependencyNowUnblocked = (prev < limit && next >= limit);
  bool dependencyNowBlocking = (((prev == State::Irrelevant) || (prev >= limit)) && next < limit);

  // No significant change to our dependency.
  if (!dependencyNowUnblocked && !dependencyNowBlocking)
  {
    return false;
  }

  if (dependencyNowUnblocked && dependencyNowBlocking)
  {
    throw std::logic_error("Impossible state.");
  }

  State newDepState = this->computeDependencyState();
  if (m_dependencyState == newDepState)
  {
    return false; // the dependency state hasn't changed
  }

  State currentTaskState = this->state();
  m_dependencyState = newDepState;
  State newTaskState = this->state();

  if (currentTaskState == newTaskState)
  {
    return false;
  }

  this->changeState(currentTaskState, newTaskState);
  return true;
}

bool Task::updateChildrenState(const Task* child, State prev, State next)
{
  (void)prev; // Not needed by the base class
  if ((m_childrenState == next) || (child->parent() != this))
  {
    return false;
  }

  State newChildrenState = this->computeChildrenState();
  if (m_childrenState == newChildrenState)
  {
    return false; // no change in children state
  }

  State currentTaskState = this->state();
  m_childrenState = newChildrenState;
  State newTaskState = this->state();

  if (currentTaskState == newTaskState)
  {
    return false;
  }

  this->changeState(currentTaskState, newTaskState);
  return true;
}

State Task::computeDependencyState() const
{
  State s = State::Irrelevant;
  for (const auto& dd : m_dependencies)
  {
    const auto& dependency(dd.first.lock());
    auto ds = dependency->state();
    if (ds == State::Irrelevant)
    {
      // We can skip irrelevant dependencies
      continue;
    }
    if (ds < State::Completable)
    {
      // If any dependency is not at least completable then
      // the computed state is unavailable
      return State::Unavailable;
    }
    if ((s == State::Irrelevant) || (s > ds))
    {
      // In this case ds is the current state of the
      // task's dependencies
      s = ds;
    }
  }
  if (m_strictDependencies && (s == State::Completable))
  {
    s = State::Unavailable;
  }

  return s;
}

State Task::computeAgentState() const
{
  State s = State::Irrelevant;
  for (const auto& agent : m_agents)
  {
    auto as = agent->state();
    if ((as == s) || (as == State::Irrelevant))
    {
      // agent matches current state or is irrelevant
      // so it can be skipped
      continue;
    }
    if (as == State::Incomplete)
    {
      // any incomplete agent will make the
      // agentState incomplete
      return State::Incomplete;
    }
    if (s == State::Irrelevant)
    {
      // a relevant agent will always result
      // in a non-irrelevant state
      s = as;
    }
    else
    {
      // If we are here then we know we
      // are trying to combine an unavailable
      // with a complete state which will
      // result in an incomplete agentState
      return State::Incomplete;
    }
  }
  return s;
}

State Task::computeChildrenState() const
{
  State s = State::Irrelevant;
  for (const auto* child : m_children)
  {
    auto cs = child->state();
    if ((cs == s) || (cs == State::Irrelevant))
    {
      // child matches current state or is irrelevant
      // so it can be skipped
      continue;
    }
    if ((s == State::Irrelevant) || (s > cs))
    {
      // In this case cs is the current state of the
      // task's children
      s = cs;
    }
  }
  return s;
}

std::shared_ptr<PortData> Task::outputPortData(const Port* port) const
{
  std::shared_ptr<PortData> result;
  // Find agents that are using the port and fetch their data.
  // If more than one agent is involved then merge the results.
  for (const auto& agent : m_agents)
  {
    auto pd = agent->portData(port);
    if (!pd)
    {
      continue;
    }
    if (result)
    {
      result->merge(pd.get());
    }
    else
    {
      result = pd;
    }
  }
  return result;
}

std::shared_ptr<PortData> Task::inputPortData(const Port* port) const
{
  std::shared_ptr<PortData> data;
  for (const auto& conn : port->connections())
  {
    // Ask the port to produce data for its connection:
    auto connData = port->portData(conn);
    if (connData)
    {
      if (data)
      {
        data->merge(connData.get());
      }
      else
      {
        data = connData;
      }
    }
  }
  return data;
}

} // namespace task
} // namespace smtk
