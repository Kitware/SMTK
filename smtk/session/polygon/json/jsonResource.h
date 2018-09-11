//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_polygon_json_jsonResource_h
#define smtk_session_polygon_json_jsonResource_h

#include "smtk/session/polygon/Exports.h"

#include "smtk/session/polygon/Resource.h"

#include "smtk/session/polygon/internal/Edge.h"
#include "smtk/session/polygon/internal/Model.h"
#include "smtk/session/polygon/internal/Vertex.h"

#include "smtk/session/polygon/json/jsonEdge.h"
#include "smtk/session/polygon/json/jsonModel.h"
#include "smtk/session/polygon/json/jsonVertex.h"

#include "smtk/model/json/jsonResource.h"

#include "nlohmann/json.hpp"

// Define how polygon resources are serialized.
namespace smtk
{
namespace session
{
namespace polygon
{
using json = nlohmann::json;
SMTKPOLYGONSESSION_EXPORT void to_json(
  json& j, const smtk::session::polygon::Resource::Ptr& resource);

SMTKPOLYGONSESSION_EXPORT void from_json(
  const json& j, smtk::session::polygon::Resource::Ptr& resource);
}
}
}

#endif
