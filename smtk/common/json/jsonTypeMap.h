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

#include "smtk/common/TypeMap.h"

#include "nlohmann/json.hpp"

// We use either STL regex or Boost regex, depending on support. These flags
// correspond to the equivalent logic used to determine the inclusion of Boost's
// regex library.
#if defined(SMTK_CLANG) ||                                                                         \
  (defined(SMTK_GCC) && __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)) ||                 \
  defined(SMTK_MSVC)
#include <regex>
using std::regex;
using std::regex_replace;
#else
#include <boost/regex.hpp>
using boost::regex;
using boost::regex_replace;
#endif

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
      tmp = regex_replace(tmp, regex("unordered_"), "unordered ");

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
