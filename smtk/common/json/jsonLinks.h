//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_json_jsonLinks_h
#define smtk_common_json_jsonLinks_h

#include "smtk/CoreExports.h"

#include "smtk/common/Links.h"

#include "nlohmann/json.hpp"

// Define how links are serialized.
namespace smtk
{
namespace common
{
using json = nlohmann::json;

template <typename id_type, typename left_type, typename right_type, typename base_type>
void to_json(json& j, const Links<id_type, left_type, right_type, base_type>& links)
{
  for (auto link : links)
  {
    json jlink;
    const base_type& base = static_cast<base_type&>(link);
    jlink["id"] = link.id;
    jlink["base"] = base;
    jlink["left"] = link.left;
    jlink["right"] = link.right;
    jlink["role"] = link.role;
    j.push_back(jlink);
  }
}

template <typename id_type, typename left_type, typename right_type, typename base_type>
void from_json(const json& j, Links<id_type, left_type, right_type, base_type>& links)
{
  for (json::const_iterator it = j.begin(); it != j.end(); ++it)
  {
    base_type base = it->at("base");
    id_type id = it->at("id");
    left_type left = it->at("left");
    right_type right = it->at("right");
    std::string role = it->at("role");
    links.insert(std::move(base), id, std::move(left), std::move(right), role);
  }
}

namespace detail
{
SMTKCORE_EXPORT void to_json(json&, const NullLinkBase&);
SMTKCORE_EXPORT void from_json(const json&, NullLinkBase&);
}
}
}

#endif
