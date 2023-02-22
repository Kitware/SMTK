//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/Managers.h"
#include "smtk/plugin/Registry.h"
#include "smtk/resource/json/Helper.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Registrar.h"
#include "smtk/task/Task.h"
#include "smtk/task/UIState.h"
#include "smtk/task/UIStateGenerator.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonManager.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "nlohmann/json.hpp"

#include <iostream>

namespace
{
class TaskUIStateGenerator : public smtk::task::UIStateGenerator
{
public:
  TaskUIStateGenerator() = default;
  virtual ~TaskUIStateGenerator() = default;

  nlohmann::json globalState() const override { return nlohmann::json(); }
  nlohmann::json taskState(const std::shared_ptr<smtk::task::Task>& task) const override
  {
    (void)task;
    nlohmann::json state = R"({ "test": "passed" })"_json;
    return state;
  }
};
} // namespace

int TestTaskUIState(int, char*[])
{
  // Create managers
  auto managers = smtk::common::Managers::create();
  auto taskRegistry = smtk::plugin::addToManagers<smtk::task::Registrar>(managers);

  auto taskManager = smtk::task::Manager::create();
  auto taskTaskRegistry = smtk::plugin::addToManagers<smtk::task::Registrar>(taskManager);

  // Add UI state generator
  std::shared_ptr<TaskUIStateGenerator> gen(new TaskUIStateGenerator);
  auto baseGen = std::dynamic_pointer_cast<smtk::task::UIStateGenerator>(gen);
  taskManager->uiState().setGenerator("TaskUIStateGenerator", baseGen);

  // Create task
  std::shared_ptr<smtk::task::Task> t1 = taskManager->taskInstances().create<smtk::task::Task>(
    smtk::task::Task::Configuration{ { "title", "Task 1" } }, *taskManager, managers);
  smtkTest(t1 != nullptr, "failed to create task.");

  // Generate json object
  auto& resourceHelper = smtk::resource::json::Helper::instance();
  resourceHelper.setManagers(managers);
  auto& taskHelper =
    smtk::task::json::Helper::pushInstance(*taskManager, resourceHelper.managers());
  taskHelper.setManagers(resourceHelper.managers());
  nlohmann::json j = *taskManager;
  smtk::task::json::Helper::popInstance();
  std::cout << j << std::endl;

  // Verify that ui content was generated
  auto jtest = j["tasks"][0]["ui"]["TaskUIStateGenerator"]["test"];
  smtkTest(jtest.get<std::string>() == "passed", "did not find \"passed\" field.");

  // Future: come up with test reading json object to initialize (second) task manager.

  return 0;
}
