//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/json/jsonProperties.h"

#include <string>

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

namespace smtk
{
namespace common
{
void to_json(nlohmann::json& j, const PropertiesContainer& properties)
{
  for (auto& property : properties.data())
  {
    std::string propertyName = property.first;
    const PropertiesBase& base = *property.second;
    auto& base_j = j[propertyName];
    base.to_json(base_j);
    if (base_j == nullptr)
    {
      j.erase(propertyName);
    }
  }
}

void from_json(const nlohmann::json& j, PropertiesContainer& properties)
{
  for (auto& property : properties.data())
  {
    auto it = j.find(property.first);
    if (it != j.end())
    {
      property.second->from_json(*it);
    }

    // This kludge provides us with backwards compatibility for properties on
    // resources and components.
    else
    {
      std::string tmp = property.first;
      tmp = regex_replace(tmp, regex("unordered_"), "unordered ");

      it = j.find(tmp);
      if (it != j.end())
      {
        property.second->from_json(*it);
      }
    }
  }
}
}
}
