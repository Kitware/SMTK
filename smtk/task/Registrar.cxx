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
#include "smtk/task/Task.h"
#include "smtk/task/TaskNeedsResources.h"

#include "smtk/plugin/Manager.h"

#include <tuple>

namespace smtk
{
namespace task
{

using TaskList = std::tuple<Task, TaskNeedsResources>;

void Registrar::registerTo(const smtk::common::Managers::Ptr& managers)
{
  if (managers->insert(smtk::task::Manager::create()))
  {
    managers->get<smtk::task::Manager::Ptr>()->setManagers(managers);

    smtk::plugin::Manager::instance()->registerPluginsTo(managers->get<smtk::task::Manager::Ptr>());
  }
}

void Registrar::unregisterFrom(const smtk::common::Managers::Ptr& managers)
{
  managers->erase<smtk::task::Manager::Ptr>();
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  (void)resourceManager;
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  (void)resourceManager;
}

void Registrar::registerTo(const smtk::task::Manager::Ptr& taskManager)
{
  auto& instances = taskManager->instances();
  instances.registerTypes<TaskList>();
}

void Registrar::unregisterFrom(const smtk::task::Manager::Ptr& taskManager)
{
  auto& instances = taskManager->instances();
  instances.unregisterTypes<TaskList>();
}

} // namespace task
} // namespace smtk
