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
#include "smtk/task/Manager.h"
#include "smtk/task/Registrar.h"
#include "smtk/task/Task.h"
#include "smtk/task/TaskNeedsResources.h"

#include "smtk/task/json/jsonManager.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/common/testing/cxx/helpers.h"

namespace test_task
{

using namespace smtk::task;

} // namespace test_task

int TestTaskJSON(int, char*[])
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
  auto taskManager = managers->get<smtk::task::Manager::Ptr>();

  auto attributeResourceRegistry =
    smtk::plugin::addToManagers<smtk::attribute::Registrar>(resourceManager);
  auto modelRegistry = smtk::plugin::addToManagers<smtk::model::Registrar>(resourceManager);
  auto taskTaskRegistry = smtk::plugin::addToManagers<smtk::task::Registrar>(taskManager);

  auto attrib = resourceManager->create<smtk::attribute::Resource>();
  auto model = resourceManager->create<smtk::model::Resource>();
  attrib->setName("simulation");
  model->setName("geometry");
  attrib->properties().get<std::string>()["project_role"] = "simulation attribute";
  model->properties().get<std::string>()["project_role"] = "model geometry";
  auto attribUUIDStr = attrib->id().toString();
  auto modelUUIDStr = model->id().toString();

  std::string configString = R"({
  "tasks": [
    {
      "id": 1,
      "type": "smtk::task::TaskNeedsResources",
      "title": "Load a model and attribute",
      "state": "completed",
      "resources": [
        {
          "role": "model geometry",
          "type": "smtk::model::Resource",
          "max": 2
        },
        {
          "role": "simulation attribute",
          "type": "smtk::attribute::Resource"
        }
      ],
      "output": [
        { "role": "model geometry", "resources": [ "feedface-0000-0000-0000-000000000000" ] },
        { "role": "simulation attribute", "resources": [ "deadbeef-0000-0000-0000-000000000000" ] }
      ]
    },
    {
      "id": 2,
      "type": "smtk::task::Task",
      "title": "Do something",
      "state": "incomplete",
      "dependencies": [ 1 ]
    }
  ],
  "//": [
    "For each dependency, we can store configuration information",
    "for the object used to adapt one task's output into",
    "configuration information for its dependent task. If no",
    "configuration is present for a dependency, we use the",
    "'Ignore' adaptor as a default – which never modifies the",
    "depedent task's configuration."
  ],
  "task-adaptors": [
    {
      "type": "smtk::task::adaptor::Ignore",
      "from": 1,
      "to": 2
    }
  ]
}
    )";
  auto cursor = configString.find("deadbeef", 0);
  configString = configString.replace(cursor, attribUUIDStr.length(), attribUUIDStr);
  cursor = configString.find("feedface", 0);
  configString = configString.replace(cursor, modelUUIDStr.length(), modelUUIDStr);

  auto config = nlohmann::json::parse(configString);
  std::cout << config.dump(2) << "\n";
  bool ok = smtk::task::json::jsonManager::deserialize(managers, config);
  test(ok, "Failed to parse configuration.");
  test(taskManager->instances().size() == 2, "Expected to deserialize 2 tasks.");

  // Round trip it.
  nlohmann::json config2;
  ok = smtk::task::json::jsonManager::serialize(managers, config2);
  test(ok, "Failed to serialize task manager.");
  std::cout << config2.dump(2) << "\n";

  // TODO: Round trip a second time so we should be guaranteed to have
  // two machine-(not-hand-)generated strings to compare.

  return 0;
}
