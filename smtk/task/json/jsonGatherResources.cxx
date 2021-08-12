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
    nlohmann::json::array_t predicates;
    nlohmann::json::array_t outputs;
    gatherResources->visitPredicates(
      [&predicates, &outputs, gatherResources](
        const GatherResources::Predicate& predicate) -> smtk::common::Visit {
        nlohmann::json jsonPredicate = {
          { "role", predicate.m_role },
          { "type", predicate.m_type },
        };
        if (predicate.m_minimumCount != 1)
        {
          jsonPredicate["min"] = predicate.m_minimumCount;
        }
        if (predicate.m_maximumCount != -1)
        {
          jsonPredicate["max"] = predicate.m_maximumCount;
        }
        predicates.push_back(jsonPredicate);

        nlohmann::json jsonOutput = {
          { "role", predicate.m_role },
        };
        nlohmann::json::array_t jsonResources;
        for (const auto& weakResource : predicate.m_resources)
        {
          if (auto resource = weakResource.lock())
          {
            jsonResources.push_back(resource->id().toString());
          }
        }
        jsonOutput["resources"] = jsonResources;
        outputs.push_back(jsonOutput);
        return smtk::common::Visit::Continue;
      });
    config["resources"] = predicates;
    config["output"] = outputs;
  }
  return config;
}

} // namespace json
} // namespace task
} // namespace smtk
