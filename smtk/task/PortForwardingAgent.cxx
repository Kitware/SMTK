//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/PortForwardingAgent.h"

#include "smtk/task/ObjectsInRoles.h"
#include "smtk/task/Port.h"
#include "smtk/task/Task.h"
#include "smtk/task/json/jsonPortForwardingAgent.h"

#include "smtk/string/json/jsonToken.h"

#include "smtk/io/Logger.h"

using namespace smtk::string::literals;

namespace smtk
{
namespace task
{
namespace
{

bool filterMatches(
  const PortForwardingAgent::ObjectFilter& filter,
  const smtk::resource::PersistentObject* object)
{
  if (!object)
  {
    return false;
  }
  if (const auto* comp = dynamic_cast<const smtk::resource::Component*>(object))
  {
    // Do not match resource-only filters.
    if (filter.m_componentFilter.empty())
    {
      return false;
    }

    const auto* rsrc = comp->parentResource();
    if (!rsrc)
    {
      return false;
    }

    if (rsrc->matchesType(filter.m_resourceFilter))
    {
      auto test = rsrc->queryOperation(filter.m_componentFilter);
      return test && test(*comp);
    }
  }
  else if (const auto* rsrc = dynamic_cast<const smtk::resource::Resource*>(object))
  {
    if (
      filter.m_componentFilter.empty() &&
      (filter.m_resourceFilter == "*" || rsrc->matchesType(filter.m_resourceFilter)))
    {
      return true;
    }
  }
  return false;
}

bool filtersMatch(
  const std::vector<PortForwardingAgent::ObjectFilter>& filters,
  const smtk::resource::PersistentObject* object)
{
  return std::any_of(
    filters.begin(), filters.end(), [&](const PortForwardingAgent::ObjectFilter& filter) {
      return filterMatches(filter, object);
    });
}

std::shared_ptr<smtk::task::ObjectsInRoles> transformPortData(
  const std::shared_ptr<smtk::task::ObjectsInRoles>& roleData,
  const PortForwardingAgent::RolesToFilters& filters,
  smtk::string::Token outputRole)
{
  auto transformed = std::make_shared<ObjectsInRoles>();
  for (const auto& entry : roleData->data())
  {
    smtk::string::Token role = outputRole.valid() ? outputRole : entry.first;
    auto roleMatchIt = filters.find(entry.first);
    auto wildcardMatchIt = filters.find("*"_token);
    for (const auto& object : entry.second)
    {
      if (
        (roleMatchIt != filters.end() && filtersMatch(roleMatchIt->second, object)) ||
        (wildcardMatchIt != filters.end() && filtersMatch(wildcardMatchIt->second, object)))
      {
        transformed->addObject(object, role);
      }
    }
  }
  return transformed;
}

} // anonymous namespace

PortForwardingAgent::PortForwardingAgent(Task* owningTask)
  : Agent(owningTask)
{
}

State PortForwardingAgent::state() const
{
  return State::Completable;
}

void PortForwardingAgent::configure(const Configuration& config)
{
  this->Superclass::configure(config);

  m_forwards.clear();
  auto it = config.find("forwards");
  if (it == config.end() || !it->is_array())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Missing/invalid port configuration. Agent will do nothing.");
    return;
  }
  for (const auto& spec : *it)
  {
    auto iit = spec.find("input-port");
    auto oit = spec.find("output-port");
    auto fit = spec.find("filters");
    auto rit = spec.find("output-role");
    if (iit == spec.end() || oit == spec.end())
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Forwarding rule \"" << spec.dump()
                             << "\" must "
                                "have both an input and output port named.");
      continue;
    }
    const auto& ports(m_parent->ports());
    auto ipit = ports.find(iit->get<smtk::string::Token>());
    auto opit = ports.find(oit->get<smtk::string::Token>());
    if (ipit == ports.end() || opit == ports.end())
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Input (" << iit->get<std::string>()
                  << ") or "
                     "output ("
                  << oit->get<std::string>() << ") ports must both exist.");
      continue;
    }
    Forward fwd;
    fwd.m_inputPort = ipit->second;
    fwd.m_outputPort = opit->second;
    if (fit != spec.end() && fit->is_object())
    {
      fwd.m_filters.reserve(fit->size());
      for (const auto& jRoleFilters : fit->items())
      {
        std::vector<PortForwardingAgent::ObjectFilter> filters = jRoleFilters.value();
        fwd.m_filters[jRoleFilters.key()] = filters;
      }
    }
    if (rit != spec.end())
    {
      fwd.m_outputRole = rit->get<smtk::string::Token>();
    }
    m_forwards.push_back(fwd);
  }
}

PortForwardingAgent::Configuration PortForwardingAgent::configuration() const
{
  auto cfg = this->Superclass::configuration();
  nlohmann::json::array_t forwards;
  forwards.reserve(m_forwards.size());
  for (const auto& rule : m_forwards)
  {
    nlohmann::json jRule = { { "input-port", rule.m_inputPort->name() },
                             { "output-port", rule.m_outputPort->name() } };
    nlohmann::json jRoleFilters;
    for (const auto& roleToFilters : rule.m_filters)
    {
      nlohmann::json::array_t jFilters;
      jFilters.reserve(roleToFilters.second.size());
      for (const auto& filter : roleToFilters.second)
      {
        jFilters.push_back(
          { { "resource", filter.m_resourceFilter }, { "component", filter.m_componentFilter } });
      }
      jRoleFilters[roleToFilters.first.data()] = jFilters;
    }
    if (!jRoleFilters.empty())
    {
      jRule["filters"] = jRoleFilters;
    }
    forwards.push_back(jRule);
  }
  if (!forwards.empty())
  {
    cfg["forwards"] = forwards;
  }
  return cfg;
}

std::shared_ptr<PortData> PortForwardingAgent::portData(const Port* port) const
{
  std::shared_ptr<PortData> result;
  for (const auto& forward : m_forwards)
  {
    if (port == forward.m_outputPort)
    {
      // Copy port data from the input port as directed.
      auto data = forward.m_inputPort->parent()->portData(forward.m_inputPort);
      if (data)
      {
        // If the data is formatted as ObjectsInRoles, filter it using m_filters
        // and m_outputRole. Otherwise, merge/pass it without modification.
        if (auto roleData = std::dynamic_pointer_cast<smtk::task::ObjectsInRoles>(data))
        {
          if (!forward.m_filters.empty() || forward.m_outputRole.valid())
          {
            data = transformPortData(roleData, forward.m_filters, forward.m_outputRole);
          }
        }

        if (result)
        {
          result->merge(data.get());
        }
        else
        {
          result = data;
        }
      }
    }
  }
  return result;
}

void PortForwardingAgent::portDataUpdated(const Port* port)
{
  // Collect a list of output ports whose data needs to be updated
  // because \a port is attached from upstream via this agent.
  std::unordered_set<Port*> downstreamPorts;
  for (const auto& forward : m_forwards)
  {
    if (forward.m_inputPort == port)
    {
      downstreamPorts.insert(forward.m_outputPort);
    }
  }
  // Notify connections on downstream ports once (for this input \a port):
  for (const auto& downstreamPort : downstreamPorts)
  {
    this->parent()->portDataUpdated(downstreamPort);
  }
}

} // namespace task
} // namespace smtk
