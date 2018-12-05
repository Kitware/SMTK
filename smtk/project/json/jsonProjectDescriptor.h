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

#include "smtk/project/ProjectDescriptor.h"

#include "nlohmann/json.hpp"

#include <string>

using json = nlohmann::json;

namespace smtk
{
namespace project
{
SMTKCORE_EXPORT void to_json(json& j, const ProjectDescriptor& pd);
SMTKCORE_EXPORT void from_json(const json& j, ProjectDescriptor& pd);
SMTKCORE_EXPORT std::string dump_json(const ProjectDescriptor& pd, int indent = 2);
SMTKCORE_EXPORT void parse_json(const std::string& input, ProjectDescriptor& pd);
} // namespace project
} // namespace smtk

#endif // smtk_project_jsonProjectDescriptor_h
