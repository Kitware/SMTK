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

#include "smtk/common/UUID.h"
#include "smtk/common/json/Helper.h"
#include "smtk/common/json/jsonUUID.h"

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
  const auto* helper = Helper<left_type, right_type>::instance();
  if (helper)
  {
    for (const auto& link : links)
    {
      if (!helper->includeLink(link.left))
      {
        continue; // This link is not required
      }
      const base_type& base = static_cast<const base_type&>(link);
      json jlink{ { "id", link.id },
                  { "base", base },
                  { "role", link.role },
                  { "left", helper->serializeLeft(link.left) },
                  { "right", helper->serializeRight(link.right) } };
      j.push_back(jlink);
    }
  }
  else
  {
    for (const auto& link : links)
    {
      const base_type& base = static_cast<const base_type&>(link);
      json jlink{ { "id", link.id },
                  { "base", base },
                  { "role", link.role },
                  { "left", link.left },
                  { "right", link.right } };
      j.push_back(jlink);
    }
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
  const auto* helper = Helper<left_type, right_type>::instance();
  if (helper)
  {
    for (json::const_iterator it = j.begin(); it != j.end(); ++it)
    {
      links.insert(
        std::move(it->at("base").get<base_type>()),
        it->at("id").get<id_type>(),
        helper->deserializeLeft(it->at("left")),
        helper->deserializeRight(it->at("right")),
        it->at("role").get<role_type>());
    }
  }
  else
  {
    for (json::const_iterator it = j.begin(); it != j.end(); ++it)
    {
      links.insert(
        std::move(it->at("base").get<base_type>()),
        it->at("id").get<id_type>(),
        it->at("left").get<left_type>(),
        it->at("right").get<right_type>(),
        it->at("role").get<role_type>());
    }
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
