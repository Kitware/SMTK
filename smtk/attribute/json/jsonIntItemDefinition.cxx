//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonIntItemDefinition.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/IntItemDefinition.h"

#include "nlohmann/json.hpp"
#include "smtk/attribute/json/jsonValueItemDefinition.h"

#include <string>
using json = nlohmann::json;

/**\brief Provide a way to serialize IntItemDefinitionPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(nlohmann::json& j, const smtk::attribute::IntItemDefinitionPtr& defPtr)
{
  // No need to call ItemDefinition's to_json function since ValueItemDefinition's
  // to_json function would take care of it
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ValueItemDefinition>(defPtr));
  smtk::attribute::processDerivedValueDefToJson(j, defPtr);
}

SMTKCORE_EXPORT void from_json(const nlohmann::json& j,
  smtk::attribute::IntItemDefinitionPtr& defPtr, const smtk::attribute::ResourcePtr& resPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto valItemDef = smtk::dynamic_pointer_cast<ValueItemDefinition>(defPtr);
  smtk::attribute::from_json(j, valItemDef, resPtr);
  smtk::attribute::processDerivedValueDefFromJson<smtk::attribute::IntItemDefinitionPtr, int>(
    j, defPtr, resPtr);
}
}
}
