//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonVoidItemDefinition.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/VoidItemDefinition.h"
#include "smtk/attribute/json/jsonItemDefinition.h"

#include "nlohmann/json.hpp"

#include <string>
using json = nlohmann::json;

/**\brief Provide a way to serialize VoidItemDefinitionPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(
  nlohmann::json& j,
  const smtk::attribute::VoidItemDefinitionPtr& defPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ItemDefinition>(defPtr));
}

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j,
  smtk::attribute::VoidItemDefinitionPtr& defPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto itemDef = smtk::dynamic_pointer_cast<ItemDefinition>(defPtr);
  smtk::attribute::from_json(j, itemDef);
}
} // namespace attribute
} // namespace smtk
