//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/task/TaskNeedsResources.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"
#include "smtk/project/ResourceContainer.h"

#include <stdexcept>

namespace smtk
{
namespace task
{

void to_json(json& j, const TaskNeedsResources::Predicate& p)
{
  j = json{ { "role", p.m_role }, { "type", p.m_type } };
  if (p.m_minimumCount == 0 && p.m_maximumCount < 0)
  {
    // skip counts; any number of resources are allowed.
  }
  else
  {
    j["min"] = p.m_minimumCount;
    j["max"] = p.m_maximumCount;
  }
  if (p.m_validator)
  {
    j["validator"] = "Cannot serialize validators yet.";
  }
}

void from_json(const json& j, TaskNeedsResources::Predicate& p)
{
  if (j.contains("role"))
  {
    j.at("role").get_to(p.m_role);
  }
  if (j.contains("type"))
  {
    j.at("type").get_to(p.m_type);
  }
  if (j.contains("min"))
  {
    j.at("min").get_to(p.m_minimumCount);
  }
  else
  {
    p.m_minimumCount = 1;
  }
  if (j.contains("max"))
  {
    j.at("max").get_to(p.m_maximumCount);
  }
  else
  {
    p.m_maximumCount = -1;
  }
  if (j.contains("validator"))
  {
    throw std::logic_error("todo"); // TODO
  }
  else
  {
    // Accept any resource
    p.m_validator = nullptr;
    /*
      [](const smtk::resource::Resource&, const TaskNeedsResource::Predicate&)
      { return true; };
      */
  }
}

TaskNeedsResources::TaskNeedsResources() = default;

TaskNeedsResources::TaskNeedsResources(
  const Configuration& config,
  const smtk::common::Managers::Ptr& managers)
  : Task(config, managers)
  , m_managers(managers)
{
  this->configure(config);
}

TaskNeedsResources::TaskNeedsResources(
  const Configuration& config,
  const PassedDependencies& dependencies,
  const smtk::common::Managers::Ptr& managers)
  : Task(config, dependencies, managers)
  , m_managers(managers)
{
  this->configure(config);
}

void TaskNeedsResources::configure(const Configuration& config)
{
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

        Predicate predicate;
        spec.get_to(predicate);
        m_resourcesByRole[predicate.m_role] = spec.get<Predicate>();
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
        "TaskNeedsResources monitors results for resources and their roles.");
    }
  }
  if (!m_resourcesByRole.empty())
  {
    this->internalStateChanged(this->computeInternalState());
  }
}

void TaskNeedsResources::updateResources(
  smtk::resource::Resource& resource,
  smtk::resource::EventType event)
{
  bool predicatesUpdated = false;
  auto resourcePtr = resource.shared_from_this();
  switch (event)
  {
    case smtk::resource::EventType::ADDED:
    {
      // Add the resource to the appropriate entry:
      const std::string& role = smtk::project::detail::role(resourcePtr);
      auto it = m_resourcesByRole.find(role);
      if (it != m_resourcesByRole.end())
      {
        if (!it->second.m_validator || it->second.m_validator(resource, it->second))
        {
          it->second.m_resources.insert(resourcePtr);
          predicatesUpdated = true;
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
        predicatesUpdated = it->second.m_resources.erase(resourcePtr) > 0;
      }
    }
    break;
    case smtk::resource::EventType::MODIFIED:
      // TODO
      break;
  }
  if (predicatesUpdated)
  {
    this->internalStateChanged(this->computeInternalState());
  }
}

State TaskNeedsResources::computeInternalState() const
{
  State s = State::Completable;
  for (const auto& entry : m_resourcesByRole)
  {
    const auto& predicate(entry.second);
    if (predicate.m_resources.size() < static_cast<std::size_t>(predicate.m_minimumCount))
    {
      s = State::Incomplete;
    }
    else if (
      predicate.m_maximumCount >= 0 &&
      predicate.m_resources.size() > static_cast<std::size_t>(predicate.m_maximumCount))
    {
      s = State::Incomplete;
    }
  }
  return s;
}

} // namespace task
} // namespace smtk
