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

#include "smtk/common/json/Helper.h"

#include "nlohmann/json.hpp"

// Define how links are serialized.
namespace smtk
{
namespace common
{
using json = nlohmann::json;

template<
  typename id_type,
  typename left_type,
  typename right_type,
  typename role_type,
  typename base_type>
void to_json(json& j, const Links<id_type, left_type, right_type, role_type, base_type>& links)
{
  const auto& helper = Helper<left_type, right_type>::instance();
  const auto& requiredIds = helper.requiredIds();
  for (const auto& link : links)
  {
    if (helper.hasRequiredIds() && (requiredIds.find(link.left) == requiredIds.end()))
    {
      continue; // This link is not required
    }
    json jlink;
    const base_type& base = static_cast<const base_type&>(link);
    jlink["id"] = link.id;
    jlink["base"] = base;
    if (helper.hasLeftPlaceholderId() && (helper.leftPlaceholderId() == link.left))
    {
      jlink["left"] = helper.leftPlaceholderText();
    }
    else
    {
      jlink["left"] = link.left;
    }
    if (helper.hasRightPlaceholderId() && (helper.rightPlaceholderId() == link.right))
    {
      jlink["right"] = helper.rightPlaceholderText();
    }
    else
    {
      jlink["right"] = link.right;
    }
    jlink["role"] = link.role;
    j.push_back(jlink);
  }
}

template<
  typename id_type,
  typename left_type,
  typename right_type,
  typename role_type,
  typename base_type>
void from_json(const json& j, Links<id_type, left_type, right_type, role_type, base_type>& links)
{
  const auto& helper = Helper<left_type, right_type>::instance();
  for (json::const_iterator it = j.begin(); it != j.end(); ++it)
  {
    base_type base = it->at("base");
    id_type id = it->at("id");
    left_type left;
    if (
      helper.hasLeftPlaceholderId() &&
      (helper.leftPlaceholderText() == it->at("left").get<std::string>()))
    {
      left = helper.leftPlaceholderId();
    }
    else
    {
      left = it->at("left");
    }
    right_type right;
    if (
      helper.hasRightPlaceholderId() &&
      (helper.rightPlaceholderText() == it->at("right").get<std::string>()))
    {
      right = helper.rightPlaceholderId();
    }
    else
    {
      right = it->at("right");
    }
    role_type role = it->at("role");
    links.insert(std::move(base), id, left, right, role);
  }
}

namespace detail
{
SMTKCORE_EXPORT void to_json(json&, const NullLinkBase&);
SMTKCORE_EXPORT void from_json(const json&, NullLinkBase&);
} // namespace detail
} // namespace common
} // namespace smtk

#endif
