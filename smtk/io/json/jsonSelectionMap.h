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

#include "smtk/PublicPointerDefs.h"
#include "smtk/common/json/jsonUUID.h"

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
SMTKCORE_EXPORT void to_json(json& j, const std::map<smtk::resource::ComponentPtr, int>& seln);

/// Conversion from JSON requires a resource manager to look up pointers from UUIDs.
SMTKCORE_EXPORT void
from_json(const json&, std::map<smtk::resource::ComponentPtr, int>&, smtk::resource::ManagerPtr);
} // namespace resource
} // namespace smtk

#endif
