//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_mesh_json_jsonHandleRange_h
#define smtk_mesh_json_jsonHandleRange_h

#include "smtk/CoreExports.h"

#include "smtk/mesh/core/Handle.h"

#include "nlohmann/json.hpp"

// Define how resources are serialized.
namespace smtk
{
namespace mesh
{
SMTKCORE_EXPORT void to_json(nlohmann::json&, const smtk::mesh::HandleRange&);

SMTKCORE_EXPORT void from_json(const nlohmann::json&, smtk::mesh::HandleRange&);
} // namespace mesh
} // namespace smtk

namespace nlohmann
{
template<>
struct adl_serializer<smtk::mesh::HandleRange>
{
  static void to_json(json& j, const smtk::mesh::HandleRange& opt) { smtk::mesh::to_json(j, opt); }

  static void from_json(const json& j, smtk::mesh::HandleRange& opt)
  {
    smtk::mesh::from_json(j, opt);
  }
};
} // namespace nlohmann

#endif
