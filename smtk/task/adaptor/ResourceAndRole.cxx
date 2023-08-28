//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/task/adaptor/ResourceAndRole.h"

#include "smtk/task/FillOutAttributes.h"
#include "smtk/task/GatherResources.h"
#include "smtk/task/Group.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonFillOutAttributes.h"
#include "smtk/task/json/jsonGatherResources.h"

#include "smtk/attribute/Resource.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace task
{
namespace adaptor
{

ResourceAndRole::ResourceAndRole() = default;
ResourceAndRole::ResourceAndRole(const Configuration& config)
{
  this->configureSelf(config);
}

ResourceAndRole::ResourceAndRole(const Configuration& config, Task* from, Task* to)
  : Superclass(config, from, to)
{
  this->configureSelf(config);
}

bool ResourceAndRole::updateDownstreamTask(State upstreamPrev, State upstreamNext)
{
  bool didChange = false;
  if (upstreamPrev >= upstreamNext || upstreamNext < State::Completable)
  {
    return didChange;
  }

  std::map<std::string, std::set<smtk::resource::ResourcePtr>> configs;
  FillOutAttributes* fill = nullptr;
  auto updateFillOutAttributes =
    [&configs, &fill, &didChange](FillOutAttributes::AttributeSet& attributeSet) {
      auto it = configs.find(attributeSet.m_role);
      if (it != configs.end())
      {
        std::set<smtk::common::UUID> unused;
        // Fill "unused" with all resource IDs being tracked.
        for (const auto& entry : attributeSet.m_resources)
        {
          unused.insert(entry.first);
        }
        // Add entries from GatherResource's output and remove from unused.
        for (const auto& resource : it->second)
        {
          auto asit = attributeSet.m_resources.find(resource->id());
          if (asit != attributeSet.m_resources.end())
          {
            unused.erase(resource->id());
          }
          else
          {
            didChange = true;
            asit = attributeSet.m_resources.insert({ resource->id(), { {}, {} } }).first;
            fill->updateResourceEntry(
              *(dynamic_cast<smtk::attribute::Resource*>(resource.get())),
              attributeSet,
              asit->second);
          }
        }
        // Remove unused (no longer relevant resources)
        for (const auto& uid : unused)
        {
          attributeSet.m_resources.erase(uid);
          didChange = true;
        }
      }
      return smtk::common::Visit::Continue;
    };

  if (auto* source = this->from())
  {
    if (auto* gather = dynamic_cast<GatherResources*>(source))
    {
      auto* dest = this->to();
      if ((fill = dynamic_cast<FillOutAttributes*>(dest)))
      {
        gather->visitResourceSets(
          [&configs](const GatherResources::ResourceSet& resourceSet) -> smtk::common::Visit {
            auto& role_config = configs[resourceSet.m_role];
            for (auto const& resource : resourceSet.m_resources)
            {
              auto resource_locked = resource.lock();
              if (resource_locked)
              {
                role_config.insert(resource_locked);
              }
            }
            return smtk::common::Visit::Continue;
          });
        // Look for matching roles in attribute sets and populate with
        // the gathered resources.
        fill->visitAttributeSets(updateFillOutAttributes);
        if (didChange)
        {
          fill->internalStateChanged(fill->computeInternalState());
        }
      }
      else if (auto* group = dynamic_cast<Group*>(dest))
      {
        Task::Configuration configs;
        gather->visitResourceSets(
          [&configs](const GatherResources::ResourceSet& resourceSet) -> smtk::common::Visit {
            nlohmann::json jsonResourceSet = resourceSet;
            configs.push_back(jsonResourceSet);
            return smtk::common::Visit::Continue;
          });
        group->setAdaptorData(m_fromTag, configs);
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "Cannot configure resource and role on a \"" << (dest ? dest->typeName() : "null")
                                                       << "\" task.");
      }
    }
    else if (auto* group = dynamic_cast<Group*>(source))
    {
      auto* dest = this->to();
      if ((fill = dynamic_cast<FillOutAttributes*>(dest)))
      {
        if (group->adaptorData().contains(m_toTag))
        {
          json::Helper::instance().setManagers(group->managers());
          for (const auto& jsonResourceSet : group->adaptorData()[m_toTag])
          {
            auto resourceSet = jsonResourceSet.get<smtk::task::GatherResources::ResourceSet>();
            auto& role_config = configs[resourceSet.m_role];
            for (auto const& weakResource : resourceSet.m_resources)
            {
              auto resource = weakResource.lock();
              if (resource)
              {
                role_config.insert(resource);
              }
            }
          }
        }
        // Look for matching roles in attribute sets and populate with
        // the gathered resources.
        fill->visitAttributeSets(updateFillOutAttributes);
        if (didChange)
        {
          fill->internalStateChanged(fill->computeInternalState());
        }
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "Cannot configure resource and role on a \"" << (dest ? dest->typeName() : "null")
                                                       << ".");
      }
    }
    else
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Cannot adapt resource and role from a \"" << source->typeName() << "\".");
    }
  }
  return didChange;
}

void ResourceAndRole::configureSelf(const Configuration& config)
{
  if (config.contains("from-tag"))
  {
    config.at("from-tag").get_to(m_fromTag);
  }
  if (config.contains("to-tag"))
  {
    config.at("to-tag").get_to(m_toTag);
  }
}

} // namespace adaptor
} // namespace task
} // namespace smtk
