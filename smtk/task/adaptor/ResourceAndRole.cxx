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

ResourceAndRole::ResourceAndRole(
  const Configuration& config,
  std::shared_ptr<Task>& from,
  std::shared_ptr<Task>& to)
  : Superclass(config, from, to)
{
  this->configureSelf(config);
}

bool ResourceAndRole::reconfigureTask()
{
  bool didChange = false;
  if (auto source = this->from())
  {
    if (auto gather = std::dynamic_pointer_cast<GatherResources>(source))
    {
      auto dest = this->to();
      std::map<std::string, std::set<smtk::resource::ResourcePtr>> configs;
      gather->visitResourceSets(
        [&configs](const GatherResources::ResourceSet& resourceSet) -> smtk::common::Visit {
          configs[resourceSet.m_role].insert(
            resourceSet.m_resources.begin(), resourceSet.m_resources.end());
          return smtk::common::Visit::Continue;
        });
      if (auto fill = std::dynamic_pointer_cast<FillOutAttributes>(dest))
      {
        // Look for matching roles in attribute sets and populate with
        // the gathered resources.
        auto visitor =
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
        fill->visitAttributeSets(visitor);
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
                                                       << "\" task.");
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

void ResourceAndRole::configureSelf(const Configuration& config) {}

} // namespace adaptor
} // namespace task
} // namespace smtk
