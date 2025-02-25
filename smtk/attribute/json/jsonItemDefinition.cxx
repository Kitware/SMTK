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
  nlohmann::json& j,
  const smtk::attribute::ItemDefinitionPtr& itemDefPtr)
{
  j = {
    { "Type", smtk::attribute::Item::type2String(itemDefPtr->type()) },
    { "Name", itemDefPtr->name() },
    { "TypeName", itemDefPtr->typeName() },
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
  // Process Local Category Expression
  if (itemDefPtr->localCategories().isSet())
  {
    // If the expression string is not empty save it, else we are dealing with
    // a trivial All Pass/Reject constraint
    if (!itemDefPtr->localCategories().expression().empty())
    {
      j["CategoryExpression"]["Expression"] = itemDefPtr->localCategories().expression();
    }
    else if (itemDefPtr->localCategories().allPass())
    {
      j["CategoryExpression"]["PassMode"] = "All";
    }
    else
    {
      j["CategoryExpression"]["PassMode"] = "None";
    }
  }

  // Inheritance Option - this is always needed
  j["CategoryExpression"]["InheritanceMode"] =
    smtk::common::Categories::combinationModeAsString(itemDefPtr->categoryInheritanceMode());

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
    std::map<std::string, std::set<std::string>> tagInfo;
    for (const auto& tag : tags)
    {
      tagInfo[tag.name()] = tag.values();
    }

    j["Tags"] = tagInfo;
  }
}

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j,
  smtk::attribute::ItemDefinitionPtr& itemDefPtr)
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

  // Process Category Information
  auto catExp = j.find("CategoryExpression"); // Current Form
  auto catInfo = j.find("CategoryInfo");      // Old Form since 05/24
  auto categories = j.find("Categories");     // Old Deprecated Form

  if (catExp != j.end())
  {
    smtk::common::Categories::CombinationMode cmode;
    auto inheritanceMode = catExp->find("InheritanceMode");
    if (inheritanceMode != catExp->end())
    {
      if (smtk::common::Categories::combinationModeFromString(*inheritanceMode, cmode))
      {
        itemDefPtr->setCategoryInheritanceMode(cmode);
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "When converting json, definition "
            << itemDefPtr->label()
            << " has an invalid category inheritance mode = " << *inheritanceMode);
      }
    }
    // Is this a trivial reject / pass constraint
    auto passMode = catExp->find("PassMode");
    if (passMode != catExp->end())
    {
      if (*passMode == "All")
      {
        itemDefPtr->localCategories().setAllPass();
      }
      else if (*passMode == "None")
      {
        itemDefPtr->localCategories().setAllReject();
      }
    }
    else
    {
      auto catExpression = catExp->find("Expression");
      if (catExpression != catExp->end())
      {
        itemDefPtr->localCategories().setExpression(*catExpression);
      }
    }
  }
  else if (catInfo != j.end())
  {
    smtk::common::Categories::CombinationMode cmode;
    auto inheritanceMode = catInfo->find("InheritanceMode");
    if (inheritanceMode != catInfo->end())
    {
      if (smtk::common::Categories::combinationModeFromString(*inheritanceMode, cmode))
      {
        itemDefPtr->setCategoryInheritanceMode(cmode);
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "When converting json, definition "
            << itemDefPtr->label()
            << " has an invalid category inheritance mode = " << *inheritanceMode);
      }
    }
    else
    {
      auto okToInherit = catInfo->find("Inherit");
      if (okToInherit != catInfo->end())
      {
        if (*okToInherit)
        {
          itemDefPtr->setCategoryInheritanceMode(smtk::common::Categories::CombinationMode::Or);
        }
        else
        {
          itemDefPtr->setCategoryInheritanceMode(
            smtk::common::Categories::CombinationMode::LocalOnly);
        }
      }
    }
    common::Categories::Set& localCats = itemDefPtr->localCategories();
    auto combineMode = catInfo->find("Combination");
    // If Combination is not specified - assume the default value;
    if (combineMode != catInfo->end())
    {
      if (smtk::common::Categories::combinationModeFromString(*combineMode, cmode))
      {
        localCats.setCombinationMode(cmode);
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "When converting json, definition "
            << itemDefPtr->label()
            << " has an invalid top level combination mode = " << *combineMode);
      }
    }
    // Lets process included categories
    combineMode = catInfo->find("InclusionCombination");
    if (combineMode != catInfo->end())
    {
      if (smtk::common::Categories::combinationModeFromString(*combineMode, cmode))
      {
        localCats.setInclusionMode(cmode);
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "When converting json, definition "
            << itemDefPtr->label()
            << " has an invalid inclusion combination mode = " << *combineMode);
      }
    }
    auto catsGroup = catInfo->find("IncludeCategories");
    if (catsGroup != catInfo->end())
    {
      for (const auto& category : *catsGroup)
      {
        localCats.insertInclusion(category);
      }
    }
    // Lets process excluded categories
    combineMode = catInfo->find("ExclusionCombination");
    if (combineMode != catInfo->end())
    {
      if (smtk::common::Categories::combinationModeFromString(*combineMode, cmode))
      {
        localCats.setExclusionMode(cmode);
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "When converting json, definition "
            << itemDefPtr->label()
            << " has an invalid exclusion combination mode = " << *combineMode);
      }
    }
    catsGroup = catInfo->find("ExcludeCategories");
    if (catsGroup != catInfo->end())
    {
      for (const auto& category : *catsGroup)
      {
        localCats.insertExclusion(category);
      }
    }
  }
  else if (categories != j.end()) // Deprecated Format
  {
    smtk::common::Categories::Set& localCats = itemDefPtr->localCategories();
    smtk::common::Categories::CombinationMode cmode;
    result = j.find("categoryCheckMode");
    // If categoryCheckMode is not specified - assume the default value;
    if (result != j.end())
    {
      if (smtk::common::Categories::combinationModeFromString(*result, cmode))
      {
        localCats.setInclusionMode(cmode);
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "When converting json, itemDefinition " << itemDefPtr->name()
                                                  << " has an invalid InclusionMode = " << *result);
      }
    }
    for (const auto& category : *categories)
    {
      localCats.insertInclusion(category);
    }
  }
  // Also part of the old format
  auto okToInherit = j.find("OkToInheritCategories");
  if (okToInherit != j.end())
  {
    if (*okToInherit)
    {
      itemDefPtr->setCategoryInheritanceMode(smtk::common::Categories::CombinationMode::Or);
    }
    else
    {
      itemDefPtr->setCategoryInheritanceMode(smtk::common::Categories::CombinationMode::LocalOnly);
    }
  }

  result = j.find("Tags");
  if (result != j.end())
  {
    std::map<std::string, std::set<std::string>> tagInfo = *result;
    for (const auto& t : tagInfo)
    {
      smtk::attribute::Tag tag(t.first, t.second);
      itemDefPtr->addTag(tag);
    }
  }
}
} // namespace attribute
} // namespace smtk
