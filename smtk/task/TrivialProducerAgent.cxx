//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/TrivialProducerAgent.h"

#include "smtk/task/ObjectsInRoles.h"
#include "smtk/task/json/Helper.h"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace task
{

TrivialProducerAgent::TrivialProducerAgent(Task* owningTask)
  : Agent(owningTask)
  , m_data(new ObjectsInRoles)
{
}

State TrivialProducerAgent::state() const
{
  return State::Completable;
}

void TrivialProducerAgent::configure(const Configuration& config)
{
  bool addedData = false;
  auto& helper(smtk::task::json::Helper::instance());
  auto it = config.find("data");
  if (it != config.end() && it->is_object())
  {
    for (const auto& entry : it->items())
    {
      smtk::string::Token role(entry.key());
      if (entry.value().is_array())
      {
        for (const auto& objectSpec : entry.value())
        {
          auto* obj = helper.objectFromJSONSpec(objectSpec, "port");
          if (obj)
          {
            addedData |= m_data->addObject(obj, role);
          }
        }
      }
    }
  }
  it = config.find("name");
  if (it != config.end())
  {
    m_name = it->get<std::string>();
  }
  it = config.find("output-port");
  if (it != config.end())
  {
    auto pit = m_parent->ports().find(it->get<smtk::string::Token>());
    if (pit != m_parent->ports().end())
    {
      m_outputPort = pit->second;
    }
  }
  if (addedData && m_outputPort)
  {
    this->portDataUpdated(m_outputPort);
  }
}

std::shared_ptr<PortData> TrivialProducerAgent::portData(const Port* port) const
{
  if (port == m_outputPort)
  {
    return m_data;
  }
  return nullptr;
}

bool TrivialProducerAgent::addObjectInRole(
  Task* task,
  const std::string& agentName,
  smtk::string::Token role,
  smtk::resource::PersistentObject* object)
{
  if (!task || agentName.empty() || !object)
  {
    return false;
  }
  for (const auto& agent : task->agents())
  {
    if (auto* trivialProducer = dynamic_cast<TrivialProducerAgent*>(agent))
    {
      if (trivialProducer->name() == agentName)
      {
        return trivialProducer->m_data->addObject(object, role);
      }
    }
  }
  return false;
}

bool TrivialProducerAgent::addObjectInRole(
  Task* task,
  Port* port,
  smtk::string::Token role,
  smtk::resource::PersistentObject* object)
{
  if (!task || !port || !object)
  {
    return false;
  }
  for (const auto& agent : task->agents())
  {
    if (auto* trivialProducer = dynamic_cast<TrivialProducerAgent*>(agent))
    {
      if (trivialProducer->outputPort() == port)
      {
        return trivialProducer->m_data->addObject(object, role);
      }
    }
  }
  return false;
}

bool TrivialProducerAgent::removeObjectFromRole(
  Task* task,
  const std::string& agentName,
  smtk::string::Token role,
  smtk::resource::PersistentObject* object)
{
  if (!task || agentName.empty() || !object)
  {
    return false;
  }
  for (const auto& agent : task->agents())
  {
    if (auto* trivialProducer = dynamic_cast<TrivialProducerAgent*>(agent))
    {
      if (trivialProducer->name() == agentName)
      {
        if (trivialProducer->m_data->removeObject(object, role))
        {
          return true;
        }
      }
    }
  }
  return false;
}

bool TrivialProducerAgent::removeObjectFromRole(
  Task* task,
  Port* port,
  smtk::string::Token role,
  smtk::resource::PersistentObject* object)
{
  if (!task || !port || !object)
  {
    return false;
  }
  for (const auto& agent : task->agents())
  {
    if (auto* trivialProducer = dynamic_cast<TrivialProducerAgent*>(agent))
    {
      if (trivialProducer->outputPort() == port)
      {
        if (trivialProducer->m_data->removeObject(object, role))
        {
          return true;
        }
      }
    }
  }
  return false;
}

bool TrivialProducerAgent::resetData(Task* task, const std::string& agentName)
{
  bool didChange = false;
  if (!task || agentName.empty())
  {
    return didChange;
  }
  for (const auto& agent : task->agents())
  {
    if (auto* trivialProducer = dynamic_cast<TrivialProducerAgent*>(agent))
    {
      if (trivialProducer->name() == agentName)
      {
        didChange |= trivialProducer->m_data->clear();
      }
    }
  }
  return didChange;
}

bool TrivialProducerAgent::resetData(Task* task, Port* port)
{
  bool didChange = false;
  if (!task || !port)
  {
    return didChange;
  }
  for (const auto& agent : task->agents())
  {
    if (auto* trivialProducer = dynamic_cast<TrivialProducerAgent*>(agent))
    {
      didChange |= trivialProducer->m_data->clear();
    }
  }
  return didChange;
}

} // namespace task
} // namespace smtk
