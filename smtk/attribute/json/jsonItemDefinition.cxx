//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonItemDefinition.h"
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
  nlohmann::json& j, const smtk::attribute::ItemDefinitionPtr& itemDefPtr)
{
  j = {
    { "Name", itemDefPtr->name() }, { "Version", itemDefPtr->version() },
  };
  if (!itemDefPtr->label().empty())
  {
    j["Label"] = itemDefPtr->label();
  }
  if (itemDefPtr->isOptional())
  {
    j["Optional"] = true;
    j["isEnabledByDefault"] = itemDefPtr->isEnabledByDefault();
  }
  if (itemDefPtr->advanceLevel(0) || itemDefPtr->advanceLevel(1))
  {
    // OK - we have a non-zero advance level in either read or write
    // if they are both set the same use the AdvanceLevel xml attribute
    if (itemDefPtr->advanceLevel(0) == itemDefPtr->advanceLevel(1))
    {
      j["AdvanceLevel"] = itemDefPtr->advanceLevel(0);
    }
    else
    {
      if (itemDefPtr->advanceLevel(0))
      {
        j["AdvanceReadLevel"] = itemDefPtr->advanceLevel(0);
      }
      if (itemDefPtr->advanceLevel(1))
      {
        j["AdvanceWriteLevel"] = itemDefPtr->advanceLevel(1);
      }
    }
  }
  if (itemDefPtr->numberOfCategories() && (itemDefPtr->type() != Item::GroupType))
  {
    std::set<std::string> cats = itemDefPtr->categories();
    j["Categories"] = itemDefPtr->categories();
  }
  if (!itemDefPtr->briefDescription().empty())
  {
    j["BriefDescription"] = itemDefPtr->briefDescription();
  }
  if (!itemDefPtr->detailedDescription().empty())
  {
    j["DetailedDescription"] = itemDefPtr->detailedDescription();
  }
}

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j, smtk::attribute::ItemDefinitionPtr& itemDefPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!itemDefPtr.get())
  {
    return;
  }
  try
  {
    itemDefPtr->setLabel(j.at("Label"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    itemDefPtr->setVersion(j.at("Version"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    itemDefPtr->setIsOptional(j.at("Optional"));
    itemDefPtr->setIsEnabledByDefault(j.at("isEnabledByDefault"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    itemDefPtr->setAdvanceLevel(0, j.at("AdvanceLevel"));
    itemDefPtr->setAdvanceLevel(1, j.at("AdvanceLevel"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    itemDefPtr->setAdvanceLevel(0, j.at("AdvanceReadLevel"));
    itemDefPtr->setAdvanceLevel(1, j.at("AdvanceWriteLevel"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    itemDefPtr->setBriefDescription(j.at("BriefDescription"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    itemDefPtr->setDetailedDescription(j.at("DetailedDescription"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    std::vector<std::string> categories = j.at("Categories");
    for (const auto& category : categories)
    {
      itemDefPtr->addCategory(category);
    }
  }
  catch (std::exception& /*e*/)
  {
  }
  // TODO defaultCategories?
  // Reference: XmlDocV1Parser::process L478
}
}
}
