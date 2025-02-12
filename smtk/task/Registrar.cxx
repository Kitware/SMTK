//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/task/Registrar.h"

#include "smtk/task/Adaptor.h"
#include "smtk/task/FillOutAttributes.h"
#include "smtk/task/FillOutAttributesAgent.h"
#include "smtk/task/GatherObjectsAgent.h"
#include "smtk/task/GatherResources.h"
#include "smtk/task/Port.h"
#include "smtk/task/PortForwardingAgent.h"
#include "smtk/task/SubmitOperation.h"
#include "smtk/task/SubmitOperationAgent.h"
#include "smtk/task/Task.h"
#include "smtk/task/TrivialProducerAgent.h"
#include "smtk/task/adaptor/ConfigureOperation.h"
#include "smtk/task/adaptor/ResourceAndRole.h"
#include "smtk/task/json/Configurator.h"
#include "smtk/task/json/Configurator.txx"
#include "smtk/task/json/jsonAdaptor.h"
#include "smtk/task/json/jsonConfigureOperation.h"
#include "smtk/task/json/jsonFillOutAttributes.h"
#include "smtk/task/json/jsonGatherResources.h"
#include "smtk/task/json/jsonPort.h"
#include "smtk/task/json/jsonResourceAndRole.h"
#include "smtk/task/json/jsonSubmitOperation.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/task/operators/AddDependency.h"
#include "smtk/task/operators/ConnectPorts.h"
#include "smtk/task/operators/DisconnectPorts.h"
#include "smtk/task/operators/EmplaceWorklet.h"
#include "smtk/task/operators/RemoveDependency.h"
#include "smtk/task/operators/RenameTask.h"

#include "smtk/operation/groups/ArcCreator.h"
#include "smtk/operation/groups/ArcDeleter.h"

#include "smtk/plugin/Manager.h"

#include <tuple>

namespace smtk
{
namespace task
{

using TaskList = std::tuple<Task, FillOutAttributes, GatherResources, SubmitOperation>;
using TaskJSON = std::tuple<
  json::jsonTask,
  json::jsonFillOutAttributes,
  json::jsonGatherResources,
  json::jsonSubmitOperation>;

using AdaptorList = std::tuple<adaptor::ConfigureOperation, adaptor::ResourceAndRole>;
using AdaptorJSON = std::tuple<json::jsonConfigureOperation, json::jsonResourceAndRole>;

using PortList = std::tuple<Port>;
using PortJSON = std::tuple<json::jsonPort>;

using AgentList = std::tuple<
  FillOutAttributesAgent,
  GatherObjectsAgent,
  PortForwardingAgent,
  SubmitOperationAgent,
  TrivialProducerAgent>;

using OperationList = std::
  tuple<AddDependency, ConnectPorts, DisconnectPorts, EmplaceWorklet, RemoveDependency, RenameTask>;

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  auto& typeLabels = resourceManager->objectTypeLabels();
  // clang-format off
  // Tasks
  typeLabels[smtk::common::typeName<smtk::task::Task>()] = "task";
  typeLabels[smtk::common::typeName<smtk::task::FillOutAttributes>()] = "fill out attributes task";
  typeLabels[smtk::common::typeName<smtk::task::GatherResources>()] = "gather resources task";
  typeLabels[smtk::common::typeName<smtk::task::SubmitOperation>()] = "operation task";

  // Adaptors
  typeLabels[smtk::common::typeName<smtk::task::adaptor::ConfigureOperation>()] = "operation adaptor";
  typeLabels[smtk::common::typeName<smtk::task::adaptor::ResourceAndRole>()] = "resource and role adaptor";

  // Ports
  typeLabels[smtk::common::typeName<smtk::task::Port>()] = "port";
  // clang-format on
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  auto& typeLabels = resourceManager->objectTypeLabels();
  typeLabels.erase(smtk::common::typeName<smtk::task::Task>());
  typeLabels.erase(smtk::common::typeName<smtk::task::FillOutAttributes>());
  typeLabels.erase(smtk::common::typeName<smtk::task::GatherResources>());
  typeLabels.erase(smtk::common::typeName<smtk::task::SubmitOperation>());
  typeLabels.erase(smtk::common::typeName<smtk::task::adaptor::ConfigureOperation>());
  typeLabels.erase(smtk::common::typeName<smtk::task::adaptor::ResourceAndRole>());
}

void Registrar::registerTo(const smtk::task::Manager::Ptr& taskManager)
{
  auto& taskInstances = taskManager->taskInstances();
  taskInstances.registerTypes<TaskList>();
  json::Configurator<Task>::registerTypes<TaskList, TaskJSON>();

  auto& adaptorInstances = taskManager->adaptorInstances();
  adaptorInstances.registerTypes<AdaptorList>();
  json::Configurator<Adaptor>::registerTypes<AdaptorList, AdaptorJSON>();

  auto& portInstances = taskManager->portInstances();
  portInstances.registerTypes<PortList>();
  json::Configurator<Port>::registerTypes<PortList, PortJSON>();

  auto& agentFactory = taskManager->agentFactory();
  agentFactory.registerTypes<AgentList>();
}

void Registrar::unregisterFrom(const smtk::task::Manager::Ptr& taskManager)
{
  auto& taskInstances = taskManager->taskInstances();
  taskInstances.unregisterTypes<TaskList>();
  json::Configurator<Task>::unregisterTypes<TaskList>();

  auto& adaptorInstances = taskManager->adaptorInstances();
  adaptorInstances.unregisterTypes<AdaptorList>();
  json::Configurator<Adaptor>::unregisterTypes<AdaptorList>();

  auto& portInstances = taskManager->portInstances();
  portInstances.unregisterTypes<PortList>();
  json::Configurator<Port>::unregisterTypes<PortList>();

  auto& agentFactory = taskManager->agentFactory();
  agentFactory.unregisterTypes<AgentList>();
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperations<OperationList>();

  smtk::operation::ArcCreator arcCreator(operationManager);
  arcCreator.registerOperation<smtk::task::AddDependency>({ "task dependency" });
  arcCreator.registerOperation<smtk::task::ConnectPorts>({ "port connection" });
  smtk::operation::ArcDeleter arcDeleter(operationManager);
  arcDeleter.registerOperation<smtk::task::DisconnectPorts>();
  arcDeleter.registerOperation<smtk::task::RemoveDependency>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->unregisterOperations<OperationList>();

  smtk::operation::ArcCreator arcCreator(operationManager);
  arcCreator.unregisterOperation<smtk::task::AddDependency>();
  arcCreator.unregisterOperation<smtk::task::ConnectPorts>();
  smtk::operation::ArcDeleter arcDeleter(operationManager);
  arcDeleter.unregisterOperation<smtk::task::DisconnectPorts>();
  arcDeleter.unregisterOperation<smtk::task::RemoveDependency>();
}

} // namespace task
} // namespace smtk
