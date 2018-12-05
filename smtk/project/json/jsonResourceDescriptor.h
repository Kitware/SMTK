//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_jsonResourceDescriptor_h
#define smtk_project_jsonResourceDescriptor_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"

#include "smtk/project/ResourceDescriptor.h"

#include "nlohmann/json.hpp"

#include <string>
#include <vector>

using json = nlohmann::json;

namespace smtk
{
namespace project
{
SMTKCORE_EXPORT void to_json(json& j, const ResourceDescriptor& rd);
SMTKCORE_EXPORT void from_json(const json& j, ResourceDescriptor& rd);
} // namespace project
} // namespace smtk

#endif // smtk_project_jsonResourceDescriptor_h
