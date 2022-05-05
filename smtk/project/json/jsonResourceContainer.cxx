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

#include "smtk/project/Project.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

namespace
{
void replaceWindowsSeparators(boost::filesystem::path& path)
{
  std::string pathString = boost::algorithm::replace_all_copy(path.string(), "\\", "/");
  path = boost::filesystem::path(pathString);
}
} // namespace

namespace smtk
{
namespace project
{
void to_json(json& j, const ResourceContainer& resourceContainer, const ProjectPtr& project)
{
  // get the base path of the project
  std::string projectPath = project->location();
  boost::filesystem::path parentPath = boost::filesystem::path(projectPath).parent_path();

  j["types"] = resourceContainer.types();
  j["resources"] = json::array();
  for (const auto& resource : resourceContainer)
  {
    nlohmann::json jResource = resource;
    std::string path = jResource["location"]; // stored path may be an absolute path

    // convert stored path (which may be an absolute path) to a relative path
    boost::filesystem::path filePath(path);
    boost::filesystem::path newPath = boost::filesystem::relative(filePath, parentPath);
    replaceWindowsSeparators(newPath);
    jResource["location"] = newPath.string();
    j["resources"].push_back(jResource);
  }
}

void from_json(const json& j, ResourceContainer& resourceContainer, const ProjectPtr& project)
{
  resourceContainer.types() = j["types"].get<std::set<std::string>>();
  auto manager = resourceContainer.manager();
  if (!manager)
  {
    return;
  }

  // get the base path of the project
  std::string projectPath = project->location();
  boost::filesystem::path parentPath = boost::filesystem::path(projectPath).parent_path();

  for (json::const_iterator it = j["resources"].begin(); it != j["resources"].end(); ++it)
  {
    std::string location = it->at("location").get<std::string>();
    boost::filesystem::path locationPath(location);
    if (!locationPath.is_absolute())
    {
      locationPath = boost::filesystem::absolute(locationPath, parentPath);
    }
    replaceWindowsSeparators(locationPath);

    smtk::resource::ResourcePtr resource =
      manager->read(it->at("type").get<std::string>(), locationPath.string());
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
