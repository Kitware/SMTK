//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"
#include "smtk/common/Managers.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/Registrar.h"
#include "smtk/plugin/Registry.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Registrar.h"
#include "smtk/task/Instances.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Registrar.h"
#include "smtk/task/SubmitOperation.h"
#include "smtk/task/Task.h"

#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonManager.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "nlohmann/json.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace
{
std::string attTemplate = R"(
  <SMTK_AttributeResource Version="6">
    <Definitions>
      <AttDef Type="source-att">
        <ItemDefinitions>
          <Double Name="item1">
            <DefaultValue>1.1</DefaultValue>
          </Double>
        </ItemDefinitions>
      </AttDef>
    </Definitions>
    <Attributes>
      <Att Type="source-att" Name="source-att" />
    </Attributes>
  </SMTK_AttributeResource>
)";

// Note that the FillOutAttributes task has "instances" in lieu of "definitions"
std::string tasksConfig = R"(
  {
    "adaptors": [
      {
        "from": 1,
        "id": 1,
        "to": 2,
        "type": "smtk::task::adaptor::ResourceAndRole"
      },
      {
        "configure": [
          {
            "attribute[type='source-att']/item1": "/parameter1",
            "from-role": "attributes"
          }
        ],
        "from": 2,
        "id": 2,
        "to": 3,
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
        "auto-configure": true,
        "id": 1,
        "resources": [
          {
            "role": "attributes",
            "type": "smtk::attribute::Resource"
          }
        ],
        "title": "Assign Attribute Resource",
        "type": "smtk::task::GatherResources"
      },
      {
        "attribute-sets": [
          {
            "instances": [
              "source-att"
            ],
            "role": "attributes"
          }
        ],
        "id": 2,
        "title": "Edit Attributes",
        "type": "smtk::task::FillOutAttributes"
      },
      {
        "id": 3,
        "operation": "SimpleOperation",
        "parameters": [],
        "run-style": "smtk::task::SubmitOperation::RunStyle::OnCompletion",
        "style": [
          "operation_view"
        ],
        "title": "Simple Operation",
        "type": "smtk::task::SubmitOperation"
      }
    ]
  }
)";

std::string simpleOpSpec = R"(
  <SMTK_AttributeResource Version="6">
    <Definitions>
      <AttDef Type="simple" BaseType="operation">
        <ItemDefinitions>
          <Double Name="parameter1"/>
        </ItemDefinitions>
      </AttDef>
    </Definitions>
  </SMTK_AttributeResource>
)";

class SimpleOperation : public smtk::operation::Operation
{
public:
  smtkTypeMacro(SimpleOperation);
  smtkCreateMacro(SimpleOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  SimpleOperation() = default;
  ~SimpleOperation() override = default;

  smtk::operation::Operation::Specification createSpecification() override
  {
    auto spec = this->createBaseSpecification();
    smtk::io::AttributeReader reader;
    bool err = reader.readContents(spec, simpleOpSpec, this->log());
    smtkTest(!err, "Error creating SimpleOperation spec.");
    return spec;
  }

  Result operateInternal() override { return this->createResult(m_outcome); }

  Outcome m_outcome{ Outcome::SUCCEEDED };
};

} // anonymous namespace

int TestConfigureOperation(int, char*[])
{
  std::cout << std::boolalpha;
  auto& logger = smtk::io::Logger::instance();

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
  auto taskTaskRegistry = smtk::plugin::addToManagers<smtk::task::Registrar>(taskManager);

  // Register the test operation
  bool registered = operationManager->registerOperation<SimpleOperation>("SimpleOperation");
  smtkTest(registered, "failed to register SimpleOperation");

  // Create attribute resource
  auto attResource = resourceManager->create<smtk::attribute::Resource>();
  smtk::io::AttributeReader attReader;
  bool err = attReader.readContents(attResource, attTemplate, logger);
  smtkTest(!err, "failed to read attribute template");

  smtkTest(attResource->hasAttributes(), "expected att resource to have attributes");
  // Verify that attribute was created
  auto specAtt = attResource->findAttribute("source-att");
  smtkTest(specAtt != nullptr, "source-att attribute not found");
  smtkTest(specAtt->isValid(), "source-att attribute not valid");

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
    taskManager->taskInstances().size() == 3,
    "Expected to deserialize 3 tasks, not " << taskManager->taskInstances().size());

  smtkTest(
    taskManager->adaptorInstances().size() == 2,
    "Expected to deserialize 2 adaptors, not " << taskManager->adaptorInstances().size());

  // Organize tasks into std::vector
  std::vector<std::string> taskNames = { "Assign Attribute Resource",
                                         "Edit Attributes",
                                         "Simple Operation" };
  std::size_t numTasks = taskNames.size();
  std::vector<smtk::task::Task::Ptr> tasks(numTasks);
  bool hasErrors;
  taskManager->taskInstances().visit(
    [&tasks, &taskNames, &hasErrors](const smtk::task::Task::Ptr& task) {
      bool found = false;
      for (unsigned int i = 0; (i < taskNames.size()) || (!found); i++)
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

  auto submitTask = std::dynamic_pointer_cast<smtk::task::SubmitOperation>(tasks.back());
  smtkTest(submitTask != nullptr, "failed to get SubmitOperation task");
  std::cout << __FILE__ << ":" << __LINE__ << " " << submitTask->operation()->ableToOperate()
            << std::endl;

  // For now, dump out task states
  for (const auto& task : tasks)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " " << task->title() << " -- " << task->state()
              << std::endl;
  }

  // Set GatherResources task to complete state
  tasks[0]->markCompleted(true);
  std::cout << __FILE__ << ":" << __LINE__ << " " << submitTask->operation()->ableToOperate()
            << std::endl;

  // Verify that SubmitOperation task is now completable
  smtkTest(submitTask->operation()->ableToOperate(), "operation not able to operate");
  smtkTest(
    submitTask->state() == smtk::task::State::Completable, "SubmitOperation task not completable");

  for (const auto& task : tasks)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " " << task->title() << " -- " << task->state()
              << std::endl;
  }

  if (logger.numberOfRecords() > 0)
  {
    std::cout << "\nLog:\n" << logger.convertToString(true) << std::endl;
  }
  else
  {
    std::cout << "\n(Log is empty)\n" << std::endl;
  }
  return 0;
}
