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

#include "json.hpp"
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
  if (itemPtr->isOptional())
  {
    j["Enabled"] = itemPtr->isEnabled();
  }
  // Does the item have explicit advance level information
  if (!itemPtr->usingDefinitionAdvanceLevel(0))
  {
    j["AdvanceReadLevel"] = itemPtr->advanceLevel(0);
  }

  if (!itemPtr->usingDefinitionAdvanceLevel(1))
  {
    j["AdvanceWriteLevel"] = itemPtr->advanceLevel(1);
  }
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::ItemPtr& itemPtr)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr.get())
  {
    return;
  }
  if (itemPtr->isOptional())
  {
    try
    {
      itemPtr->setIsEnabled(j.at("Enabled"));
    }
    catch (std::exception& /*e*/)
    {
    }
  }

  json advanceLevel;
  try
  {
    advanceLevel = j.at("AdvanceLevel");
    itemPtr->setAdvanceLevel(0, j.at("AdvanceLevel"));
    itemPtr->setAdvanceLevel(1, j.at("AdvanceLevel"));
  }
  catch (std::exception& /*e*/)
  {
  }
  if (advanceLevel.is_null())
  {
    try
    {
      itemPtr->setAdvanceLevel(0, j.at("AdvanceReadLevel"));
    }
    catch (std::exception& /*e*/)
    {
    }
    try
    {
      itemPtr->setAdvanceLevel(1, j.at("AdvanceWriteLevel"));
    }
    catch (std::exception& /*e*/)
    {
    }
  }
}
}
}
