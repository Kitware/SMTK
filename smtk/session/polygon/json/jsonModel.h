//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_polygon_json_jsonModel_h
#define smtk_session_polygon_json_jsonModel_h

#include "smtk/session/polygon/Exports.h"

#include "smtk/session/polygon/internal/Model.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace session
{
namespace polygon
{
namespace internal
{
using json = nlohmann::json;
// Define how polygon resources are serialized.
SMTKPOLYGONSESSION_EXPORT void to_json(
  json& j,
  const smtk::session::polygon::internal::pmodel::Ptr& pmodel);

SMTKPOLYGONSESSION_EXPORT void from_json(
  const json& j,
  smtk::session::polygon::internal::pmodel::Ptr& pmodel);
} // namespace internal
} // namespace polygon
} // namespace session
} // namespace smtk

#endif
