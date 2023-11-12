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
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/operators/Signal.h"
#include "smtk/common/Managers.h"
#include "smtk/model/Registrar.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Volume.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/plugin/Registry.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Registrar.h"
#include "smtk/task/FillOutAttributes.h"
#include "smtk/task/GatherResources.h"
#include "smtk/task/Instances.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Registrar.h"
#include "smtk/task/Task.h"
// #include "smtk/task/GatherResources.h"

#include "smtk/resource/json/Helper.h"

#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonManager.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/common/testing/cxx/helpers.h"

namespace test_task
{

using namespace smtk::task;

} // namespace test_task

namespace
{

void printTaskStates(smtk::task::Manager::Ptr taskManager)
{
  taskManager->taskInstances().visit([](const std::shared_ptr<smtk::task::Task>& task) {
    std::cout << task->name() << ": " << smtk::task::stateName(task->state()) << "\n";
    return smtk::common::Visit::Continue;
  });
}

} // anonymous namespace

int TestTaskJSON(int, char*[])
{
  using smtk::task::State;
  using smtk::task::Task;
  using smtk::task::TaskManagerWorkflowObserver;
  using smtk::task::WorkflowEvent;

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
  auto modelRegistry = smtk::plugin::addToManagers<smtk::model::Registrar>(resourceManager);
  auto taskTaskRegistry = smtk::plugin::addToManagers<smtk::task::Registrar>(taskManager);

  TaskManagerWorkflowObserver wfObserver =
    [&](const std::set<Task*>& workflows, WorkflowEvent event, Task* subject) {
      std::cout << "-- Workflow event " << static_cast<int>(event) << " subject: " << subject
                << " workflows:\n";
      for (const auto& wftask : workflows)
      {
        std::cout << "--  head task " << wftask->name() << "\n";
      }
      std::cout << "--\n";
    };
  auto workflowObserver = taskManager->workflowObservers().insert(wfObserver);

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
      "type": "smtk::task::GatherResources",
      "name": "Load a model and attribute",
      "style": [ "foo", "bar", "baz" ],
      "state": "completed",
      "auto-configure": true,
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
      ]
    },
    {
      "id": 2,
      "type": "smtk::task::FillOutAttributes",
      "name": "Mark up model",
      "state": "incomplete",
      "dependencies": [ 1 ],
      "attribute-sets": [
        {
          "role": "simulation attribute",
          "definitions": [
            "Material",
            "BoundaryCondition"
          ]
        }
      ]
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
  "adaptors": [
    {
      "id": 1,
      "type": "smtk::task::adaptor::ResourceAndRole",
      "from": 1,
      "to": 2
    }
  ],
  "styles": {
    "foo": { },
    "bar": { },
    "baz": { }
  }
}
    )";

  auto config = nlohmann::json::parse(configString);
  std::cout << config.dump(2) << "\n";

  bool ok = true;
  auto& resourceHelper = smtk::resource::json::Helper::instance();
  resourceHelper.setManagers(managers);
  try
  {
    auto& taskHelper =
      smtk::task::json::Helper::pushInstance(*taskManager, resourceHelper.managers());
    taskHelper.setManagers(resourceHelper.managers());
    from_json(config, *taskManager);
    smtk::task::json::Helper::popInstance();
  }
  catch (std::exception&)
  {
    ok = false;
  }

  test(ok, "Failed to parse configuration.");
  test(taskManager->taskInstances().size() == 2, "Expected to deserialize 2 tasks.");

  smtk::task::GatherResources::Ptr gatherResources;
  smtk::task::FillOutAttributes::Ptr fillOutAttributes;
  taskManager->taskInstances().visit(
    [&gatherResources, &fillOutAttributes](const smtk::task::Task::Ptr& task) {
      if (!gatherResources)
      {
        gatherResources = std::dynamic_pointer_cast<smtk::task::GatherResources>(task);
      }
      if (!fillOutAttributes)
      {
        fillOutAttributes = std::dynamic_pointer_cast<smtk::task::FillOutAttributes>(task);
      }
      return smtk::common::Visit::Continue;
    });

  // Test that styles are properly deserialized.
  test(fillOutAttributes->style().empty(), "Expected no style for fillOutAttributes.");
  test(gatherResources->style().size() == 3, "Expected 3 style class-names for gatherResources.");

  // Add components and signal to test FillOutAttributes.
  // First, the FillOutAttributes task is irrelevant
  printTaskStates(taskManager);
  gatherResources->markCompleted(true);

  printTaskStates(taskManager);

  auto volume1 = model->addVolume();
  auto volume2 = model->addVolume();
  auto def = attrib->createDefinition("Material");
  auto assoc = def->createLocalAssociationRule();
  assoc->setAcceptsEntries("smtk::model::Resource", "volume", true);
  assoc->setNumberOfRequiredValues(1);
  assoc->setIsExtensible(true);
  auto material1 = attrib->createAttribute("CarbonFiber", "Material");

  // Signal the change
  auto signal = operationManager->create<smtk::attribute::Signal>();
  signal->parameters()->findComponent("created")->appendValue(material1);
  auto result = signal->operate();
  std::cout << "Signaled after adding a material\n";
  // Now, the FillOutAttributes task is incomplete
  printTaskStates(taskManager);

  material1->associate(volume1.component());
  signal->parameters()->findComponent("created")->setNumberOfValues(0);
  signal->parameters()->findComponent("modified")->appendValue(material1);
  result = signal->operate();
  std::cout << "Signaled after associating to the material\n";
  // Finally, the FillOutAttributes task is completable
  printTaskStates(taskManager);

  std::string configString2;
  {
    // Round trip it.
    nlohmann::json config2;
    try
    {
      smtk::task::json::Helper::pushInstance(*taskManager, resourceHelper.managers());
      config2 = *taskManager;
      smtk::task::json::Helper::popInstance();
    }
    catch (std::exception&)
    {
      ok = false;
    }
    test(ok, "Failed to serialize task manager.");
    configString2 = config2.dump(2);
    std::cout << configString2 << "\n";
  }
  {
    // TODO: Round trip a second time so we should be guaranteed to have
    // two machine-(not-hand-)generated strings to compare.
    auto managers2 = smtk::common::Managers::create();
    managers2->insert(resourceManager);
    managers2->insert(operationManager);
    auto taskManager2 = smtk::task::Manager::create();
    smtk::task::Registrar::registerTo(taskManager2);
    auto config3 = nlohmann::json::parse(configString2);
    try
    {
      smtk::task::json::Helper::pushInstance(*taskManager2, resourceHelper.managers());
      from_json(config3, *taskManager2);
      smtk::task::json::Helper::popInstance();
    }
    catch (std::exception&)
    {
      ok = false;
    }
    test(ok, "Failed to parse second configuration.");
    test(taskManager2->taskInstances().size() == 2, "Expected to deserialize 2 tasks.");

    // Test that styles are properly round-tripped.
    smtk::task::GatherResources::Ptr gatherResources2;
    smtk::task::FillOutAttributes::Ptr fillOutAttributes2;
    taskManager->taskInstances().visit(
      [&gatherResources2, &fillOutAttributes2](const smtk::task::Task::Ptr& task) {
        if (!gatherResources2)
        {
          gatherResources2 = std::dynamic_pointer_cast<smtk::task::GatherResources>(task);
        }
        if (!fillOutAttributes2)
        {
          fillOutAttributes2 = std::dynamic_pointer_cast<smtk::task::FillOutAttributes>(task);
        }
        return smtk::common::Visit::Continue;
      });
    test(fillOutAttributes2->style().empty(), "Expected no style for fillOutAttributes2.");
    test(
      gatherResources2->style().size() == 3, "Expected 3 style class-names for gatherResources2.");

    nlohmann::json config4;
    try
    {
      smtk::task::json::Helper::pushInstance(*taskManager2, resourceHelper.managers());
      config4 = *taskManager2;
      smtk::task::json::Helper::popInstance();
    }
    catch (std::exception&)
    {
      ok = false;
    }
    test(ok, "Failed to serialize second manager.");
    std::string configString3 = config4.dump(2);
    std::cout << "----\n" << configString3 << "\n";
    // test(configString2 == configString3, "Failed to match strings.");
  }

  return 0;
}
