//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_bridge_polygon_json_jsonResource_h
#define smtk_bridge_polygon_json_jsonResource_h

#include "smtk/bridge/polygon/Exports.h"

#include "smtk/bridge/polygon/Resource.h"

#include "smtk/bridge/polygon/internal/Edge.h"
#include "smtk/bridge/polygon/internal/Model.h"
#include "smtk/bridge/polygon/internal/Vertex.h"

#include "smtk/bridge/polygon/json/jsonEdge.h"
#include "smtk/bridge/polygon/json/jsonModel.h"
#include "smtk/bridge/polygon/json/jsonVertex.h"

#include "smtk/model/json/jsonResource.h"

#include "nlohmann/json.hpp"

// Define how polygon resources are serialized.
namespace smtk
{
namespace bridge
{
namespace polygon
{
using json = nlohmann::json;
SMTKPOLYGONSESSION_EXPORT void to_json(
  json& j, const smtk::bridge::polygon::Resource::Ptr& resource);

SMTKPOLYGONSESSION_EXPORT void from_json(
  const json& j, smtk::bridge::polygon::Resource::Ptr& resource);
}
}
}

#endif
