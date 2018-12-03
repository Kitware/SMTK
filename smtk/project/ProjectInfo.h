//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_ProjectInfo_h
#define smtk_project_ProjectInfo_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"

#include "smtk/common/UUID.h"

#include "nlohmann/json.hpp"

#include <string>
#include <vector>

using json = nlohmann::json;

// This file contains two header-only classes:
//   * ResourceInfo: persistent data for one resource
//   * ProjectInfo: persistent data for project
// This file also includes static to_json() and from_json()
// methods for each class.

namespace smtk
{
namespace project
{
/// Class representing the persistent data stored for each
/// resource used in a project. Intended primarily for internal use.
class SMTKCORE_EXPORT ResourceInfo
{
public:
  /// Default constructor
  ResourceInfo() {}

  /// Copy constructor
  ResourceInfo(const ResourceInfo& source);

  /// Resource filename
  std::string m_filename;

  /// User-specified string for labeling resource in UI widgets.
  std::string m_identifier;

  /// The filesystem location for the file, if any, that was imported
  /// to create the resource. Examples include .gen or.exo file for
  /// a model resource, or .sbt file for an attribute resource.
  /// (Future) this will be expanded to encompass URL locations, as
  /// well as multiple locations of the same resource,
  std::string m_importLocation;

  /// Identifies how the resource is used in the project.
  /// Examples include "default" and "export".
  std::string m_role;

  /// (Future) Consider m_checksum for veryifying that import file
  /// hasn't changed. Might be moot if we store projects as archive
  /// files.
  //  unsigned int m_checksum;

  /// Stores the resource type, as the string returned from smtk::resource::typeName()
  std::string m_typeName;

  /// Resource UUID
  smtk::common::UUID m_uuid;
}; // class smtk::project::ResourceInfo

inline ResourceInfo::ResourceInfo(const ResourceInfo& source)
{
  m_filename = source.m_filename;
  m_identifier = source.m_identifier;
  m_importLocation = source.m_importLocation;
  m_role = source.m_role;
  m_typeName = source.m_typeName;
  m_uuid = source.m_uuid;
}

// Static methods to convert to/from json
static void to_json(json& j, const ResourceInfo& info)
{
  j = { { "filename", info.m_filename }, { "identifier", info.m_identifier },
    { "importLocation", info.m_importLocation }, { "role", info.m_role },
    { "typeName", info.m_typeName }, { "uuid", info.m_uuid.toString() } };
} // to_json()

static void from_json(const json& j, ResourceInfo& info)
{
  try
  {
    info.m_filename = j.at("filename");
    info.m_identifier = j.at("identifier");
    info.m_importLocation = j.at("importLocation");
    info.m_role = j.at("role");
    info.m_typeName = j.at("typeName");

    std::string uuidString = j.at("uuid");
    info.m_uuid = smtk::common::UUID(uuidString);
  }
  catch (std::exception& ex)
  {
    std::cerr << "ERROR: " << ex.what();
  }
} // from_json()

/// Class representing the persistent data stored for a project.
class SMTKCORE_EXPORT ProjectInfo
{
public:
  /// User-supplied name for the project
  std::string m_name;

  /// Filesystem directory where project resources are stored.
  std::string m_directory;

  /// Array of ResourceInfo objects for each project resource.
  /// These data are stored in a file in the project directory.
  std::vector<ResourceInfo> m_resourceInfos;
}; // class smtk::project::ProjectInfo

// Static methods to convert to/from json
static void to_json(json& j, const ProjectInfo& projectInfo)
{
  j = {
    { "fileVersion", 1 }, { "projectName", projectInfo.m_name },
    { "projectDirectory", projectInfo.m_directory },
  };
  json jInfos = json::array();
  for (auto& resourceInfo : projectInfo.m_resourceInfos)
  {
    nlohmann::json jInfo = resourceInfo;
    jInfos.push_back(jInfo);
  }
  j["resourceInfos"] = jInfos;
} // to_json()

static void from_json(const json& j, ProjectInfo& projectInfo)
{
  try
  {
    projectInfo.m_name = j.at("projectName");
    projectInfo.m_directory = j.at("projectDirectory");
    auto jInfos = j.at("resourceInfos");
    for (auto& jInfo : jInfos)
    {
      ResourceInfo resourceInfo = jInfo;
      projectInfo.m_resourceInfos.push_back(resourceInfo);
    }
  }
  catch (std::exception& ex)
  {
    std::cerr << "ERROR: " << ex.what();
  }
} // from_json()

} // namespace project
} // namespace smtk

#endif // smtk_project_ProjectInfo_h
