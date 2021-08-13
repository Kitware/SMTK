//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/jsonGatherResources.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/task/GatherResources.h"

namespace smtk
{
namespace task
{
namespace json
{

Task::Configuration jsonGatherResources::operator()(const Task* task, Helper& helper) const
{
  Task::Configuration config;
  auto* nctask = const_cast<Task*>(task);
  auto* gatherResources = dynamic_cast<GatherResources*>(nctask);
  if (gatherResources)
  {
    jsonTask superclass;
    config = superclass(gatherResources, helper);
    nlohmann::json::array_t resourceSets;
    gatherResources->visitResourceSets(
      [&resourceSets,
       gatherResources](const GatherResources::ResourceSet& resourceSet) -> smtk::common::Visit {
        nlohmann::json jsonResourceSet = {
          { "role", resourceSet.m_role },
          { "type", resourceSet.m_type },
        };
        if (resourceSet.m_minimumCount != 1)
        {
          jsonResourceSet["min"] = resourceSet.m_minimumCount;
        }
        if (resourceSet.m_maximumCount != -1)
        {
          jsonResourceSet["max"] = resourceSet.m_maximumCount;
        }
        resourceSets.push_back(jsonResourceSet);

        nlohmann::json jsonOutput = {
          { "role", resourceSet.m_role },
        };
        nlohmann::json::array_t jsonResources;
        for (const auto& weakResource : resourceSet.m_resources)
        {
          if (auto resource = weakResource.lock())
          {
            jsonResources.push_back(resource->id().toString());
          }
        }
        jsonOutput["resources"] = jsonResources;
        return smtk::common::Visit::Continue;
      });
    config["resources"] = resourceSets;
  }
  return config;
}

} // namespace json
} // namespace task
} // namespace smtk
