//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonItemDefinition_h
#define smtk_attribute_jsonItemDefinition_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/ItemDefinition.h"

#include "nlohmann/json.hpp"
#include "smtk/CoreExports.h"

#include <string>

/**\brief Provide a way to serialize itemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(
  nlohmann::json& j, const smtk::attribute::ItemDefinitionPtr& itemDefPtr);

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j, smtk::attribute::ItemDefinitionPtr& itemDefPtr);
}
}

#endif
