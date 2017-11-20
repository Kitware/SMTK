//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_io_jsonComponentSet_h
#define smtk_io_jsonComponentSet_h

#include "smtk/io/jsonUUID.h"

#include "smtk/resource/Resource.h"

#include "nlohmann/json.hpp"

#include <string>

/**\brief Provide a way to serialize a set of components.
  *
  * Deserialization requires a resource manager from which to fetch
  * pointers to resources and their components given only UUIDs.
  */
namespace nlohmann
{
template <>
struct adl_serializer<smtk::resource::ComponentSet>
{
  /// Convert a SelectionManager's currentSelection() to JSON.
  static void to_json(json& j, const smtk::resource::ComponentSet& cset)
  {
    if (cset.empty())
    {
      j = nullptr;
    }
    else
    {
      // Group components by resource IDs:
      std::map<smtk::common::UUID, std::set<smtk::common::UUID> > rcset;
      for (auto entry : cset)
      {
        auto rset = rcset.find(entry->resource()->id());
        if (rset == rcset.end())
        {
          smtk::common::UUIDs blank;
          rset = rcset.insert(std::make_pair(entry->resource()->id(), blank)).first;
        }
        rset->second.insert(entry->id());
      }

      // Now output arrays of resources that list component-id members.
      j = json::array();
      for (auto rsrc : rcset)
      {
        json ra = json::array();
        ra.push_back(rsrc.first);
        ra.push_back(rsrc.second);
        j.push_back(ra);
      }
    }
  }

  /// Conversion from JSON requires a resource manager to look up pointers from UUIDs.
  static void from_json(
    const json&, std::set<smtk::resource::ComponentPtr>&, smtk::resource::ManagerPtr)
  {
  }
};
}

#endif
