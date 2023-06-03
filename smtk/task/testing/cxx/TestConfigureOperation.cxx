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
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/plugin/Registry.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Registrar.h"
#include "smtk/task/Instances.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Registrar.h"
#include "smtk/task/Task.h"

#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonManager.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "nlohmann/json.hpp"

#include <string>
#include <vector>

namespace
{
std::string attTemplate = R"(
  <SMTK_AttributeResource Version="6">
    <Definitions>
      <AttDef Type="spec-2d">
        <ItemDefinitions>
          <Double Name="origin" NumberOfRequiredValues="2">
            <DefaultValue>1.1,2.2</DefaultValue>
          </Double>
        </ItemDefinitions>
      </AttDef>
    </Definitions>
  </SMTK_AttributeResource>
)";

std::string tasksConfig = R"(
  {
    "adaptors": [
      {
        "configure": [
          {
            "attribute[type='spec-2d']/origin": "/dimension/origin2d",
            "from-role": "attributes"
          }
        ],
        "from": 1,
        "id": 1,
        "to": 2,
        "type": "smtk::task::adaptor::ConfigureOperation"
      }
    ],
    "styles": {
      "operation_view": {
        "operation-panel": {
          "focus-task-operation": true
        }
      }
    },
    "tasks": [
      {
        "attribute-sets": [
          {
            "definitions": [
              "spec-2d"
            ],
            "role": "attributes"
          }
        ],
        "id": 1,
        "title": "Edit Attributes",
        "type": "smtk::task::FillOutAttributes"
      },
      {
        "id": 2,
        "operation": "smtk::session::mesh::CreateUniformGrid",
        "parameters": [],
        "run-style": "smtk::task::SubmitOperation::RunStyle::Once",
        "style": [
          "operation_view"
        ],
        "title": "Create Grid",
        "type": "smtk::task::SubmitOperation"
      }
    ]
  }
)";

} // anonymous namespace

int TestConfigureOperation(int, char*[])
{
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
  auto attributeOperationRegistry =
    smtk::plugin::addToManagers<smtk::attribute::Registrar>(operationManager);
  // auto modelRegistry = smtk::plugin::addToManagers<smtk::model::Registrar>(resourceManager);
  auto taskTaskRegistry = smtk::plugin::addToManagers<smtk::task::Registrar>(taskManager);

  // Create attribute resource
  auto attResource = resourceManager->create<smtk::attribute::Resource>();
  smtk::io::AttributeReader attReader;
  auto logger = smtk::io::Logger::instance();
  bool err = attReader.readContents(attResource, attTemplate, logger);
  smtkTest(!err, "failed to read attribute template");

  resourceManager->add(attResource);
  attResource->setName("attributes");
  attResource->properties().get<std::string>()["project_role"] = "attributes";
  // auto attribUUIDStr = attrib->id().toString();

  // Populate taskManager
  auto config = nlohmann::json::parse(tasksConfig);
  // std::cout << config.dump(2) << "\n";
  bool ok = true;
  try
  {
    smtk::task::json::Helper::pushInstance(*taskManager, managers);
    smtk::task::from_json(config, *taskManager);
    smtk::task::json::Helper::popInstance();
  }
  catch (std::exception&)
  {
    ok = false;
  }
  smtkTest(ok, "Failed to parse configuration.");
  smtkTest(
    taskManager->taskInstances().size() == 2,
    "Expected to deserialize 2 tasks, not " << taskManager->taskInstances().size());

  smtkTest(
    taskManager->adaptorInstances().size() == 1,
    "Expected to deserialize 1 adaptor, not " << taskManager->adaptorInstances().size());

  // Organize tasks into std::vector
  std::vector<smtk::task::Task::Ptr> tasks(2);
  std::vector<std::string> taskNames = { "Edit Attributes", "Create Grid" };
  bool hasErrors;
  taskManager->taskInstances().visit(
    [&tasks, &taskNames, &hasErrors](const smtk::task::Task::Ptr& task) {
      bool found = false;
      for (unsigned int i = 0; (i < 4) || (!found); i++)
      {
        if (task->title() == taskNames[i])
        {
          found = true;
          tasks[i] = task;
        }
      }
      if (!found)
      {
        std::cerr << "Found unexpected task: " << task->title() << std::endl;
        hasErrors = true;
      }
      return smtk::common::Visit::Continue;
    });

  return 0;
}
