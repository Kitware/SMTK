//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonItem.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

#include "nlohmann/json.hpp"
#include "smtk/CoreExports.h"

#include <string>
using json = nlohmann::json;

/**\brief Provide a way to serialize itemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::ItemPtr& itemPtr)
{
  j["Name"] = itemPtr->name();
  if (itemPtr->definition()->isOptional())
  {
    j["Enabled"] = itemPtr->localEnabledState();
    j["ForceRequired"] = itemPtr->forceRequired();
  }
  // Does the item have explicit advance level information
  if (itemPtr->hasLocalAdvanceLevelInfo(0))
  {
    j["AdvanceReadLevel"] = itemPtr->localAdvanceLevel(0);
  }

  if (itemPtr->hasLocalAdvanceLevelInfo(1))
  {
    j["AdvanceWriteLevel"] = itemPtr->localAdvanceLevel(1);
  }
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::ItemPtr& itemPtr)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr.get())
  {
    return;
  }

  auto result = j.find("Enabled");
  if (result != j.end())
  {
    itemPtr->setIsEnabled(*result);
  }

  result = j.find("ForceRequired");
  if (result != j.end())
  {
    itemPtr->setForceRequired(*result);
  }

  result = j.find("AdvanceReadLevel");
  if (result != j.end())
  {
    itemPtr->setLocalAdvanceLevel(0, *result);
  }

  result = j.find("AdvanceWriteLevel");
  if (result != j.end())
  {
    itemPtr->setLocalAdvanceLevel(1, *result);
  }
}
} // namespace attribute
} // namespace smtk
