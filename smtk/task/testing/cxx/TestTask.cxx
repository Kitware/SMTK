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
#include "smtk/resource/Manager.h"
#include "smtk/resource/Registrar.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Registrar.h"
#include "smtk/task/Task.h"
#include "smtk/task/TaskNeedsResources.h"

#include "smtk/common/testing/cxx/helpers.h"

namespace test_task
{

using namespace smtk::task;

class UnavailableTask : public Task
{
public:
  smtkTypeMacro(test_task::UnavailableTask);
  smtkCreateMacro(smtk::task::Task);
  UnavailableTask() = default;
  UnavailableTask(
    const Configuration& config,
    const smtk::common::Managers::Ptr& managers = nullptr)
    : Task(config, managers)
  {
  }
  UnavailableTask(
    const Configuration& config,
    const Task::PassedDependencies& deps,
    const smtk::common::Managers::Ptr& managers = nullptr)
    : Task(config, deps, managers)
  {
  }
  State state() const override { return State::Unavailable; }
};

} // namespace test_task

int TestTask(int, char*[])
{
  using smtk::task::State;
  using smtk::task::Task;
  using test_task::UnavailableTask;

  // Create managers
  auto managers = smtk::common::Managers::create();
  {
    smtk::resource::Registrar::registerTo(managers);
    smtk::attribute::Registrar::registerTo(managers);
    smtk::operation::Registrar::registerTo(managers);
    smtk::task::Registrar::registerTo(managers);
  }

  auto resourceManager = managers->get<smtk::resource::Manager::Ptr>();
  auto operationManager = managers->get<smtk::operation::Manager::Ptr>();
  auto taskManager = managers->get<smtk::task::Manager::Ptr>();

  {
    smtk::attribute::Registrar::registerTo(resourceManager);
    smtk::model::Registrar::registerTo(resourceManager);
    smtk::task::Registrar::registerTo(taskManager);
  }

  taskManager->taskFactory().registerType<test_task::UnavailableTask>();
  smtk::task::Task::PassedDependencies empty;

  std::shared_ptr<Task> t1 =
    taskManager->taskFactory().create<Task>(Task::Configuration{ { "title", "Task 1" } }, managers);
  test(!!t1, "Expecting to create a non-null task.");
  test(t1->state() == State::Completable, "Expected task without dependencies to be completable.");

  State from;
  State to;
  int called = 0;
  auto callback = [&from, &to, &called](Task& task, State prev, State next) {
    (void)task;
    ++called;
    from = prev;
    to = next;
    std::cout << "  " << task.title() << " transitioned: " << prev << " ⟶ " << next << "\n";
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
  std::shared_ptr<Task> t2 = taskManager->taskFactory().create<test_task::UnavailableTask>(
    Task::Configuration{ { "title", "Task 2" } }, managers);
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
  Task::Configuration c3{ { "title", "Task 3" }, { "completed", true } };
  auto t3 = taskManager->taskFactory().create<smtk::task::Task>(
    c3, std::set<std::shared_ptr<Task>>{ { t1, t2 } }, managers);

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
    from == State::Unavailable && to == State::Completed,
    "Expected state transition unavailable⟶completed.");
  success = t1->removeDependency(t2);
  test(!success, "Expected removal of non-existent dependency to fail.");

  // Test TaskNeedsResources
  // I. Construction w/ configuration.
  //    The task is incomplete unless 1 or 2 model geometry resources
  //    and 1 or more simulation attribute resources are held by the
  //    resource manager.
  Task::Configuration c4{
    { "title", "Task 4" },
    { "resources",
      { { { "role", "model geometry" }, { "type", "smtk::model::Resource" }, { "max", 2 } },
        { { "role", "simulation attribute" }, { "type", "smtk::attribute::Resource" } } } }
  };
  auto t4 = taskManager->taskFactory().create<smtk::task::TaskNeedsResources>(c4, managers);
  test(!!t4, "Could not create TaskNeedsResources.");
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
  //      This should also test initialization of TaskNeedsResources when
  //      resource manager is not empty.
  Task::Configuration c5{ { "title", "Task 5" },
                          { "resources",
                            { { { "type", "smtk::model::Resource" } },
                              { { "role", "simulation attribute" } } } } };
  auto t5 =
    taskManager->taskFactory().createFromName("smtk::task::TaskNeedsResources", c5, managers);
  test(!!t5, "Could not create TaskNeedsResources.");
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

  return 0;
}
