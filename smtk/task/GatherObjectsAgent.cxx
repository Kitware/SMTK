//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/GatherObjectsAgent.h"

#include "smtk/task/Port.h"
#include "smtk/task/Task.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "smtk/common/Managers.h"

#include "smtk/common/json/jsonUUID.h"
#include "smtk/string/json/jsonToken.h"

namespace smtk
{
namespace task
{

GatherObjectsAgent::GatherObjectsAgent(Task* owningTask)
  : Agent(owningTask)
{
}

State GatherObjectsAgent::state() const
{
  return State::Completable;
}

void GatherObjectsAgent::configure(const Configuration& config)
{
  this->Superclass::configure(config);
  if (config.contains("output-port"))
  {
    m_outputPortName = config.at("output-port").get<smtk::string::Token>();
  }

  // Attempt to deserialize output port data from a previous session.
  // All resources mentioned by the operation must be present for
  // this to work, but the project owning this agent's task may not
  // be registered yet.
  if (config.contains("objects"))
  {
    m_objects = config.at("objects").get<DataMap>();
  }
}

GatherObjectsAgent::Configuration GatherObjectsAgent::configuration() const
{
  Configuration config = this->Superclass::configuration();
  config["output-port"] = m_outputPortName;
  Configuration outputs = m_objects;
  if (!outputs.empty())
  {
    config["objects"] = outputs;
  }
  return config;
}

std::shared_ptr<PortData> GatherObjectsAgent::portData(const Port* port) const
{
  if (!m_outputPortName.valid() || m_objects.empty() || !port || port->name() != m_outputPortName)
  {
    return nullptr;
  }

  auto data = std::make_shared<smtk::task::ObjectsInRoles>();
  auto* parentRsrc = m_parent ? m_parent->parentResource() : nullptr;
  if (!parentRsrc)
  {
    return nullptr;
  }
  auto rsrcMgr = parentRsrc->manager();
  if (!rsrcMgr)
  {
    return nullptr;
  }

  for (const auto& roleSetEntry : m_objects)
  {
    auto role = roleSetEntry.first;
    for (const auto& rsrcEntry : roleSetEntry.second)
    {
      auto rsrcId = rsrcEntry.first;
      auto* rsrc = rsrcMgr->get(rsrcId).get();
      if (!rsrc && parentRsrc->id() == rsrcId)
      {
        rsrc = parentRsrc;
      }
      if (!rsrc)
      {
        continue;
      } // No resource, so we cannot find any components

      for (const auto& compId : rsrcEntry.second)
      {
        smtk::resource::PersistentObject* obj;
        if (compId)
        {
          obj = rsrc->find(compId).get();
        }
        else
        {
          obj = rsrc;
        }
        if (obj)
        {
          data->addObject(obj, role);
        }
      }
    }
  }

  return data;
}

bool GatherObjectsAgent::addObjectInRole(
  smtk::resource::PersistentObject* object,
  smtk::string::Token role,
  bool signal)
{
  if (!object || !role.valid())
  {
    return false;
  }
  auto* rsrc = dynamic_cast<smtk::resource::Resource*>(object);
  auto* comp = dynamic_cast<smtk::resource::Component*>(object);
  if (comp && !rsrc)
  {
    rsrc = comp->parentResource();
  }
  if (!rsrc)
  {
    return false;
  }

  // Verify that this is not a duplicate (which we will not insert).
  auto rit = m_objects.find(role);
  if (rit != m_objects.end())
  {
    auto u1it = rit->second.find(rsrc->id());
    if (u1it != rit->second.end())
    {
      auto u2it = u1it->second.find(comp ? comp->id() : smtk::common::UUID::null());
      if (u2it != u1it->second.end())
      {
        return false;
      }
    }
  }
  m_objects[role][rsrc->id()].insert(comp ? comp->id() : smtk::common::UUID::null());
  if (signal)
  {
    auto it = m_parent->ports().find(m_outputPortName);
    if (it != m_parent->ports().end())
    {
      m_parent->portDataUpdated(it->second);
    }
  }
  return true;
}

bool GatherObjectsAgent::removeObjectFromRole(
  smtk::resource::PersistentObject* object,
  smtk::string::Token role,
  bool signal)
{
  if (!object || !role.valid())
  {
    return false;
  }
  auto* rsrc = dynamic_cast<smtk::resource::Resource*>(object);
  auto* comp = dynamic_cast<smtk::resource::Component*>(object);
  if (comp && !rsrc)
  {
    rsrc = comp->parentResource();
  }
  if (!rsrc)
  {
    return false;
  }

  auto rit = m_objects.find(role);
  if (rit != m_objects.end())
  {
    auto u1it = rit->second.find(rsrc->id());
    if (u1it != rit->second.end())
    {
      auto u2it = u1it->second.find(comp ? comp->id() : smtk::common::UUID::null());
      if (u2it != u1it->second.end())
      {
        // Object was present, remove it.
        u1it->second.erase(u2it);
        if (u1it->second.empty())
        {
          // If the resource has no more objects, remove its entry.
          rit->second.erase(u1it);
          if (rit->second.empty())
          {
            // If the role has no more children, remove its entry.
            m_objects.erase(rit);
          }
        }
        if (signal)
        {
          auto it = m_parent->ports().find(m_outputPortName);
          if (it != m_parent->ports().end())
          {
            m_parent->portDataUpdated(it->second);
          }
        }
        return true;
      }
    }
  }
  return false;
}

bool GatherObjectsAgent::clearOutputPort(bool signal)
{
  bool wasNotEmpty = !m_objects.empty();
  m_objects.clear();
  if (signal && wasNotEmpty)
  {
    auto it = m_parent->ports().find(m_outputPortName);
    if (it != m_parent->ports().end())
    {
      m_parent->portDataUpdated(it->second);
    }
  }
  return wasNotEmpty;
}

Port* GatherObjectsAgent::outputPort() const
{
  if (!m_outputPortName.valid())
  {
    return nullptr;
  }
  auto it = m_parent->ports().find(m_outputPortName);
  if (it != m_parent->ports().end())
  {
    return it->second;
  }
  return nullptr;
}

} // namespace task
} // namespace smtk
