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

#include "smtk/io/Logger.h"

namespace smtk
{
namespace task
{

void from_json(const nlohmann::json& j, GatherResources::ResourceSet& resourceSet)
{
  if (j.contains("role"))
  {
    j.at("role").get_to(resourceSet.m_role);
  }
  if (j.contains("type"))
  {
    j.at("type").get_to(resourceSet.m_type);
  }
  if (j.contains("min"))
  {
    j.at("min").get_to(resourceSet.m_minimumCount);
  }
  else
  {
    resourceSet.m_minimumCount = 1;
  }
  if (j.contains("max"))
  {
    j.at("max").get_to(resourceSet.m_maximumCount);
  }
  else
  {
    resourceSet.m_maximumCount = -1;
  }
  if (j.contains("validator"))
  {
    throw std::logic_error("todo"); // TODO
  }
  else
  {
    // Accept any resource
    resourceSet.m_validator = nullptr;
    /*
      [](const smtk::resource::Resource&, const TaskNeedsResource::ResourceSet&)
      { return true; };
      */
  }
  if (j.contains("resource-ids"))
  {
    auto resourceIds = j.at("resource-ids");
    bool warnOnIds = !resourceIds.is_array() || !resourceIds.empty();
    auto managers = json::Helper::instance().managers();
    if (managers)
    {
      if (auto resourceManager = managers->get<smtk::resource::Manager::Ptr>())
      {
        for (const auto& jsonId : j.at("resource-ids"))
        {
          auto resource = resourceManager->get(jsonId.get<smtk::common::UUID>());
          if (resource)
          {
            resourceSet.m_resources.insert(resource);
            warnOnIds = false;
          }
          else
          {
            warnOnIds = true;
            smtkWarningMacro(
              smtk::io::Logger::instance(), "Resource \"" << jsonId << "\" not found.");
          }
        }
      }
      else
      {
        warnOnIds = true;
      }
    }
    else
    {
      warnOnIds = true;
    }
    if (warnOnIds)
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Resource IDs were provided but either were in the wrong form "
        "or a resource manager was unavailable.");
    }
  }
}

void to_json(nlohmann::json& j, const GatherResources::ResourceSet& resourceSet)
{
  j = {
    { "role", resourceSet.m_role },
    { "type", resourceSet.m_type },
  };
  if (resourceSet.m_minimumCount != 1)
  {
    j["min"] = resourceSet.m_minimumCount;
  }
  if (resourceSet.m_maximumCount != -1)
  {
    j["max"] = resourceSet.m_maximumCount;
  }
  nlohmann::json::array_t resourceIds;
  for (const auto& weakResource : resourceSet.m_resources)
  {
    if (auto resource = weakResource.lock())
    {
      resourceIds.push_back(resource->id());
    }
  }
  if (!resourceIds.empty())
  {
    j["resource-ids"] = resourceIds;
  }
}

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
      [&resourceSets](const GatherResources::ResourceSet& resourceSet) -> smtk::common::Visit {
        nlohmann::json jsonResourceSet = resourceSet;
        resourceSets.push_back(jsonResourceSet);
        return smtk::common::Visit::Continue;
      });
    config["resources"] = resourceSets;
    config["auto-configure"] = gatherResources->autoConfigure();
  }
  return config;
}

} // namespace json
} // namespace task
} // namespace smtk
