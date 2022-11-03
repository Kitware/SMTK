//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_json_jsonTypeMap_h
#define smtk_common_json_jsonTypeMap_h

#include "smtk/CoreExports.h"

#include "smtk/Regex.h"
#include "smtk/common/TypeMap.h"

#include "nlohmann/json.hpp"

// Define how type maps are serialized.
namespace smtk
{
namespace common
{

template<typename KeyType>
void to_json(nlohmann::json& j, const TypeMapBase<KeyType>& typemap)
{
  for (auto& entry : typemap.data())
  {
    std::string entryName = entry.first;
    const TypeMapEntryBase& base = *entry.second;
    auto& base_j = j[entryName];
    base.to_json(base_j);
    if (base_j == nullptr)
    {
      j.erase(entryName);
    }
  }
}

template<typename KeyType>
void from_json(const nlohmann::json& j, TypeMapBase<KeyType>& typemap)
{
  for (auto& entry : typemap.data())
  {
    auto it = j.find(entry.first);
    if (it != j.end())
    {
      entry.second->from_json(*it);
    }

    // This kludge provides us with backwards compatibility for typemap on
    // resources and components.
    else
    {
      std::string tmp = entry.first;
      tmp = smtk::regex_replace(tmp, smtk::regex("unordered_"), "unordered ");

      it = j.find(tmp);
      if (it != j.end())
      {
        entry.second->from_json(*it);
      }
    }
  }
}
} // namespace common
} // namespace smtk

#endif
