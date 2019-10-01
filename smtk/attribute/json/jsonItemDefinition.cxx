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
    { "Type", smtk::attribute::Item::type2String(itemDefPtr->type()) },
    { "Name", itemDefPtr->name() }, { "Version", itemDefPtr->version() },
  };
  if (itemDefPtr->label() != itemDefPtr->name())
  {
    j["Label"] = itemDefPtr->label();
  }
  if (itemDefPtr->isOptional())
  {
    j["Optional"] = true;
    j["isEnabledByDefault"] = itemDefPtr->isEnabledByDefault();
  }
  if (itemDefPtr->categoryCheckMode() == smtk::attribute::ItemDefinition::CategoryCheckMode::All)
  {
    j["categoryCheckMode"] = "All";
  }
  else
  {
    j["categoryCheckMode"] = "Any";
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
  if (!itemDefPtr->localCategories().empty())
  {
    j["Categories"] = itemDefPtr->localCategories();
  }
  j["OkToInheritCategories"] = itemDefPtr->isOkToInherit();
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
  auto located = j.find("Label");
  if (located != j.end())
  {
    itemDefPtr->setLabel(*located);
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
  located = j.find("categoryCheckMode");
  if (located != j.end())
  {
    if (*located == "All")
    {
      itemDefPtr->setCategoryCheckMode(smtk::attribute::ItemDefinition::CategoryCheckMode::All);
    }
    else
    {
      itemDefPtr->setCategoryCheckMode(smtk::attribute::ItemDefinition::CategoryCheckMode::Any);
    }
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

  auto categories = j.find("Categories");
  if (categories != j.end())
  {
    for (const auto& category : *categories)
    {
      itemDefPtr->addLocalCategory(category);
    }
  }
  auto okToInherit = j.find("OkToInheritCategories");
  if (okToInherit != j.end())
  {
    itemDefPtr->setIsOkToInherit(*okToInherit);
  }
}
}
}
