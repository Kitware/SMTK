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

#include "smtk/CoreExports.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"
#include "smtk/common/Managers.h"
#include "smtk/common/Observers.h"
#include "smtk/common/Visit.h"
#include "smtk/task/State.h"

#include "nlohmann/json.hpp"

#include <map>
#include <memory>
#include <string>
#include <utility>

namespace smtk
{
namespace task
{

class Manager;
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
  *
  * Subclasses of Task are responsible for monitoring the application's state;
  * the base class provides no methods to aid in this other than access to the
  * application's managers at construction.
  * Once the subclass has determined conditions are met, it should call
  * the `internalStateChanged()` method.
  * The base class will determine if this results in a state transition or not
  * (based on dependencies blocking the change) and notify observers as needed.
  */
class SMTKCORE_EXPORT Task : smtkEnableSharedPtr(Task)
{
public:
  smtkTypeMacroBase(smtk::task::Task);
  smtkCreateMacro(smtk::task::Task);

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

  Task();
  Task(
    const Configuration& config,
    const std::shared_ptr<smtk::common::Managers>& managers = nullptr);
  Task(
    const Configuration& config,
    const PassedDependencies& dependencies,
    const std::shared_ptr<smtk::common::Managers>& managers = nullptr);

  virtual ~Task() = default;

  /// A method called by all constructors passed Configuration information.
  ///
  /// In general, this method should set member variables directly
  /// instead of calling set/get methods since those may invoke observers
  /// with an uninitialized object.
  ///
  /// Depedendencies (if they were provided) will already have been added
  /// when this method is called.
  void configure(const Configuration& config);

  /// Return the title of the task (if one was provided).
  const std::string& title() const { return m_title; }

  /// Set the title of the task to be presented to users.
  /// This is not intended to be a unique identifier.
  void setTitle(const std::string& title);

  /// Set/get style classes for the task.
  /// A style class specifies how applications should present the task
  /// (e.g., what type of view to provide the user, what rendering mode
  /// to use, what objects to list or exclude).
  const std::set<std::string>& style() const { return m_style; }
  bool addStyle(const std::string& styleClass);
  bool removeStyle(const std::string& styleClass);
  bool clearStyle();

  /// Populate a type-container with view-related data for configuration.
  ///
  /// Subclasses should override this method.
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
  /// This state is a composition of \a m_internalState – updated by subclasses as
  /// needed – and the state of any dependencies.
  /// Because \a m_internalState is initialized to State::Completable,
  /// the default behavior is to become Completable once all dependencies are met
  /// (i.e., Completable or Completed) and Completed only if dependencies are met
  /// **and** markCompleted(true) has been called.
  /// Thus, the implementation in this base class will never return Incomplete.
  /// If a subclass marks the internal state as Incomplete, then this method may
  /// return Incomplete.
  ///
  /// <table>
  /// <caption>State "truth table" given internal and dependency states</caption>
  /// <tr><th>Dependencies/<br>Internal</th><th>Unavailable</th><th>Incomplete</th> <th>Completable</th><th>Completed</th></tr>
  /// <tr><td colspan="5">User has not marked task completed</td></tr>
  /// <tr><th>Irrelevant</th>               <td>Irrelevant</td> <td>Irrelevant</td> <td>Irrelevant</td> <td>Irrelevant</td></tr>
  /// <tr><th>Unavailable</th>              <td>Unavailable</td><td>Unavailable</td><td>Unavailable</td><td>Unavailable</td></tr>
  /// <tr><th>Incomplete</th>               <td>Unavailable</td> <td>Incomplete</td><td>Incomplete</td> <td>Incomplete</td></tr>
  /// <tr><th>Completable</th>              <td>Unavailable</td> <td>Incomplete</td><td>Completable</td><td>Completable</td></tr>
  /// <tr><td colspan="5">User has marked task completed</td></tr>
  /// <tr><th>Irrelevant</th>               <td>Irrelevant</td> <td>Irrelevant</td> <td>Irrelevant</td> <td>Irrelevant</td></tr>
  /// <tr><th>Unavailable</th>              <td>Unavailable</td><td>Unavailable</td><td>Unavailable</td><td>Unavailable</td></tr>
  /// <tr><th>Incomplete</th>               <td>Unavailable</td> <td>Incomplete</td><td>Incomplete</td> <td>Incomplete</td></tr>
  /// <tr><th>Completable</th>              <td>Unavailable</td> <td>Incomplete</td><td>Completable</td><td>Completed</td></tr>
  /// </table>
  ///
  /// Note that the internal state does not include State::Completed; only the user may mark a task
  /// completed and the base class implements a method to handle user input.
  /// A subclass that wishes to autocomplete might invoke the base-class method although this could
  /// frustrate users.
  virtual State state() const;

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
  bool markCompleted(bool completed);

  /// Return the tasks which this task depends upon.
  /// WARNING: The returned set is read-only (modifying it does not modify
  /// this Task); however, modifying tasks in the returned set can affect
  /// this Task's state.
  PassedDependencies dependencies() const;

  /// Add a dependency.
  ///
  /// Returns true if the \a dependency was added, false if it already existed or is null.
  /// This method will invoke observers if adding the dependency changes this task's state.
  bool addDependency(const std::shared_ptr<Task>& dependency);

  /// Add a container of task-pointers as dependencies.
  ///
  /// Returns true if all \a dependencies were added, false if any already existed or were null.
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
  /// Returns true if all \a dependencies were removed, false otherwise.
  /// This method will invoke observers if removing the dependencies changes this task's state.
  template<typename Container>
  bool removeDependencies(const Container& tasks);

  /// Return a parent task if one exists; null otherwise.
  Task* parent() const { return m_parent; }

  /// Return whether or not the task has children.
  /// By default, tasks do not support children.
  virtual bool hasChildren() const { return false; }

  /// Visit children. If hasChildren returns false, this will return immediately.
  ///
  /// For the signature taking a Visitor, this method returns
  /// smtk::common::Visit::Halt if iteration was terminated.
  ///
  /// Subclasses that override hasChildren should override these methods.
  virtual smtk::common::Visit visit(RelatedTasks relation, Visitor visitor) const;

  /// Return this object's observers so other classes may insert themselves.
  Observers& observers() { return m_observers; }

  /// Return the internal state of the task.
  ///
  /// This should not generally be used; instead, call `state()`.
  /// This state does not include dependencies or user-completion;
  /// it only reports whether the application has met conditions
  /// inherent for the task itself.
  State internalState() const { return m_internalState; }

protected:
  friend SMTKCORE_EXPORT void
  workflowsOfTask(Task*, std::set<smtk::task::Task*>&, std::set<smtk::task::Task*>&);

  /// Indicate the state has changed.
  /// This method invokes observers if and only if \a previous and \a next are different.
  /// It will also update m_completed to match the new state.
  ///
  /// It returns false if \a previous == \a next and true otherwise.
  bool changeState(State previous, State next);

  /// Update our state because a dependent task has changed state or
  /// a subclass has marked the internal state as changed.
  ///
  /// Returns true if this task's state changed and false otherwise.
  /// This method will invoke observers if it returns true.
  virtual bool updateState(Task& dependency, State prev, State next);

  /// Update the internal state, possibly transitioning the task's final state.
  ///
  /// This method returns true if the task's final state changed and
  /// false otherwise.
  /// If true is returned, observers have been invoked.
  bool internalStateChanged(State next);

  /// A task name to present to the user.
  std::string m_title;
  /// The set of style classes for this task.
  std::set<std::string> m_style;
  /// Whether the user has marked the task completed or not.
  bool m_completed = false;
  /// A set of dependent tasks and the keys used to observe their
  /// state so that this task can update its state in response.
  std::map<WeakPtr, Observers::Key, std::owner_less<WeakPtr>> m_dependencies;
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

private:
  /// The internal state of the task as provided by subclasses.
  /// This is private so subclasses cannot alter it directly;
  /// instead they should invoke `internalStateChanged()` so that
  /// the Task's final state can be updated and observers invoked.
  State m_internalState = State::Completable;
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

} // namespace task
} // namespace smtk

#endif // smtk_task_Task_h
