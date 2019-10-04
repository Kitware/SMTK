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
  }
}
}
}
