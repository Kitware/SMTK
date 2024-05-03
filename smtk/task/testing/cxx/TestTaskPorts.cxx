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
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/operators/Signal.h"
#include "smtk/common/Managers.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Volume.h"
#include "smtk/operation/Manager.h"
#include "smtk/plugin/Registry.h"
#include "smtk/resource/Manager.h"
#include "smtk/task/FillOutAttributes.h"
#include "smtk/task/GatherResources.h"
#include "smtk/task/Instances.h"
#include "smtk/task/Manager.h"
#include "smtk/task/ObjectsInRoles.h"
#include "smtk/task/Port.h"
#include "smtk/task/Task.h"
// #include "smtk/task/GatherResources.h"

#include "smtk/resource/json/Helper.h"

#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonManager.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/attribute/Registrar.h"
#include "smtk/model/Registrar.h"
#include "smtk/operation/Registrar.h"
#include "smtk/project/Registrar.h"
#include "smtk/resource/Registrar.h"
#include "smtk/task/Registrar.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::string::literals;

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

void printPortData(const std::shared_ptr<smtk::task::PortData>& data)
{
  std::cout << "Data (" << data->typeName() << ")\n";
  if (auto oir = std::dynamic_pointer_cast<smtk::task::ObjectsInRoles>(data))
  {
    for (const auto& entry : oir->data())
    {
      std::cout << "  " << entry.first.data() << ": (" << entry.second.size() << ")\n";
      for (const auto& obj : entry.second)
      {
        std::cout << "    " << obj << " (" << obj->typeName() << ")\n";
      }
    }
  }
  else
  {
    std::cout << "  Nothing to report.\n";
  }
}

} // anonymous namespace

int TestTaskPorts(int, char*[])
{
  using smtk::task::Port;
  using smtk::task::PortData;
  using smtk::task::State;
  using smtk::task::Task;
  using smtk::task::TaskManagerWorkflowObserver;
  using smtk::task::WorkflowEvent;

  auto managers = smtk::common::Managers::create();
#if 0
  auto registries = smtk::plugin::registerWithManagers<
    std::tuple<
        smtk::resource::Registrar
      , smtk::attribute::Registrar
      , smtk::operation::Registrar
      , smtk::task::Registrar
      , smtk::project::Registrar
    >,
    std::tuple<
        smtk::attribute::AssociationRuleManager
      , smtk::attribute::EvaluatorManager
      , smtk::common::Managers
      // , smtk::extension::paraview::appcomponents::Registrar
      // , smtk::extension::qtManager
      // , smtk::geometry::Manager
      , smtk::operation::Manager
      , smtk::project::Manager
      , smtk::resource::Manager
      , smtk::resource::query::Manager
      , smtk::task::Manager
      , smtk::view::Manager
    >
  >(managers);
#endif

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

  auto attrib = resourceManager->create<smtk::attribute::Resource>();
  auto model = resourceManager->create<smtk::model::Resource>();
  resourceManager->add(attrib);
  resourceManager->add(model);

  attrib->setName("simulation");
  model->setName("geometry");
  attrib->properties().get<std::string>()["project_role"] = "simulation attribute";
  model->properties().get<std::string>()["project_role"] = "model geometry";
  auto attribUUIDStr = attrib->id().toString();
  auto modelUUIDStr = model->id().toString();

  std::string configString = R"({
  "ports": [
    {
      "id": 1,
      "type": "smtk::task::Port",
      "direction": "in",
      "name": "attribute resource in",
      "data-types": [ "smtk::task::ObjectsInRoles" ]
    },
    {
      "id": 2,
      "type": "smtk::task::Port",
      "direction": "out",
      "name": "attribute resource out",
      "data-types": [ "smtk::task::ObjectsInRoles" ]
    }
  ],
  "tasks": [
    {
      "id": 1,
      "type": "smtk::task::FillOutAttributes",
      "name": "Mark up model",
      "state": "incomplete",
      "ports": {
        "in": 1,
        "out": 2
      },
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
  ]
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
  test(taskManager->taskInstances().size() == 1, "Expected to deserialize 1 tasks.");

  smtk::task::FillOutAttributes::Ptr fillOutAttributes;
  taskManager->taskInstances().visit([&fillOutAttributes](const smtk::task::Task::Ptr& task) {
    if (!fillOutAttributes)
    {
      fillOutAttributes = std::dynamic_pointer_cast<smtk::task::FillOutAttributes>(task);
    }
    return smtk::common::Visit::Continue;
  });

  test(!!fillOutAttributes, "Failed to find FillOutAttributes task.");

  // Test that styles are properly deserialized.
  test(fillOutAttributes->style().empty(), "Expected no style for fillOutAttributes.");

  // Add components and signal to test FillOutAttributes.
  // First, the FillOutAttributes task is irrelevant
  printTaskStates(taskManager);
  test(
    fillOutAttributes->state() == smtk::task::State::Unavailable,
    "Expected an unconfigured task to be unavailable.");
  // Connect resource to input port.
  auto ports = fillOutAttributes->ports();
  auto portIt = ports.find("in");
  test(portIt != ports.end(), "Failed to find input port.");
  auto* attInPort = portIt->second;
  std::unordered_set<smtk::string::Token> expectedDataTypes{
    { "smtk::task::ObjectsInRoles"_token }
  };
  test(
    attInPort->dataTypes() == expectedDataTypes,
    "Expected input port to accept ObjectsInRoles data.");
  attInPort->connections().insert(attrib.get());
  // Signal that connections have been modified.
  // This should cause a change in state.
  fillOutAttributes->portDataUpdated(attInPort);

  printTaskStates(taskManager);
  test(
    fillOutAttributes->state() == smtk::task::State::Unavailable,
    "Expected task with no attributes to be unavailable.");

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
  test(
    fillOutAttributes->state() == smtk::task::State::Incomplete,
    "Expected task with partially-assigned attributes to be incomplete.");

  material1->associate(volume1.component());
  signal->parameters()->findComponent("created")->setNumberOfValues(0);
  signal->parameters()->findComponent("modified")->appendValue(material1);
  result = signal->operate();
  std::cout << "Signaled after associating to the material\n";
  // Finally, the FillOutAttributes task is completable
  printTaskStates(taskManager);
  test(
    fillOutAttributes->state() == smtk::task::State::Completable,
    "Expected task with fully-assigned attributes to be completable.");

  portIt = ports.find("out");
  test(portIt != ports.end(), "Expected to find output port for FillOutAttributes.");
  auto* attOutPort = portIt->second;
  test(!!attOutPort, "Expected to have an output port.");
  test(
    attOutPort->dataTypes() == expectedDataTypes,
    "Expected output port to produce ObjectsInRoles data.");
  test(!fillOutAttributes->portData(attInPort), "Expect no data for input port.");
  auto data = fillOutAttributes->portData(attOutPort);
  printPortData(data);
  auto oir = std::dynamic_pointer_cast<smtk::task::ObjectsInRoles>(data);
  test(!!oir, "Expected data to be 'ObjectsInRoles' type.");
  test(oir->data().size() == 1, "Expected a single object in output port data.");
  auto pdit = oir->data().find("simulation attribute");
  test(pdit != oir->data().end(), "Expected to find 'simulation attribute' role.");
  test(pdit->second.size() == 1, "Expected to find one object in 'simulation attribute' role.");
  test(*pdit->second.begin() == attrib.get(), "Expected attribute resource as output data.");

  return 0;
}
