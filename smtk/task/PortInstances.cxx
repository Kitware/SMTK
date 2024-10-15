//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/PortInstances.h"
#include "smtk/task/Manager.h"

namespace smtk
{
namespace task
{

PortInstances::PortInstances(Manager& taskManager)
  : m_taskManager(taskManager)
{
}

Port::Ptr PortInstances::createFromName(const std::string& taskType)
{
  return this->Superclass::createFromName(taskType);
}

Port::Ptr PortInstances::createFromName(
  const std::string& taskType,
  const Port::Configuration& configuration,
  std::shared_ptr<smtk::common::Managers> managers)
{
  return this->Superclass::createFromName(taskType, configuration, m_taskManager, managers);
}

Port::Ptr PortInstances::createFromName(
  const std::string& taskType,
  const Port::Configuration& configuration,
  smtk::task::Task* parentTask,
  std::shared_ptr<smtk::common::Managers> managers)
{
  return this->Superclass::createFromName(taskType, configuration, parentTask, managers);
}

std::set<smtk::task::Port::Ptr> PortInstances::findByName(const std::string& name) const
{
  std::set<smtk::task::Port::Ptr> foundPorts;
  this->visit([&foundPorts, name](const std::shared_ptr<smtk::task::Port>& task) {
    if (task->name() == name)
    {
      foundPorts.insert(task);
    }
    return smtk::common::Visit::Continue;
  });
  return foundPorts;
}

smtk::task::Port::Ptr PortInstances::findById(const smtk::common::UUID& taskId) const
{
  smtk::task::Port::Ptr foundPort;
  this->visit([&foundPort, taskId](const std::shared_ptr<smtk::task::Port>& task) {
    if (task->id() == taskId)
    {
      foundPort = task;
      return smtk::common::Visit::Halt;
    }
    return smtk::common::Visit::Continue;
  });
  return foundPort;
}

} // namespace task
} // namespace smtk
