//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_json_jsonMeshInfo_h
#define __smtk_mesh_json_jsonMeshInfo_h

#include "nlohmann/json.hpp"

#include <vector>

namespace smtk
{
namespace mesh
{
namespace json
{
class MeshInfo;

using json = nlohmann::json;

void to_json(json&, const std::vector<MeshInfo>&);

void from_json(const json&, std::vector<MeshInfo>&);
}
}
}

#endif
