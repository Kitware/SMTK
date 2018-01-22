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

#include "smtk/common/json/jsonUUID.h"

#include "smtk/resource/Resource.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include <string>

/**\brief Provide a way to serialize a set of components.
  *
  * Deserialization requires a resource manager from which to fetch
  * pointers to resources and their components given only UUIDs.
  */
namespace smtk
{
namespace resource
{
/// Convert a set of resource components to JSON.
SMTKCORE_EXPORT void to_json(json& j, const smtk::resource::ComponentSet& cset);

/// Conversion from JSON requires a resource manager to look up pointers from UUIDs.
SMTKCORE_EXPORT void from_json(
  const json&, std::set<smtk::resource::ComponentPtr>&, smtk::resource::ManagerPtr);
}
}

#endif
