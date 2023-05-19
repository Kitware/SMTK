//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"
#include "smtk/common/Managers.h"
#include "smtk/model/Registrar.h"
#include "smtk/model/Resource.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/plugin/Registry.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Registrar.h"
#include "smtk/task/GatherResources.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Registrar.h"
#include "smtk/task/Task.h"

#include "smtk/common/testing/cxx/helpers.h"

namespace test_task
{

using namespace smtk::task;

static bool taskDestroyed = false;

class UnavailableTask : public Task
{
public:
  smtkTypeMacro(test_task::UnavailableTask);
  smtkCreateMacro(smtk::task::Task);
  UnavailableTask() = default;
  UnavailableTask(
    const Configuration& config,
    Manager& taskManager,
    const smtk::common::Managers::Ptr& managers = nullptr)
    : Task(config, taskManager, managers)
  {
    this->internalStateChanged(State::Unavailable);
  }
  UnavailableTask(
    const Configuration& config,
    const Task::PassedDependencies& deps,
    Manager& taskManager,
    const smtk::common::Managers::Ptr& managers = nullptr)
    : Task(config, deps, taskManager, managers)
  {
    this->internalStateChanged(State::Unavailable);
  }
  ~UnavailableTask() override
  {
    std::cout << "Destroying " << this->name() << "\n";
    taskDestroyed = true;
  }
};

} // namespace test_task

int TestTaskBasics(int, char*[])
{
  using smtk::task::State;
  using smtk::task::Task;
  using test_task::UnavailableTask;

  // Create managers
  auto managers = smtk::common::Managers::create();
  auto attributeRegistry = smtk::plugin::addToManagers<smtk::attribute::Registrar>(managers);
  auto resourceRegistry = smtk::plugin::addToManagers<smtk::resource::Registrar>(managers);
  auto operationRegistry = smtk::plugin::addToManagers<smtk::operation::Registrar>(managers);
  auto taskRegistry = smtk::plugin::addToManagers<smtk::task::Registrar>(managers);

  auto resourceManager = managers->get<smtk::resource::Manager::Ptr>();
  auto operationManager = managers->get<smtk::operation::Manager::Ptr>();
  auto taskManager = smtk::task::Manager::create();

  auto attributeResourceRegistry =
    smtk::plugin::addToManagers<smtk::attribute::Registrar>(resourceManager);
  auto modelRegistry = smtk::plugin::addToManagers<smtk::model::Registrar>(resourceManager);
  auto taskTaskRegistry = smtk::plugin::addToManagers<smtk::task::Registrar>(taskManager);

  taskManager->taskInstances().registerType<test_task::UnavailableTask>();

  auto ikey = taskManager->taskInstances().observers().insert(
    [](smtk::common::InstanceEvent event, const smtk::task::Task::Ptr& task) {
      std::cout << (event == smtk::common::InstanceEvent::Managed ? "Manage" : "Unmanage") << " "
                << task->name() << "\n";
    });

  int count = 0;
  auto countInstances = [&count](const std::shared_ptr<smtk::task::Task>& task) {
    ++count;
    std::cout << "  " << task->name() << " " << task->state() << "\n";
    return smtk::common::Visit::Continue;
  };

  {
    std::shared_ptr<Task> t1 = taskManager->taskInstances().create<Task>(
      Task::Configuration{ { "name", "Task 1" } }, *taskManager, managers);
    test(!!t1, "Expecting to create a non-null task.");
    test(
      t1->state() == State::Completable, "Expected task without dependencies to be completable.");

    State from;
    State to;
    int called = 0;
    auto callback = [&from, &to, &called](Task& task, State prev, State next) {
      (void)task;
      ++called;
      from = prev;
      to = next;
      std::cout << "  " << task.name() << " transitioned: " << prev << " ⟶ " << next << "\n";
    };
    auto okey = t1->observers().insert(callback);

    bool success = t1->markCompleted(true);
    test(success, "Expected completion to be accepted.");
    test(called == 1, "Expected observer to be invoked.");
    test(
      from == State::Completable && to == State::Completed,
      "Expected state transition completable⟶completed.");

    called = 0;
    success = t1->markCompleted(true);
    test(!success, "Expected completion to be rejected on double-complete.");
    test(called == 0, "Expected observer to be skipped on double-complete.");

    called = 0;
    success = t1->markCompleted(false);
    test(success, "Expected uncompletion to be accepted.");
    test(called == 1, "Expected observer to be invoked.");
    test(
      from == State::Completed && to == State::Completable,
      "Expected state transition completed⟶completable.");

    // Now add a dependent task that is unavailable.
    test(
      taskManager->taskInstances().contains<test_task::UnavailableTask>(),
      "Expected UnavailableTask to be registered.");
    std::shared_ptr<Task> t2 = taskManager->taskInstances().create<test_task::UnavailableTask>(
      Task::Configuration{ { "name", "Task 2" } }, *taskManager, managers);
    called = 0;
    success = t1->addDependency(t2);
    test(success, "Expected to add dependency to task.");
    test(called == 1, "Expected observer to be invoked upon dependency insertion.");
    test(
      from == State::Completable && to == State::Unavailable,
      "Expected state transition completable⟶unavailable.");
    success = t1->addDependency(t2);
    test(!success, "Expected failure when adding redundant dependency to task.");

    // Test construction with dependencies
    Task::Configuration c3{ { "name", "Task 3" }, { "completed", true } };
    auto t3 = taskManager->taskInstances().create<smtk::task::Task>(
      c3, std::set<std::shared_ptr<Task>>{ { t1, t2 } }, *taskManager, managers);

    // Test visitors.
    called = 0;
    test(!t3->hasChildren(), "Expected t3 to have no children.");
    t3->visit(Task::RelatedTasks::Child, [&called](Task&) {
      ++called;
      return smtk::common::Visit::Continue;
    });
    test(called == 0, "Expected not to visit any children of t3.");
    t3->visit(Task::RelatedTasks::Depend, [&called](Task&) {
      ++called;
      return smtk::common::Visit::Continue;
    });
    test(called == 2, "Expected to visit 2 dependencies of t3.");
    called = 0;
    t3->visit(Task::RelatedTasks::Depend, [&called](Task&) -> smtk::common::Visit {
      ++called;
      return smtk::common::Visit::Halt;
    });
    test(called == 1, "Expected to terminate early for dependency visitor.");

    // Test dependency removal and notification.
    auto dokey = t3->observers().insert(callback);
    called = 0;
    success = t3->removeDependency(t2);
    test(success, "Expected to remove dependency from Task 3.");
    test(called == 0, "Did not expect state to change when removing first dependency.");
    success = t1->removeDependency(t2);
    test(success, "Expected to remove dependency from Task 1.");
    test(called == 2, "Expected 2 tasks to change state.");
    test(
      from == State::Unavailable && to == State::Completable,
      "Expected state transition unavailable⟶completable.");
    t1->markCompleted(true);
    success = t1->removeDependency(t2);
    test(!success, "Expected removal of non-existent dependency to fail.");

    // Test GatherResources
    // I. Construction w/ configuration.
    //    The task is incomplete unless 1 or 2 model geometry resources
    //    and 1 or more simulation attribute resources are held by the
    //    resource manager.
    Task::Configuration c4{
      { "name", "Task 4" },
      { "auto-configure", true },
      { "resources",
        { { { "role", "model geometry" }, { "type", "smtk::model::Resource" }, { "max", 2 } },
          { { "role", "simulation attribute" }, { "type", "smtk::attribute::Resource" } } } }
    };
    auto t4 =
      taskManager->taskInstances().create<smtk::task::GatherResources>(c4, *taskManager, managers);
    test(!!t4, "Could not create GatherResources.");
    test(t4->state() == State::Incomplete, "Task with no resources should be incomplete.");
    auto hokey = t4->observers().insert(callback);

    // II. State transitions
    called = 0;
    bool didAdd;
    // Add 3 resources. Addition of the last should cause transition.
    auto model1 = smtk::model::Resource::create();
    model1->properties().get<std::string>()["project_role"] = "model geometry";
    didAdd = resourceManager->add(model1);
    test(didAdd, "Expected to add model1 resource.");
    test(called == 0, "Did not expect state to change when adding first model.");
    auto model2 = smtk::model::Resource::create();
    model2->properties().get<std::string>()["project_role"] = "model geometry";
    didAdd = resourceManager->add(model2);
    test(didAdd, "Expected to add model2 resource.");
    test(called == 0, "Did not expect state to change when adding first model.");
    auto attr1 = smtk::attribute::Resource::create();
    attr1->properties().get<std::string>()["project_role"] = "simulation attribute";
    didAdd = resourceManager->add(attr1);
    test(didAdd, "Expected to add attr1 resource.");
    test(called == 1, "Expected state to change when required resources present.");
    test(
      from == State::Incomplete && to == State::Completable,
      "Expected state transition incomplete⟶completable.");

    t4->markCompleted(true); // We'll test below that completion is reset.
    test(called == 2, "Expected state to change when user marks completed.");
    test(
      from == State::Completable && to == State::Completed,
      "Expected state transition completable⟶completed.");

    called = 0;
    bool didRemove;
    // Remove 2 resources. Removal of the last should cause a transition.
    didRemove = resourceManager->remove(model1);
    test(didRemove, "Expected to remove model1 resource.");
    test(called == 0, "Did not expect state to change when removing model1.");
    didRemove = resourceManager->remove(attr1);
    test(didRemove, "Expected to remove attr1 resource.");
    test(called == 1, "Expected state to change when removing attr1 model.");
    test(
      from == State::Completed && to == State::Incomplete,
      "Expected state transition completed⟶incomplete.");

    // Test that user completion is reset by transitions "below" Completeable and back.
    called = 0;
    didAdd = resourceManager->add(attr1);
    test(didAdd, "Expected to add attr1 resource.");
    test(called == 1, "Expected state to change when required resources present.");
    test(
      from == State::Incomplete && to == State::Completable,
      "Expected state transition incomplete⟶completable.");

    // III. Verify that an empty role is allowed, as are empty resource types.
    //      This should also test initialization of GatherResources when
    //      resource manager is not empty.
    Task::Configuration c5{ { "name", "Task 5" },
                            { "auto-configure", true },
                            { "resources",
                              { { { "type", "smtk::model::Resource" } },
                                { { "role", "simulation attribute" } } } } };
    auto t5 =
      taskManager->taskInstances().createFromName("smtk::task::GatherResources", c5, managers);
    test(!!t5, "Could not create GatherResources.");
    auto pokey = t5->observers().insert(callback);
    test(t5->state() == State::Completable, "Task 5 should be completable initially.");
    called = 0;
    didRemove = resourceManager->remove(attr1);
    test(didRemove, "Expected to remove attr1 resource.");
    // NB: called == 2 since both task 4 and 5 should transition:
    test(called == 2, "Expected state to change when removing attr1 model.");
    test(
      from == State::Completable && to == State::Incomplete,
      "Expected state transition completable⟶incomplete.");

    // Test task Instances methods:

    // Verify double-add fails.
    didAdd = taskManager->taskInstances().manage(t5);
    test(!didAdd, "Should not return true when duplicate instance managed.");

    // Test visit()
    std::cout << "Tasks:\n";
    taskManager->taskInstances().visit(countInstances);
    test(count == 5, "Expected 5 tasks.");

    // Test removal and addition.
    didRemove = taskManager->taskInstances().unmanage(t5);
    test(didRemove, "Expected to unmanage task 5.");
    test(!taskManager->taskInstances().contains(t5), "Expected task 5 to be absent.");
    didAdd = taskManager->taskInstances().manage(t5);
    test(didAdd, "Expected to manage task 5.");
    test(taskManager->taskInstances().contains(t5), "Expected task 5 to be present.");

    taskManager->taskInstances().clear();
    test(!test_task::taskDestroyed, "Task 2 should still be alive while t2 in scope.");
  }
  test(test_task::taskDestroyed, "Task 2 should be dead once t2 is out of scope.");

  count = 0;
  taskManager->taskInstances().visit(countInstances);
  test(count == 0, "Expected 0 tasks.");

  return 0;
}
