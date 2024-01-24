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
#include "smtk/attribute/operators/Signal.h"
#include "smtk/common/Managers.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/Registrar.h"
#include "smtk/plugin/Registry.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Registrar.h"
#include "smtk/task/GatherResources.h"
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

#include <atomic>
#include <cmath>
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
      <AttDef Type="unused-att">
        <ItemDefinitions>
          <Double Name="item1">
            <DefaultValue>-999.999</DefaultValue>
          </Double>
        </ItemDefinitions>
      </AttDef>
    </Definitions>
    <Attributes>
      <Att Type="source-att" Name="source-att" />
      <Att Type="unused-att" Name="unused-att" />
    </Attributes>
  </SMTK_AttributeResource>
)";

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
        }
      }
    },
    "tasks": [
      {
        "auto-configure": false,
        "id": 1,
        "resources": [
          {
            "role": "attributes",
            "type": "smtk::attribute::Resource",
            "max": 1
          }
        ],
        "title": "Assign Attribute Resource",
        "type": "smtk::task::GatherResources"
      },
      {
        "attribute-sets": [
          {
            "definitions": [
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

  Result operateInternal() override
  {
    std::cerr << "Running SimpleOperation!\n";
    return this->createResult(m_outcome);
  }

  Outcome m_outcome{ Outcome::SUCCEEDED };
};

void checkTaskStates(
  const std::vector<smtk::task::Task::Ptr>& tasks,
  const std::vector<smtk::task::State>& expected)
{
  for (std::size_t i = 0; i < tasks.size(); ++i)
  {
    const smtk::task::Task::Ptr task = tasks[i];
    smtkTest(
      task->state() == expected[i],
      "Task " << task->name() << " expected state " << expected[i] << " actual state "
              << task->state());
  }
}

void printTaskStates(
  const std::vector<smtk::task::Task::Ptr>& tasks,
  const std::string& note = std::string())
{
  if (!note.empty())
  {
    std::cout << note << '\n';
  }

  for (const auto& task : tasks)
  {
    std::cout << task->name() << " -- " << task->state() << '\n';
  }

  std::cout << std::flush;
}

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
  taskManager->setManagers(managers);

  auto attributeResourceRegistry =
    smtk::plugin::addToManagers<smtk::attribute::Registrar>(resourceManager);
  auto attributeOperationRegistry =
    smtk::plugin::addToManagers<smtk::attribute::Registrar>(operationManager);
  auto taskTaskRegistry = smtk::plugin::addToManagers<smtk::task::Registrar>(taskManager);

  // Register the test operation
  bool registered = operationManager->registerOperation<SimpleOperation>("SimpleOperation");
  smtkTest(registered, "failed to register SimpleOperation");

  // Since we will have operations running in different threads, we need to know when all of them
  // have completed before we return.
  std::atomic<int> numberOfOperationsCompleted(0);
  smtk::operation::Observers::Key handleTmp = operationManager->observers().insert(
    [&numberOfOperationsCompleted](
      const smtk::operation::Operation& op,
      smtk::operation::EventType event,
      smtk::operation::Operation::Result /*unused*/) -> int {
      if (event == smtk::operation::EventType::DID_OPERATE)
      {
        std::cerr << "[x] " << op.typeName() << " event " << static_cast<int>(event)
                  << " number of operations run: " << numberOfOperationsCompleted + 1 << ".\n";
        std::cerr.flush();
        numberOfOperationsCompleted++;
      }
      return 0;
    },
    std::numeric_limits<smtk::operation::Observers::Priority>::lowest(),
    false);

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

  // Add a second attribute resource with same role to make sure it isn't used
  auto unusedAttResource = resourceManager->create<smtk::attribute::Resource>();
  resourceManager->add(unusedAttResource);
  unusedAttResource->setName("attributes");
  unusedAttResource->properties().get<std::string>()["project_role"] = "attributes";

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
        if (task->name() == taskNames[i])
        {
          found = true;
          tasks[i] = task;
        }
      }
      if (!found)
      {
        std::cerr << "Found unexpected task: " << task->name() << std::endl;
        hasErrors = true;
      }
      return smtk::common::Visit::Continue;
    });

  // Set attribute resource for GatherResources
  auto gatherTask = std::dynamic_pointer_cast<smtk::task::GatherResources>(tasks.front());
  smtkTest(gatherTask != nullptr, "failed to get GatherResources task");

  // Check initial task states
  printTaskStates(tasks, "\n*** Initial states:");
  std::vector<smtk::task::State> initialExpected = { smtk::task::State::Incomplete,
                                                     smtk::task::State::Unavailable,
                                                     smtk::task::State::Incomplete };
  checkTaskStates(tasks, initialExpected);

  // Set the GatherResources' attribute resource
  gatherTask->addResourceInRole(attResource, "attributes");

  printTaskStates(tasks, "\n*** After GatherResources:");
  std::vector<smtk::task::State> gatherExpected = { smtk::task::State::Completable,
                                                    smtk::task::State::Completable,
                                                    smtk::task::State::Completable };
  checkTaskStates(tasks, gatherExpected);

  // Check SubmitOperation content
  auto submitTask = std::dynamic_pointer_cast<smtk::task::SubmitOperation>(tasks.back());
  smtkTest(submitTask != nullptr, "failed to get SubmitOperation task");
  smtkTest(submitTask->operation()->ableToOperate(), "operation not able to operate");
  {
    double expected = 1.1;
    double value = submitTask->operation()->parameters()->findDouble("parameter1")->value();
    double diff = std::fabs(value - expected);
    smtkTest(diff < 0.001, "expected parameter value to be " << expected << " not " << value);
  }

  // Change the source item's value and emit Signal
  auto specItem = specAtt->findDouble("item1");
  specItem->setValue(3.14159);
  auto signal = operationManager->create<smtk::attribute::Signal>();
  signal->parameters()->findComponent("modified")->appendValue(specAtt);
  auto result = signal->operate();

  // Wait for all of the operations to be done
  while (numberOfOperationsCompleted != 3)
  {
  }

  // Task states should be the same but parameter value changed
  checkTaskStates(tasks, gatherExpected);
  {
    double expected = 3.14159;
    double value = submitTask->operation()->parameters()->findDouble("parameter1")->value();
    double diff = std::fabs(value - expected);
    smtkTest(diff < 0.001, "expected parameter value to be " << expected << " not " << value);
  }

  // Print any log messages
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
