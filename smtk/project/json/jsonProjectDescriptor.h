//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_jsonProjectDescriptor_h
#define smtk_project_jsonProjectDescriptor_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"

#include "smtk/common/UUID.h"
#include "smtk/project/ProjectDescriptor.h"
#include "smtk/project/ResourceDescriptor.h"
#include "smtk/project/json/jsonResourceDescriptor.h"

#include "nlohmann/json.hpp"

#include <cassert>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace smtk
{
namespace project
{
static void to_json(json& j, const ProjectDescriptor& pd)
{
  j = {
    { "fileVersion", 1 }, { "projectName", pd.m_name }, { "projectDirectory", pd.m_directory },
  };
  json jDescriptors = json::array();
  for (auto& descriptor : pd.m_resourceDescriptors)
  {
    json jDescriptor = descriptor;
    jDescriptors.push_back(jDescriptor);
  }
  j["resources"] = jDescriptors;
} // to_json()

static void from_json(const json& j, ProjectDescriptor& pd)
{
  try
  {
    int fileVersion = j.at("fileVersion");
    assert(fileVersion == 1);

    pd.m_name = j.at("projectName");
    pd.m_directory = j.at("projectDirectory");
    auto jDescriptors = j.at("resources");
    for (auto& jDescriptor : jDescriptors)
    {
      ResourceDescriptor descriptor = jDescriptor;
      pd.m_resourceDescriptors.push_back(descriptor);
    }
  }
  catch (std::exception& ex)
  {
    std::cerr << "ERROR: " << ex.what();
  }
} // from_json()

static std::string dump_json(const ProjectDescriptor& pd, int indent = 2)
{
  json j = pd;
  return j.dump(indent);
}

static void parse_json(const std::string& input, ProjectDescriptor& pd)
{
  auto j = nlohmann::json::parse(input);
  pd = j;
}

} // namespace project
} // namespace smtk

#endif // smtk_project_jsonProjectDescriptor_h
