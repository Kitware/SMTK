//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#pragma once

#include "nlohmann/json.hpp"

namespace nlohmann
{

/**\brief Merge 2 JSON dictionaries together by taking the union of their keys.
  *
  * Where the same key exists with different values, choose one unless the
  * values are both dictionaries in which case the merge is continued recursively.
  */
json combine(const json& ja, const json& jb)
{
  json result(ja);
  if (ja.is_object() && jb.is_object())
  {
    for (auto it = jb.begin(); it != jb.end(); ++it)
    {
      json::iterator kit;
      if (!it->is_object() || (kit = result.find(it.key())) == result.end() || !kit->is_object())
      {
        result[it.key()] = *it;
      }
      else
      {
        *kit = combine(*kit, *it);
      }
    }
  }
  return result;
}
} // namespace nlohmann
