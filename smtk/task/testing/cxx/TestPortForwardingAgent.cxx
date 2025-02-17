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
#include "smtk/attribute/Resource.h"
#include "smtk/common/Managers.h"
#include "smtk/model/Face.h"
#include "smtk/model/Resource.h"
#include "smtk/operation/Manager.h"
#include "smtk/plugin/Registry.h"
#include "smtk/resource/Manager.h"
#include "smtk/task/Instances.h"
#include "smtk/task/Manager.h"
#include "smtk/task/ObjectsInRoles.h"
#include "smtk/task/Port.h"
#include "smtk/task/Task.h"

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

namespace
{

std::string configString = R"v0g0nPoetry({
  "ports": [
    {
      "id": 1,
      "type": "smtk::task::Port",
      "direction": "in",
      "name": "attribute objects in",
      "unassigned-role": "attribute",
      "data-types": [ "smtk::task::ObjectsInRoles" ]
    },
    {
      "id": 2,
      "type": "smtk::task::Port",
      "direction": "in",
      "unassigned-role": "model",
      "name": "model objects in",
      "data-types": [ "smtk::task::ObjectsInRoles" ]
    },
    {
      "id": 3,
      "type": "smtk::task::Port",
      "direction": "out",
      "name": "all out",
      "data-types": [ "smtk::task::ObjectsInRoles" ]
    },
    {
      "id": 4,
      "type": "smtk::task::Port",
      "direction": "out",
      "name": "components out",
      "data-types": [ "smtk::task::ObjectsInRoles" ]
    },
    {
      "id": 5,
      "type": "smtk::task::Port",
      "direction": "out",
      "name": "resources out",
      "data-types": [ "smtk::task::ObjectsInRoles" ]
    }
  ],
  "tasks": [
    {
      "id": 1,
      "type": "smtk::task::Task",
      "ports": {
        "attribute objects in": 1,
        "model objects in": 2,
        "all out": 3,
        "components out": 4,
        "resources out": 5
      },
      "agents": [
        {
          "type": "smtk::task::PortForwardingAgent",
          "name": "foo",
          "forwards": [
            {
              "//": "Copy everything to 'all out' via default filters.",
              "input-port": "attribute objects in",
              "output-port": "all out"
            },
            {
              "//": "Copy only components to 'components out' by wildcards.",
              "input-port": "attribute objects in",
              "output-port": "components out",
              "output-role": "components",
              "filters": {
                "attribute": [ [ "*", "*" ] ]
              }
            },
            {
              "//": "Copy only resources to 'resources out' by typename.",
              "input-port": "attribute objects in",
              "output-port": "resources out",
              "output-role": "resources",
              "filters": {
                "attribute": [
                  { "resource": "smtk::attribute::Resource" }
                ]
              }
            },
            {
              "//": "Copy everything to 'all out' via default filters.",
              "input-port": "model objects in",
              "output-port": "all out"
            },
            {
              "//": "Copy only components to 'components out' by typename.",
              "input-port": "model objects in",
              "output-port": "components out",
              "filters": {
                "model": [
                  { "resource": "smtk::model::Resource", "component": "face" },
                  { "resource": "smtk::model::Resource", "component": "model" }
                ]
              }
            },
            {
              "//": "Copy only resources to 'resources out' by typename.",
              "input-port": "model objects in",
              "output-port": "resources out",
              "filters": {
                "model": [
                  { "resource": "smtk::model::Resource" }
                ]
              }
            }
          ]
        }
      ],
      "name": "TestPortForwarding",
      "state": "completable"
    }
  ]
})v0g0nPoetry";
smtk::task::Port* getTaskPort(const smtk::task::Task::Ptr& task, smtk::string::Token portName)
{
  auto it = task->ports().find(portName);
  if (it == task->ports().end())
  {
    return nullptr;
  }
  return it->second;
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

void checkPortData(
  const std::shared_ptr<smtk::task::PortData>& pdata,
  const smtk::task::ObjectsInRoles::RoleMap& expected)
{
  auto objectsInRoles = std::dynamic_pointer_cast<smtk::task::ObjectsInRoles>(pdata);
  test(!!objectsInRoles, "Expected port data to be ObjectsInRoles.");

  if (objectsInRoles->data() != expected)
  {
    printPortData(pdata);
  }
  test(objectsInRoles->data() == expected, "Port data is unexpected.");
}

} // anonymous namespace

// Test that the port forwarding agent obeys forwarding rules as expected.
int TestPortForwardingAgent(int, char*[])
{
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

  auto attrib = resourceManager->create<smtk::attribute::Resource>();
  auto model = resourceManager->create<smtk::model::Resource>();
  resourceManager->add(attrib);
  resourceManager->add(model);

  attrib->setName("simulation");
  model->setName("geometry");
  auto att0 = attrib->createAttribute(attrib->createDefinition("ic"));
  auto att1 = attrib->createAttribute(attrib->createDefinition("bc"));
  auto mmod = model->addModel(3, 3, "domain");
  auto mfac = model->addFace();
  mfac.setName("boundary");

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

  smtk::task::Task::Ptr task;
  taskManager->taskInstances().visit([&task](const smtk::task::Task::Ptr& tt) {
    if (!task)
    {
      task = tt;
    }
    return smtk::common::Visit::Continue;
  });

  test(!!task, "Failed to find the task.");

  auto* portAttribIn = getTaskPort(task, "attribute objects in");
  auto* portModelIn = getTaskPort(task, "model objects in");
  auto* portAllOut = getTaskPort(task, "all out");
  auto* portResourcesOut = getTaskPort(task, "resources out");
  auto* portComponentsOut = getTaskPort(task, "components out");

  // Test that ports exist.
  test(!!portAttribIn, "Expected an 'attribute objects in' port.");
  test(!!portModelIn, "Expected a 'model objects in' port.");
  test(!!portAllOut, "Expected an 'all out' port.");
  test(!!portResourcesOut, "Expected a 'resources out' port.");
  test(!!portComponentsOut, "Expected a 'components out' port.");

  // Test that output port data is empty initially.
  auto pdata = task->portData(portAllOut);
  test(!pdata, "Expected an empty 'all out' port initially.");
  pdata = task->portData(portResourcesOut);
  test(!pdata, "Expected an empty 'resources out' port initially.");
  pdata = task->portData(portComponentsOut);
  test(!pdata, "Expected an empty 'components out' port initially.");

  // The task state should always be completable for port-forwarding agents.
  test(task->state() == smtk::task::State::Completable, "Expected a completable task.");

  // Add data to input ports and see whether it is produced properly on output ports.
  portAttribIn->connections().insert(attrib.get());
  portAttribIn->connections().insert(att0.get());
  portAttribIn->connections().insert(att1.get());
  portModelIn->connections().insert(model.get());
  portModelIn->connections().insert(mmod.entityRecord().get());
  portModelIn->connections().insert(mfac.entityRecord().get());

  std::cout << "\n-----\n" << portAllOut->describe() << "\n";
  pdata = task->portData(portAllOut);
  checkPortData(
    pdata,
    { { "model", { model.get(), mmod.entityRecord().get(), mfac.entityRecord().get() } },
      { "attribute", { attrib.get(), att0.get(), att1.get() } } });

  std::cout << "\n-----\n" << portResourcesOut->describe() << "\n";
  pdata = task->portData(portResourcesOut);
  checkPortData(pdata, { { "model", { model.get() } }, { "resources", { attrib.get() } } });

  std::cout << "\n-----\n" << portComponentsOut->describe() << "\n";
  pdata = task->portData(portComponentsOut);
  checkPortData(
    pdata,
    { { "model", { mmod.entityRecord().get(), mfac.entityRecord().get() } },
      { "components", { att0.get(), att1.get() } } });

  return 0;
}
