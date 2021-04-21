//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/project/json/jsonProjectDescriptor.h"

#include "smtk/common/UUID.h"
#include "smtk/project/ResourceDescriptor.h"
#include "smtk/project/json/jsonResourceDescriptor.h"

#include <sstream>
#include <stdexcept>
#include <vector>

using json = nlohmann::json;

namespace smtk
{
namespace project
{
void to_json(json& j, const ProjectDescriptor& pd)
{
  j = {
    { "fileVersion", 1 },
    { "simulationCode", pd.m_simulationCode },
    { "projectName", pd.m_name },
    { "projectDirectory", pd.m_directory },
  };
  json jDescriptors = json::array();
  for (const auto& descriptor : pd.m_resourceDescriptors)
  {
    json jDescriptor = descriptor;
    jDescriptors.push_back(jDescriptor);
  }
  j["resources"] = jDescriptors;
} // to_json()

void from_json(const json& j, ProjectDescriptor& pd)
{
  int fileVersion = j.at("fileVersion");
  if (fileVersion != 1)
  {
    std::stringstream ss;
    ss << "Cannot read project file version " << fileVersion;
    throw std::runtime_error(ss.str());
  }

  pd.m_simulationCode = j.at("simulationCode").get<std::string>();
  pd.m_name = j.at("projectName").get<std::string>();
  pd.m_directory = j.at("projectDirectory").get<std::string>();
  auto jDescriptors = j.at("resources");
  for (auto& jDescriptor : jDescriptors)
  {
    ResourceDescriptor descriptor = jDescriptor;
    pd.m_resourceDescriptors.push_back(descriptor);
  }
} // from_json()

std::string dump_json(const ProjectDescriptor& pd, int indent)
{
  json j = pd;
  return j.dump(indent);
}

void parse_json(const std::string& input, ProjectDescriptor& pd)
{
  auto j = nlohmann::json::parse(input);
  pd = j;
}

} // namespace project
} // namespace smtk
