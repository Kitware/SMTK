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
#include "smtk/task/GatherResources.h"
#include "smtk/task/Group.h"
#include "smtk/task/Task.h"
#include "smtk/task/adaptor/ResourceAndRole.h"
#include "smtk/task/json/Configurator.h"
#include "smtk/task/json/Configurator.txx"
#include "smtk/task/json/jsonAdaptor.h"
#include "smtk/task/json/jsonFillOutAttributes.h"
#include "smtk/task/json/jsonGatherResources.h"
#include "smtk/task/json/jsonGroup.h"
#include "smtk/task/json/jsonResourceAndRole.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/plugin/Manager.h"

#include <tuple>

namespace smtk
{
namespace task
{

using TaskList = std::tuple<Task, FillOutAttributes, GatherResources, Group>;
using TaskJSON = std::
  tuple<json::jsonTask, json::jsonFillOutAttributes, json::jsonGatherResources, json::jsonGroup>;
using AdaptorList = std::tuple<adaptor::ResourceAndRole>;
using AdaptorJSON = std::tuple<json::jsonResourceAndRole>;

void Registrar::registerTo(const smtk::task::Manager::Ptr& taskManager)
{
  auto& taskInstances = taskManager->taskInstances();
  taskInstances.registerTypes<TaskList>();
  json::Configurator<Task>::registerTypes<TaskList, TaskJSON>();

  auto& adaptorInstances = taskManager->adaptorInstances();
  adaptorInstances.registerTypes<AdaptorList>();
  json::Configurator<Adaptor>::registerTypes<AdaptorList, AdaptorJSON>();
}

void Registrar::unregisterFrom(const smtk::task::Manager::Ptr& taskManager)
{
  auto& taskInstances = taskManager->taskInstances();
  taskInstances.unregisterTypes<TaskList>();
  json::Configurator<Task>::unregisterTypes<TaskList>();

  auto& adaptorInstances = taskManager->adaptorInstances();
  adaptorInstances.unregisterTypes<AdaptorList>();
  json::Configurator<Adaptor>::unregisterTypes<AdaptorList>();
}

} // namespace task
} // namespace smtk
