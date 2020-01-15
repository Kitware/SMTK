//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_project_json_jsonProject_h
#define smtk_project_json_jsonProject_h

#include "smtk/CoreExports.h"

#include "smtk/project/Project.h"

#include "nlohmann/json.hpp"

// Define how projects are serialized.
namespace smtk
{
namespace project
{
using json = nlohmann::json;

SMTKCORE_EXPORT void to_json(json&, const ProjectPtr&);

SMTKCORE_EXPORT void from_json(const json&, ProjectPtr&);
} // namespace project
} // namespace smtk

#endif
