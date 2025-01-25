//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_Task_h
#define smtk_task_Task_h

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

#include "smtk/CoreExports.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"
#include "smtk/common/Deprecation.h"
#include "smtk/common/Managers.h"
#include "smtk/common/Observers.h"
#include "smtk/common/Visit.h"
#include "smtk/task/Agent.h"
#include "smtk/task/State.h"

#include "nlohmann/json.hpp"

#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>

namespace smtk
{
namespace task
{

class Agent;
class Manager;
class Port;
class PortData;
class Task;
/// Free functions that populate a set of workflow "head" tasks for an input \a task
/// (this is the set of tasks that \a task ultimately depends on but are not themselves
/// dependent on any task).
///
/// The variant that accepts \a visited can be used to efficiently accumulate workflows
/// across several tasks (by pruning visited nodes from its traversal).
SMTKCORE_EXPORT void workflowsOfTask(
  Task* task,
  std::set<smtk::task::Task*>& workflows,
  std::set<smtk::task::Task*>& visited);
SMTKCORE_EXPORT std::set<smtk::task::Task*> workflowsOfTask(const Task& task);

/**\brief Task is a base class for all SMTK tasks.
  *
  * SMTK tasks are nodes in a graph whose arcs connect them to dependencies.
  * The state of each task and its dependencies determine the set of available
  * (or reachable) tasks.
  * An application's user interface typically gets reconfigured when the active
  * task changes to help direct users through a workflow, although tasks should
  * not attempt to modify the user interface themselves. Instead, tasks should
  * be solely focused on determining whether conditions are met for them to be
  * (a) accessible to users and (b) marked completed by users.
  * Once the first set of conditions is met, users are allowed to undertake the
  * task. Once the second set of conditions is met, users are allowed to mark
  * the task complete and thus gain access to those tasks which depend on it.
  */
class SMTKCORE_EXPORT Task : public smtk::resource::Component
{
public:
  smtkTypeMacro(smtk::task::Task);
  smtkSuperclassMacro(smtk::resource::Component);
  smtkCreateMacro(smtk::resource::PersistentObject);

  /// A task's state changes may be observed.
  using Observer = std::function<void(Task&, State, State)>;
  /// The collection of all observers of this task instance.
  using Observers = smtk::common::Observers<Observer>;
  /// A task's dependencies are other tasks stored as weak pointers.
  using Dependencies = std::map<WeakPtr, typename Observers::Key, std::owner_less<WeakPtr>>;
  /// A task's dependencies are other tasks passed as shared pointers.
  using PassedDependencies = std::set<Ptr>;
  /// Tasks are configured with arbitrary JSON objects, though this may change.
  using Configuration = nlohmann::json;
  /// Signature of functions invoked when visiting dependencies or children while
  /// allowing early termination.
  using Visitor = std::function<smtk::common::Visit(Task&)>;

  /// The types of related tasks to visit.
  enum class RelatedTasks
  {
    Depend, //!< Visit tasks this task depends upon.
    Child   //!< Visit child tasks.
  };

  /// Options for the information() method.
  struct InformationOptions
  {
    InformationOptions() {}
    bool m_includeTitle{ true };
    bool m_includeDescription{ true };
    bool m_includeTroubleshooting{ true };
  };

  Task();
  Task(
    const Configuration& config,
    Manager& taskManager,
    const std::shared_ptr<smtk::common::Managers>& managers = nullptr);
  Task(
    const Configuration& config,
    const PassedDependencies& dependencies,
    Manager& taskManager,
    const std::shared_ptr<smtk::common::Managers>& managers = nullptr);

  ~Task() override = default;

  /// A method called by all constructors passed Configuration information.
  ///
  /// In general, this method should set member variables directly
  /// instead of calling set/get methods since those may invoke observers
  /// with an uninitialized object.
  ///
  /// Depedendencies (if they were provided) will already have been added
  /// when this method is called.
  void configure(const Configuration& config);

  /// Set/get the task's unique identifier.
  const common::UUID& id() const override { return m_id; }
  bool setId(const common::UUID& newId) override;

  /// Return the task's name
  std::string name() const override { return m_name; }
  virtual void setName(const std::string& name);

  const std::shared_ptr<resource::Resource> resource() const override;

  /// Return the title of the task (if one was provided).
  SMTK_DEPRECATED_IN_24_01("Use name() instead.")
  const std::string& title() const { return m_name; }

  /// Set the title of the task to be presented to users.
  /// This is not intended to be a unique identifier.
  SMTK_DEPRECATED_IN_24_01("Use setName() instead.")
  void setTitle(const std::string& title) { this->setName(title); }

  /// Return user-presentable information about the task in XHTML form.
  ///
  /// This is the concatenation of a description (see setDescription())
  /// along with tips from each configured agent that is incomplete
  /// or unavailable describing actions users may need to address to
  /// make the task completable.
  virtual std::string information(const InformationOptions& opts = InformationOptions()) const;

  /// Set/get a description of the task provided by workflow designers.
  ///
  /// The description should be in XHTML form but not include <html> or
  /// <body> elements as those are added by the information() method.
  void setDescription(const std::string& description);
  const std::string& description() const { return m_description; }
  std::string& description() { return m_description; }

  /// Return a set of ports for this task indexed by their function (a descriptive string).
  virtual const std::unordered_map<smtk::string::Token, Port*>& ports() const;

  /// Change the Name of a Port
  ///
  /// This method is used to update internal data structures. Please use task::Port::setName
  /// to change the name of the Port - it will call this method with an appropriate lambda.
  bool changePortName(Port* port, const std::string& newName, std::function<bool()> fp);

  /// Given a port owned by this task, return data to be transmitted over the port.
  ///
  /// If the \a port is an output port, the task's agents are each given an
  /// opportunity to produce PortData in turn.
  /// It the \a port is an input port, the port's connections are queried for
  /// port data.
  virtual std::shared_ptr<PortData> portData(const Port* port) const;

  /// Accept notification that data on the given \a port has been updated.
  ///
  /// This function will be called when processing operation results.
  /// There is no guarantee that resources other than the project owning
  /// this task is locked; if you need to modify other resources as a
  /// result of this notification, you should launch an operation so that
  /// those resources are properly locked.
  ///
  /// If \a port is one of the task's input ports (i.e., \a port->direction() == In),
  /// then each agent of the task is called with the port.
  /// If \a port is one of the task's output ports (i.e., \a port->direction() == Out),
  /// then each downstream task of \a port has its portDataUpdated() method with
  /// the connected port.
  ///
  /// Because this function will be called on the user-interface thread,
  /// it is acceptable to take actions affecting the user interface,
  /// including changing the state of this task.
  virtual void portDataUpdated(const Port* port);

  /// Set/get style classes for the task.
  /// A style class specifies how applications should present the task
  /// (e.g., what type of view to provide the user, what rendering mode
  /// to use, what objects to list or exclude).
  const std::unordered_set<smtk::string::Token>& style() const { return m_style; }
  bool addStyle(const smtk::string::Token& styleClass);
  bool removeStyle(const smtk::string::Token& styleClass);
  bool clearStyle();

#if 0
  /// For a given task and UI element, return a view configuration as
  /// dictated by the task's style.
  ///
  /// If multiple styles apply to the same UI element, this method is
  /// responsible for harmonizing them.
  /// If a UI element is not referenced by the task's style, this method
  /// will return null.
  std::shared_ptr<smtk::view::Configuration> findViewFor(
    smtk::string::Token uiElement) const { return nullptr; }
#endif

  /// Populate a type-container with view-related data for configuration.
  ///
  /// Internally, this method will iterate over agents, giving each
  /// a chance to insert view-related data.
  ///
  /// Generally, views will want access to a resource and potentially
  /// components in the resource that are the subject of the view.
  /// Other view configuration will come from view style() (see above)
  /// or smtk::common::Managers.
  ///
  /// This method will return true when the \a configuration
  /// was modified and false otherwise.
  virtual bool getViewData(smtk::common::TypeContainer& configuration) const;

  /// Get the state.
  ///
  /// This state is a composition of \a m_agentState (he combined state
  /// of all of the task's agents), \a m_childrenState (the combined state of all
  /// the task's children) – and \a m_dependencyState (the state of any dependencies).
  ///
  /// Since the task as no agents by default, \a m_agentState is initialized to
  /// State::Completable. Similarly, since the task has no children by default,
  /// m_childrenState is initialized to State::Irrelevant.  Therefore the default
  /// behavior is to become Completable once all dependencies are met
  /// (i.e., Completable or Completed) and Completed only if dependencies are met
  /// **and** markCompleted(true) has been called.
  ///
  /// In terms of expected values, \a m_dependnecyState can have the following values:
  /// - Irrelevant : There are no dependencies or all of their states are Irrelevant.
  /// - Unavailable : At least one of the relevant dependencies' states is less than Completable or
  ///   all are Completable but strict dependency observance has been requested.
  /// - Completable : All of the relevant dependencies' states are at least Completable but all
  ///   are not Completed and strict dependency observance has not been requested.
  /// - Completed : All of the relevant dependencies' states are Completed.
  ///
  /// In terms of expected values, \a m_agentState can have the following values:
  /// - Irrelevant : There are no agents or all of their states are Irrelevant
  /// - Unavailable : All relevant agents are Unavailable.
  /// - Completable : All relevant agents are Completable.
  /// - Incomplete : The states of the agents are different and .
  ///
  /// In terms of expected values, \a m_childrenState can have the following values:
  /// - Irrelevant : There are no children or all of their states are Irrelevant
  /// - Unavailable : The minimum state of all relevant children is Unavailable.
  /// - Incomplete : The minimum state of all relevant children is Incomplete.
  /// - Completable : The minimum state of all relevant children is Completable.
  /// - Completed : All relevant children are Completed.
  ///
  /// In terms of calculating the state of the task itself :
  /// <table>
  /// <tr><th>DependencyState <th>AgentState <th>ChildrenState <th>TaskState
  /// <tr><td>Unavailable <td>Any <td>Any <td>Unavailable
  /// <tr><td rowspan="18">
  ///       <br> Irrelevant
  ///       <br> Completable
  ///       <br> Completed
  ///     <td rowspan="4"> Irrelevant
  ///     <td>Irrelevant <td>Irrelevant
  /// <tr><td>Incomplete <td rowspan="2">Incomplete
  /// <tr><td>Completable
  /// <tr><td>Completed <td>
  ///                      <br> Completable (if m_complete is false)
  ///                      <br> Completed (if m_complete is true)
  /// <tr><td>Unavailable <td rowspan="3">Irrelevant <td>Unavailable
  /// <tr><td>Incomplete <td>Incomplete
  /// <tr><td>Completable <td>
  ///                       <br> Completable (if m_complete is false)
  ///                       <br> Completed (if m_complete is true)
  /// <tr><td>Unavailable <td>Unavailable <td> Unavailable
  /// <tr><td>Unavailable <td>\> Unavailable <td rowspan="5">Incomplete
  /// <tr><td>\> Unavailable <td>Unavailable
  /// <tr><td>Incomplete <td>Any
  /// <tr><td>Any <td>Incomplete
  /// <tr><td rowspan="2">Completable <td>Completable
  /// <tr><td>Completed <td>
  ///                      <br> Completable (if m_complete is false)
  ///                      <br> Completed (if m_complete is true)
  /// </table>

  virtual State state() const;

  /// Return whether or not the task has been marked completed.
  bool isCompleted() const { return m_completed; }

  /// Returns the state of a task's agents.
  State agentState() const { return m_agentState; }

  /// Returns the state of a task's children.
  State childrenState() const { return m_childrenState; }

  /// Returns the state of a task's dependencies.
  State dependencyState() const { return m_dependencyState; }

  /// Return whether or not users are allowed to mark a task completed.
  ///
  /// User interfaces (e.g., qtDiagramView) should check this to decide whether
  /// to enable UI elements that accept user completion.
  /// Subclasses of Task that automate completion (e.g., SubmitOperation
  /// when RunStyle is set to Once) can override this method to prevent users
  /// from explicitly marking completion. The default implementation returns
  /// true when the task state is completable or completed.
  virtual bool editableCompletion() const;

  /// This public method allows user-interface components to indicate
  /// when the user marks a task complete (or unmarks it).
  ///
  /// This method has no effect and returns false if the task's current state
  /// is unavailable or incomplete.
  /// Returns true if the task's current state changes (from Completable to Completed
  /// if \a completed is true or Completed to Completable if \a completed is false).
  /// If true is returned, this method invoked its observers.
  ///
  /// Be aware that if this task transitions from State::Completed
  /// to State::Incomplete or State::Unavailable, `m_completed`
  /// will be reset to false and this method must be invoked again.
  virtual bool markCompleted(bool completed);

  /// Return true if this task has been configured with strict
  /// dependency enforcement.
  ///
  /// When true, a task will (1) not become available until all its
  /// dependencies are marked completed and (2) not be completable
  /// until all its dependencies are marked completed.
  ///
  /// When false, a task will become available once its internal
  /// state becomes incomplete or completable, regardless of its
  /// dependent tasks. However, it will not be allowed to be marked
  /// complete until its dependencies are marked complete.
  bool areDependenciesStrict() const { return m_strictDependencies; }
  /// Return the tasks which this task depends upon.
  /// WARNING: The returned set is read-only (modifying it does not modify
  /// this Task); however, modifying tasks in the returned set can affect
  /// this Task's state.
  PassedDependencies dependencies() const;

  /// Check whether inserting a dependency would induce a cycle.
  ///
  /// Note that this method does not check adaptors as it is possible
  /// that they may have cycles and yet terminate. (It is also possible
  /// adaptors have cycles that would not terminate.)
  ///
  /// This method is invoked by \a addDependency().
  bool canAddDependency(const std::shared_ptr<Task>& dependency);

  /// Add a dependency.
  ///
  /// Returns true if the \a dependency was added, false if it already existed or is null.
  /// This method will invoke observers if adding the dependency changes this task's state.
  bool addDependency(const std::shared_ptr<Task>& dependency);

  /// Add a container of task-pointers as dependencies.
  ///
  /// Returns true if all \a tasks were added, false if any already existed or were null.
  /// This method will invoke observers if adding the dependencies changes this task's state.
  template<typename Container>
  bool addDependencies(const Container& tasks);

  /// Remove a dependency.
  ///
  /// Returns true if the \a dependency was removed, false if not.
  /// This method will invoke observers if removing the dependency changes this task's state.
  bool removeDependency(const std::shared_ptr<Task>& dependency);

  /// Remove a container of task-pointers as dependencies.
  ///
  /// Returns true if all \a tasks were removed, false otherwise.
  /// This method will invoke observers if removing the dependencies changes this task's state.
  template<typename Container>
  bool removeDependencies(const Container& tasks);

  /// Return a parent task if one exists; null otherwise.
  Task* parent() const { return m_parent; }

  /// Returns the ancestral tasks related to this one.
  ///
  /// Note that this will include the task itself.
  std::unordered_set<const Task*> ancestors() const;

  /// Returns the lineage of this task.
  ///
  /// The resulting vector will start with the most ancestral task and end
  /// with this task's parent.
  std::vector<Task*> lineage() const;

  /// Visit children. If hasChildren returns false, this will return immediately.
  ///
  /// For the signature taking a Visitor, this method returns
  /// smtk::common::Visit::Halt if iteration was terminated.
  ///
  /// Subclasses that override hasChildren should override these methods.
  virtual smtk::common::Visit visit(RelatedTasks relation, Visitor visitor) const;

  /// Return whether or not the task has children.
  virtual bool hasChildren() const { return !m_children.empty(); }

  /// Check whether adding a child task would induce a cycle.
  bool canAddChild(const std::shared_ptr<Task>& child) const;

  /// Add a child task.
  ///
  /// Returns true if the \a child was added, false if it already existed, is null
  /// or would result in a cycle.
  bool addChild(const std::shared_ptr<Task>& child);

  /// Add a container of task-pointers as children.
  ///
  /// Returns true if all \a children were added, false if any already existed, were null,
  /// or would result in a cycle.
  template<typename Container>
  bool addChildren(const Container& tasks);

  /// Remove a child task.
  ///
  /// Returns true if the \a child was removed, false if not.
  bool removeChild(const std::shared_ptr<Task>& child);

  /// Remove a container of task-pointers as children.
  ///
  /// Returns true if all \a task were removed, false otherwise.
  template<typename Container>
  bool removeChildren(const Container& tasks);

  /// Return the children of the task
  const std::unordered_set<Task*>& children() const { return m_children; }

  /// Return the agents of the task
  ///
  /// Note that the agents returned are owned by the task and should not be
  /// deleted.
  std::unordered_set<Agent*> agents() const;

  /// Return this object's observers so other classes may insert themselves.
  Observers& observers() { return m_observers; }

  ///\brief Updates the state of the task based on the change of an agent.
  ///
  /// Will return true if the task's state has changed.
  /// If this results in the task's state changing, observers are invoked
  /// if \a signal is true (the default).
  ///
  /// The \a signal parameter exists so that agents that wish to change
  /// their parent task's m_agentState without forcing the final state of
  /// the task to change may do so. This can happen, for instance, when
  /// a dependency causes the task's final state to become Unavailable
  /// but a task's state to transition from Completable to Incomplete.
  /// Since Incomplete > Unavailable, the overall task state should not
  /// be modified by the agent. However, the task's agent-state also
  /// needs to be modified from within Task::changeState.
  ///
  /// If you are writing an agent and \a next is less than your agent's
  /// internal state, pass false to \a signal.
  virtual bool updateAgentState(const Agent* agent, State prev, State next, bool signal = true);

  /// Return the tasks's manager (or null if unmanaged).
  Manager* manager() const { return m_manager.lock().get(); }

  /// Return the application-state container.
  ///
  /// This will return null if the task has no task::Manager.
  std::shared_ptr<smtk::common::Managers> managers() const;

protected:
  friend SMTKCORE_EXPORT void
  workflowsOfTask(Task*, std::set<smtk::task::Task*>&, std::set<smtk::task::Task*>&);

  /// Indicate the state of this task has changed.
  ///
  /// This method invokes observers if and only if \a previous and \a next are different.
  /// It will also update m_completed to match the new state.
  ///
  /// Note that this method invokes Agent::taskStateChanged() for each of the
  /// task's agents and any agent may interrupt the state change inside this method.
  /// If this occurs, the agent is responsible for calling Task::updateAgentState(),
  /// which in turn will call Task::changeState() with a different \a next value.
  /// This nested call will fire observers once the final state allowed by agents
  /// is determined. This means that not all calls to Task::changeState() will result
  /// in observers being fired, even when the state is changing.
  ///
  /// It returns false if \a previous == \a next or if an agent disallowed
  /// the state change to \a next (in which case observers from a nested call to
  /// changeState will have already succeeded) and true otherwise.
  bool changeState(State previous, State next);

  /// Adds a child to a task.
  ///
  /// /a taskSet contains this task and all of its ancestors.  This method allows reuse of
  /// the ancestral information when adding several children at once.
  bool addChild(const std::shared_ptr<Task>& child, const std::unordered_set<const Task*>& taskSet);

  /// Update our state because a dependent task has changed state or
  /// a subclass has marked the internal state as changed.
  ///
  /// Returns true if this task's state changed and false otherwise.
  /// This method will invoke observers if it returns true.
  virtual bool updateDependencyState(Task& dependency, State prev, State next);

  ///\brief Updates the state of the task based on the change of a child task.
  ///
  /// Will return true if the task's state has changed. If this results in the
  // task's state changing, observers will have been invoked.
  virtual bool updateChildrenState(const Task* child, State prev, State next);

  ///\brief Compute the state based on the state of the task's dependencies
  ///
  /// Each task instance may be configured with a "dependency strictness" that determines when
  /// users may work on a task with incomplete dependencies and whether users are allowed to mark
  /// a task with incomplete dependencies as completed.
  ///
  /// A subclass that wishes to autocomplete might invoke the base-class method although this could
  /// frustrate users.
  virtual State computeDependencyState() const;
  /// Compute the state based on state of the task's agents
  virtual State computeAgentState() const;
  /// Compute the state based on the state of the task's children
  virtual State computeChildrenState() const;

  /// Produce port data for an output port of this task by querying its agents.
  std::shared_ptr<PortData> outputPortData(const Port* port) const;
  /// Produce port data for an input port of this task by querying its connections.
  std::shared_ptr<PortData> inputPortData(const Port* port) const;

  /// A task name to present to the user.
  std::string m_name;
  /// A description of the task provided by a workflow designer.
  std::string m_description;

  /// The set of style classes for this task.
  std::unordered_set<smtk::string::Token> m_style;
  /// Whether the user has marked the task completed or not.
  bool m_completed = false;
  /// A set of dependent tasks and the keys used to observe their
  /// state so that this task can update its state in response.
  std::map<WeakPtr, Observers::Key, std::owner_less<WeakPtr>> m_dependencies;
  /// Should dependencies be strictly enforced?
  bool m_strictDependencies = false;
  /// Tasks upon which this task depends.
  ///
  /// This set is maintained by other Task instances when
  /// addDependency/removeDependency is called.
  std::set<WeakPtr, std::owner_less<WeakPtr>> m_dependents;
  /// The set of observers of *this* task's state.
  Observers m_observers;
  /// If  this task is the child of another task, this pointer references its parent.
  /// The parent is responsible for updating this pointer as needed.
  /// m_parent is not a weak pointer because it must be initialized in the child
  /// during the parent's construction (when no shared pointer may exist).
  Task* m_parent = nullptr;
  /// If this task is being managed, this will refer to its manager.
  std::weak_ptr<smtk::task::Manager> m_manager;
  /// The unique identifier for this task.
  smtk::common::UUID m_id;
  /// The children of the task
  std::unordered_set<Task*> m_children;
  /// The agents of the task
  std::unordered_set<std::unique_ptr<Agent>> m_agents;
  //std::unordered_set<Agent*> m_agents;
  /// The ports of the Task
  std::unordered_map<smtk::string::Token, Port*> m_ports;

  State m_agentState = State::Completable;
  State m_dependencyState = State::Irrelevant;
  State m_childrenState = State::Irrelevant;

private:
};

template<typename Container>
bool Task::addDependencies(const Container& tasks)
{
  bool addedAll = true;
  for (const auto& task : tasks)
  {
    addedAll &= this->addDependency(task);
  }
  return addedAll;
}

template<typename Container>
bool Task::removeDependencies(const Container& tasks)
{
  bool removedAll = true;
  for (const auto& task : tasks)
  {
    removedAll &= this->removeDependency(task);
  }
  return removedAll;
}

template<typename Container>
bool Task::addChildren(const Container& tasks)
{
  auto taskSet = this->ancestors();
  bool addedAll = true;
  for (const auto& task : tasks)
  {
    addedAll &= this->addChild(task, taskSet);
  }
  return addedAll;
}

template<typename Container>
bool Task::removeChildren(const Container& tasks)
{
  bool removedAll = true;
  for (const auto& task : tasks)
  {
    removedAll &= this->removeChild(task);
  }
  return removedAll;
}

} // namespace task
} // namespace smtk

#endif // smtk_task_Task_h
