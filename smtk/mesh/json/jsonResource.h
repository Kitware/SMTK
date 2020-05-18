//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_json_jsonResource_h
#define __smtk_mesh_json_jsonResource_h

#include "smtk/mesh/core/Resource.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace mesh
{
SMTKCORE_EXPORT void to_json(nlohmann::json&, const ResourcePtr&);

SMTKCORE_EXPORT void from_json(const nlohmann::json&, ResourcePtr&);
} // namespace mesh
} // namespace smtk

#endif
