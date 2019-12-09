//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/json/jsonSelectionMap.h"

#include "smtk/common/json/jsonUUID.h"
#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

#include "nlohmann/json.hpp"

#include <string>

/**\brief Provide a way to serialize a selection.
  *
  * Deserialization requires a resource manager from which to fetch
  * resources and their components.
  */
namespace smtk
{
namespace resource
{
/// Convert a Selection's currentSelection() to JSON.
SMTKCORE_EXPORT void to_json(json& j, const std::map<smtk::resource::ComponentPtr, int>& seln)
{
  if (seln.empty())
  {
    j = nullptr;
  }
  else
  {
    // Group components by resource IDs:
    std::map<smtk::common::UUID, std::map<smtk::common::UUID, int> > rcset;
    for (const auto& entry : seln)
    {
      auto rset = rcset.find(entry.first->resource()->id());
      if (rset == rcset.end())
      {
        std::map<smtk::common::UUID, int> blank;
        rset = rcset.insert(std::make_pair(entry.first->resource()->id(), blank)).first;
      }
      rset->second.insert(std::make_pair(entry.first->id(), entry.second));
    }

    // Now output arrays of resources that list [component-id, selection-level] pairs.
    j = json::array();
    for (const auto& rsrc : rcset)
    {
      json ra = json::array();
      ra.push_back(rsrc.first);
      for (const auto& cmap : rsrc.second)
      {
        ra.push_back(json::array({ cmap.first, cmap.second }));
      }
      j.push_back(ra);
    }
  }
}

/// Conversion from JSON requires a resource manager to look up pointers from UUIDs.
SMTKCORE_EXPORT void from_json(const json& /*unused*/,
  std::map<smtk::resource::ComponentPtr, int>& /*unused*/, smtk::resource::ManagerPtr /*unused*/)
{
}
}
}
