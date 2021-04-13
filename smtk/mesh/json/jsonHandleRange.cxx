//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/json/jsonHandleRange.h"

namespace smtk
{
namespace mesh
{
using nlohmann::json;

void to_json(nlohmann::json& j, const smtk::mesh::HandleRange& handleRange)
{
  for (auto& handleInterval : handleRange)
  {
    j.push_back(std::make_pair(handleInterval.lower(), handleInterval.upper()));
  }
}

void from_json(const nlohmann::json& j, smtk::mesh::HandleRange& handleRange)
{
  if (j.is_null())
  {
    return;
  }

  std::pair<Handle, Handle> handlePair;

  for (auto& jsonHandleInterval : j)
  {
    handlePair = jsonHandleInterval.get<std::pair<Handle, Handle>>();
    handleRange.insert(handleRange.end(), HandleInterval(handlePair.first, handlePair.second));
  }
}
} // namespace mesh
} // namespace smtk
