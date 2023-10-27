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

} // namespace test_task

int TestActiveTask(int, char*[])
{
  using smtk::task::State;
  using smtk::task::Task;

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

  std::cout << "Observing changes to active task\n";
  int count = 0;
  smtk::task::Task* previousTask = nullptr;
  smtk::task::Task* nextTask = nullptr;
  auto ikey = taskManager->active().observers().insert(
    [&count, &previousTask, &nextTask](smtk::task::Task* prev, smtk::task::Task* next) {
      std::cout << "  Active task switched " << (prev ? prev->name() : "(none)") << " ⟶  "
                << (next ? next->name() : "(none)") << "\n";
      ++count;
      previousTask = prev;
      nextTask = next;
    },
    /* priority */ 0,
    /* initialize */ true,
    "Observe active task for test");
  test(count == 1, "Expected observer to be initialized upon registration.");
  test(previousTask == nullptr && nextTask == nullptr, "Unexpected initialization.");

  {
    std::shared_ptr<Task> t1 = taskManager->taskInstances().create<Task>(
      Task::Configuration{ { "name", "Task 1" } }, *taskManager, managers);
    std::cout << "Attempting to set active task:\n";
    taskManager->active().switchTo(t1.get());
    test(count == 2, "Expected to switch active task.");
    test(previousTask == nullptr && nextTask == t1.get(), "Expected active switch null ⟶  t1.");

    {
      std::cout << "Ensuring unmanaged tasks cannot become active.\n";
      std::shared_ptr<Task> tmp = Task::create();
      test(!!tmp, "Expected to create unmanaged task.");
      tmp->configure(Task::Configuration{ { "name", "Unmanaged Task" } });
      taskManager->active().switchTo(tmp.get());
      test(count == 2, "Should not be able to switch to an unmanaged task.");
    }

    bool success = t1->markCompleted(true);
    test(success, "Expected to be able to complete task.");
    test(count == 2, "Change in task state does not imply active task switch.");
    success = t1->markCompleted(false);
    test(success, "Expected to be able to un-complete task.");

    // Now add a task and switch to it.
    std::shared_ptr<Task> t2 = taskManager->taskInstances().create<Task>(
      Task::Configuration{ { "name", "Task 2" } }, *taskManager, managers);
    success = t1->addDependency(t2);
    std::cout << "Switching to task 2:\n";
    taskManager->active().switchTo(t2.get());

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
    t1->addDependency(t4);
    std::cout << "Ensuring switches to unavailable tasks fail.\n";
    bool didSwitch = taskManager->active().switchTo(t1.get());
    test(!didSwitch, "Expected to fail switching to an unavailable task.");
    test(count == 3, "Did not expect switch to unavailable task.");

    // II. State transitions
    bool didAdd;
    // Add 3 resources. Addition of the last should cause transition.
    auto model1 = smtk::model::Resource::create();
    model1->properties().get<std::string>()["project_role"] = "model geometry";
    didAdd = resourceManager->add(model1);
    test(didAdd, "Expected to add model1 resource.");
    auto model2 = smtk::model::Resource::create();
    model2->properties().get<std::string>()["project_role"] = "model geometry";
    didAdd = resourceManager->add(model2);
    test(didAdd, "Expected to add model2 resource.");
    auto attr1 = smtk::attribute::Resource::create();
    attr1->properties().get<std::string>()["project_role"] = "simulation attribute";
    didAdd = resourceManager->add(attr1);
    test(didAdd, "Expected to add attr1 resource.");

    t4->markCompleted(true); // We'll test below that completion is reset.

    std::cout << "Ensuring switches to newly-available tasks succeed:\n";
    didSwitch = taskManager->active().switchTo(t1.get());
    test(didSwitch, "Expected to fail switching to an unavailable task.");
    test(count == 4, "Expected switch to now-available task.");
    test(previousTask == t2.get() && nextTask == t1.get(), "Expected active switch t2 ⟶  t1.");

    bool didRemove;
    // Remove 2 resources. Removal of the last should cause active task to reset.
    std::cout << "An active task becoming unavailable should reset the active task:\n";
    didRemove = resourceManager->remove(model1);
    test(didRemove, "Expected to remove model resource.");
    didRemove = resourceManager->remove(attr1);
    test(didRemove, "Expected to remove attribute resource.");
    test(count == 5, "Expected switch to null task when active task becomes unavailable.");
    test(previousTask == t1.get() && nextTask == nullptr, "Expected active switch t1 ⟶  (none).");

    // Test that user completion is reset by transitions "below" Completeable and back.
    std::cout << "Switching to a now-available t1:\n";
    didAdd = resourceManager->add(attr1);
    didSwitch = taskManager->active().switchTo(t1.get());
    test(didAdd && didSwitch, "Expected to succeed switching to a now-available task.");
    test(count == 6, "Expected switch to now-available task.");
    test(previousTask == nullptr && nextTask == t1.get(), "Expected active switch (none) ⟶  t1.");

    std::cout << "Unmanaging the active task should reset the active task:\n";
    taskManager->taskInstances().clear();
    test(count == 7, "Expected switch to null task when a task is dropped from manager.");
    test(previousTask == t1.get() && nextTask == nullptr, "Expected active switch t1 ⟶  (none).");
  }

  return 0;
}
