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

#include "smtk/io/Logger.h"

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
    { "Name", itemDefPtr->name() }, { "TypeName", itemDefPtr->typeName() },
    { "Version", itemDefPtr->version() },
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
  if (itemDefPtr->hasLocalAdvanceLevelInfo(0))
  {
    j["AdvanceReadLevel"] = itemDefPtr->localAdvanceLevel(0);
  }
  if (itemDefPtr->hasLocalAdvanceLevelInfo(1))
  {
    j["AdvanceWriteLevel"] = itemDefPtr->localAdvanceLevel(1);
  }
  if (!itemDefPtr->localCategories().empty())
  {
    smtk::attribute::Categories::Set& localCats = itemDefPtr->localCategories();
    if (localCats.mode() == smtk::attribute::Categories::Set::CombinationMode::All)
    {
      j["categoryCheckMode"] = "All";
    }
    else
    {
      j["categoryCheckMode"] = "Any";
    }
    j["Categories"] = localCats.categoryNames();
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

  //Write out tag information
  const auto& tags = itemDefPtr->tags();
  if (!tags.empty())
  {
    std::map<std::string, std::set<std::string> > tagInfo;
    for (const auto& tag : tags)
    {
      tagInfo[tag.name()] = tag.values();
    }

    j["Tags"] = tagInfo;
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
  auto result = j.find("Label");
  if (result != j.end())
  {
    itemDefPtr->setLabel(*result);
  }
  result = j.find("Version");
  if (result != j.end())
  {
    itemDefPtr->setVersion(*result);
  }
  result = j.find("Optional");
  if (result != j.end())
  {
    itemDefPtr->setIsOptional(*result);
  }
  result = j.find("isEnabledByDefault");
  if (result != j.end())
  {
    itemDefPtr->setIsEnabledByDefault(*result);
  }
  result = j.find("AdvanceLevel");
  if (result != j.end())
  {
    itemDefPtr->setLocalAdvanceLevel(0, *result);
    itemDefPtr->setLocalAdvanceLevel(1, *result);
  }
  else
  {
    auto val = j.find("AdvanceReadLevel");
    if (val != j.end())
    {
      itemDefPtr->setLocalAdvanceLevel(0, *val);
    }
    val = j.find("AdvanceWriteLevel");
    if (val != j.end())
    {
      itemDefPtr->setLocalAdvanceLevel(1, *val);
    }
  }

  result = j.find("BriefDescription");
  if (result != j.end())
  {
    itemDefPtr->setBriefDescription(*result);
  }
  result = j.find("DetailedDescription");
  if (result != j.end())
  {
    itemDefPtr->setDetailedDescription(*result);
  }
  auto categories = j.find("Categories");
  if (categories != j.end())
  {
    smtk::attribute::Categories::Set& localCats = itemDefPtr->localCategories();
    result = j.find("categoryCheckMode");
    // If categoryCheckMode is not specified - assume the default value;
    if (result != j.end())
    {
      if (*result == "All")
      {
        localCats.setMode(smtk::attribute::Categories::Set::CombinationMode::All);
      }
      else if (*result == "Any")
      {
        localCats.setMode(smtk::attribute::Categories::Set::CombinationMode::Any);
      }
      else
      {
        smtkErrorMacro(smtk::io::Logger::instance(), "When converting json, itemDefinition "
            << itemDefPtr->name() << " has an invalid categoryCheckMode = " << *result);
      }
    }
    for (const auto& category : *categories)
    {
      localCats.insert(category);
    }
  }
  auto okToInherit = j.find("OkToInheritCategories");
  if (okToInherit != j.end())
  {
    itemDefPtr->setIsOkToInherit(*okToInherit);
  }
  result = j.find("Tags");
  if (result != j.end())
  {
    std::map<std::string, std::set<std::string> > tagInfo = *result;
    for (const auto& t : tagInfo)
    {
      smtk::attribute::Tag tag(t.first, t.second);
      itemDefPtr->addTag(tag);
    }
  }
}
}
}
