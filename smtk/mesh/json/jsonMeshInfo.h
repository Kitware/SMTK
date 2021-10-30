//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_mesh_json_jsonMeshInfo_h
#define smtk_mesh_json_jsonMeshInfo_h

#include "nlohmann/json.hpp"

#include <vector>

namespace smtk
{
namespace mesh
{
namespace json
{
class MeshInfo;

void to_json(nlohmann::json&, const std::vector<MeshInfo>&);

void from_json(const nlohmann::json&, std::vector<MeshInfo>&);
} // namespace json
} // namespace mesh
} // namespace smtk

#endif
