//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_project_json_jsonResourceContainer_h
#define smtk_project_json_jsonResourceContainer_h

#include "smtk/CoreExports.h"

#include "smtk/project/ResourceContainer.h"

#include "nlohmann/json.hpp"

// Define how project ResourceContainer is serialized.
namespace smtk
{
namespace project
{
using json = nlohmann::json;

SMTKCORE_EXPORT void to_json(json&, const ResourceContainer&, const ProjectPtr& project);

SMTKCORE_EXPORT void from_json(const json&, ResourceContainer&, const ProjectPtr& project);
} // namespace project
} // namespace smtk

#endif
