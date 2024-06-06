//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonDoubleItem.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/json/jsonValueItem.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize DoubleItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::DoubleItemPtr& itemPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ValueItem>(itemPtr));
  smtk::attribute::processDerivedValueToJson(j, itemPtr);
  // if the item has explicit units we need to save them
  if (itemPtr->hasExplicitUnits())
  {
    j["Units"] = itemPtr->units();
  }
}

SMTKCORE_EXPORT void from_json(
  const json& j,
  smtk::attribute::DoubleItemPtr& itemPtr,
  std::vector<ItemExpressionInfo>& itemExpressionInfo,
  std::vector<AttRefInfo>& attRefInfo)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr.get())
  {
    return;
  }

  // See if there are units
  auto result = j.find("Units");
  if (result != j.end())
  {
    itemPtr->setUnits(*result);
  }

  auto valItem = smtk::dynamic_pointer_cast<ValueItem>(itemPtr);
  smtk::attribute::from_json(j, valItem, itemExpressionInfo, attRefInfo);
  smtk::attribute::processDerivedValueFromJson<smtk::attribute::DoubleItemPtr, double>(
    j, itemPtr, itemExpressionInfo, attRefInfo);
}
} // namespace attribute
} // namespace smtk
