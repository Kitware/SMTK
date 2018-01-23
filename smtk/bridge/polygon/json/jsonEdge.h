//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_bridge_polygon_json_jsonEdge_h
#define smtk_bridge_polygon_json_jsonEdge_h

#include "smtk/bridge/polygon/Exports.h"

#include "smtk/bridge/polygon/internal/Edge.h"

#include "smtk/model/json/jsonTessellation.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace bridge
{
namespace polygon
{
namespace internal
{
using json = nlohmann::json;
// Define how polygon resources are serialized.
SMTKPOLYGONSESSION_EXPORT void to_json(
  json& j, const smtk::bridge::polygon::internal::edge::Ptr& edge);

SMTKPOLYGONSESSION_EXPORT void from_json(
  const json& j, smtk::bridge::polygon::internal::edge::Ptr& edge);

} // namespace internal
} // namespace polygon
} // namespace bridge
} // namespace smtk
#endif
