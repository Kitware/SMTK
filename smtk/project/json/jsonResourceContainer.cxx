//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/project/json/jsonResourceContainer.h"

#include "smtk/resource/json/jsonResource.h"

#include "smtk/resource/Manager.h"

namespace smtk
{
namespace project
{
void to_json(json& j, const ResourceContainer& resourceContainer)
{
  j["types"] = resourceContainer.types();
  for (const auto& resource : resourceContainer.resources())
  {
    j["resources"].push_back(resource);
  }
}

void from_json(const json& j, ResourceContainer& resourceContainer)
{
  resourceContainer.types() = j["types"].get<std::set<std::string>>();
  auto manager = resourceContainer.manager();
  if (!manager)
  {
    return;
  }

  for (json::const_iterator it = j["resources"].begin(); it != j["resources"].end(); ++it)
  {
    smtk::resource::ResourcePtr resource =
      manager->read(it->at("type").get<std::string>(), it->at("location").get<std::string>());
    if (!resource)
    {
      continue;
    }
    resourceContainer.add(
      resource, resource->properties().get<std::string>()[ResourceContainer::role_name]);
  }
}
} // namespace project
} // namespace smtk
