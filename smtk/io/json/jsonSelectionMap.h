//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_io_jsonSelectionMap_h
#define smtk_io_jsonSelectionMap_h

#include "smtk/common/json/jsonUUID.h"

#include "nlohmann/json.hpp"

#include <string>

/**\brief Provide a way to serialize a selection.
  *
  * Deserialization requires a resource manager from which to fetch
  * resources and their components.
  */
namespace nlohmann
{
template <>
struct adl_serializer<std::map<smtk::resource::ComponentPtr, int> >
{
  /// Convert a Selection's currentSelection() to JSON.
  static void to_json(json& j, const std::map<smtk::resource::ComponentPtr, int>& seln)
  {
    if (seln.empty())
    {
      j = nullptr;
    }
    else
    {
      // Group components by resource IDs:
      std::map<smtk::common::UUID, std::map<smtk::common::UUID, int> > rcset;
      for (auto entry : seln)
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
      for (auto rsrc : rcset)
      {
        json ra = json::array();
        ra.push_back(rsrc.first);
        for (auto cmap : rsrc.second)
        {
          ra.push_back(json::array({ cmap.first, cmap.second }));
        }
        j.push_back(ra);
      }
    }
  }

  /// Conversion from JSON requires a resource manager to look up pointers from UUIDs.
  static void from_json(
    const json&, std::map<smtk::resource::ComponentPtr, int>&, smtk::resource::ManagerPtr)
  {
  }
};
}

#endif
