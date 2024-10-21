//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/task/GatherResources.h"

#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonGatherResources.h"

#include "smtk/project/ResourceContainer.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"

#include "smtk/io/Logger.h"

#include <stdexcept>

namespace smtk
{
namespace task
{

constexpr const char* const GatherResources::type_name;

GatherResources::GatherResources() = default;

GatherResources::GatherResources(
  const Configuration& config,
  Manager& taskManager,
  const smtk::common::Managers::Ptr& managers)
  : AgentlessTask(config, taskManager, managers)
  , m_managers(managers)
{
  this->configure(config);
}

GatherResources::GatherResources(
  const Configuration& config,
  const PassedDependencies& dependencies,
  Manager& taskManager,
  const smtk::common::Managers::Ptr& managers)
  : AgentlessTask(config, dependencies, taskManager, managers)
  , m_managers(managers)
{
  this->configure(config);
}

void GatherResources::configure(const Configuration& config)
{
  if (m_managers)
  {
    json::Helper::instance().setManagers(m_managers);
  }
  m_autoconfigure =
    (config.contains("auto-configure") ? config.at("auto-configure").get<bool>() : false);
  if (config.contains("resources"))
  {
    auto rsrcSpecs = config.at("resources");
    if (rsrcSpecs.is_array())
    {
      for (const auto& spec : rsrcSpecs)
      {
        if (!spec.contains("role"))
        {
          continue;
        }

        ResourceSet resourceSet;
        spec.get_to(resourceSet);
        m_resourcesByRole[resourceSet.m_role] = spec.get<ResourceSet>();
      }
    }
  }
  if (m_managers)
  {
    if (auto resourceManager = m_managers->get<smtk::resource::Manager::Ptr>())
    {
      m_observer = resourceManager->observers().insert(
        [this](const smtk::resource::Resource& resource, smtk::resource::EventType event) {
          this->updateResources(const_cast<smtk::resource::Resource&>(resource), event);
        },
        /* priority */ 0,
        /* initialize */ true,
        "GatherResources monitors resources and their roles.");
    }
  }
  if (!m_resourcesByRole.empty())
  {
    this->internalStateChanged(this->computeInternalState());
  }
}

bool GatherResources::addResourceInRole(
  const std::shared_ptr<smtk::resource::Resource>& resource,
  const std::string& role)
{
  if (!resource)
  {
    return false;
  }
  auto it = m_resourcesByRole.find(role);
  if (it == m_resourcesByRole.end())
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "GatherResources is not configured for the role \"" << role << "\".");
    return false;
  }
  if (it->second.m_type == "*" || resource->isOfType(it->second.m_type))
  {
    if (it->second.m_resources.insert(resource).second)
    {
      // TODO: Should we set the role:
      //   resource->properties().get<std::string>()[smtk::project::ResourceContainer::role_name] = role
      // so that a resource-manager event that drops the resource removes
      // it from the proper ResourceSet? Otherwise, it can be dropped but
      // we won't transition from completable to incomplete if we need to...
      this->internalStateChanged(this->computeInternalState());
      return true;
    }
  }
  return false;
}

smtk::common::Visit GatherResources::visitResourceSets(ResourceSetVisitor visitor)
{
  if (!visitor)
  {
    return smtk::common::Visit::Halt;
  }
  for (const auto& entry : m_resourcesByRole)
  {
    if (visitor(entry.second) == smtk::common::Visit::Halt)
    {
      return smtk::common::Visit::Halt;
    }
  }
  return smtk::common::Visit::Continue;
}

void GatherResources::updateResources(
  smtk::resource::Resource& resource,
  smtk::resource::EventType event)
{
  bool resourceSetsUpdated = false;
  auto resourcePtr = resource.shared_from_this();
  switch (event)
  {
    case smtk::resource::EventType::ADDED:
    {
      if (m_autoconfigure)
      {
        // Add the resource to the appropriate entry:
        const std::string& role = smtk::project::detail::role(resourcePtr);
        auto it = m_resourcesByRole.find(role);
        if (it != m_resourcesByRole.end())
        {
          if (!it->second.m_validator || it->second.m_validator(resource, it->second))
          {
            it->second.m_resources.insert(resourcePtr);
            resourceSetsUpdated = true;
          }
        }
      }
    }
    break;
    case smtk::resource::EventType::REMOVED:
    {
      // Remove the resource from the appropriate entry.
      const std::string& role = smtk::project::detail::role(resourcePtr);
      auto it = m_resourcesByRole.find(role);
      if (it != m_resourcesByRole.end())
      {
        resourceSetsUpdated = it->second.m_resources.erase(resourcePtr) > 0;
      }
    }
    break;
    default:
      break;
  }
  if (resourceSetsUpdated)
  {
    this->internalStateChanged(this->computeInternalState());
  }
}

State GatherResources::computeInternalState() const
{
  State s = State::Completable;
  for (const auto& entry : m_resourcesByRole)
  {
    const auto& resourceSet(entry.second);
    if (resourceSet.m_resources.size() < static_cast<std::size_t>(resourceSet.m_minimumCount))
    {
      s = State::Incomplete;
    }
    else if (
      resourceSet.m_maximumCount >= 0 &&
      resourceSet.m_resources.size() > static_cast<std::size_t>(resourceSet.m_maximumCount))
    {
      s = State::Incomplete;
    }
  }
  return s;
}

} // namespace task
} // namespace smtk
