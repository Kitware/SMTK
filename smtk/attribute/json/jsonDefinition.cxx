//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonDefinition.h"
#include "smtk/PublicPointerDefs.h"

#include "nlohmann/json.hpp"

#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/attribute/Resource.h"

#include "smtk/attribute/json/jsonAttribute.h"
#include "smtk/attribute/json/jsonHelperFunction.h"
#include "smtk/attribute/json/jsonItem.h"
#include "smtk/attribute/json/jsonReferenceItemDefinition.h"

#include <exception>
#include <string>

namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(nlohmann::json& j, const smtk::attribute::DefinitionPtr& defPtr)
{
  j["Type"] = defPtr->type();
  j["ID"] = defPtr->id().toString();
  if (!defPtr->label().empty())
  {
    j["Label"] = defPtr->label();
  }
  if (defPtr->baseDefinition())
  {
    j["BaseType"] = defPtr->baseDefinition()->type();
  }
  else
  {
    j["BaseType"] = "";
  }
  j["Version"] = defPtr->version();
  if (defPtr->isAbstract())
  {
    j["Abstract"] = true;
  }
  // Does the Definition have explicit advance level information
  if (defPtr->hasLocalAdvanceLevelInfo(0))
  {
    j["AdvanceReadLevel"] = defPtr->localAdvanceLevel(0);
  }

  if (defPtr->hasLocalAdvanceLevelInfo(1))
  {
    j["AdvanceWriteLevel"] = defPtr->localAdvanceLevel(1);
  }
  if (defPtr->isUnique())
  { // true is the default
    j["Unique"] = true;
  }
  else
  {
    j["Unique"] = false;
  }

  j["IgnoreCategories"] = defPtr->ignoreCategories();

  if (defPtr->rootName() != defPtr->type())
  {
    j["RootName"] = defPtr->rootName();
  }
  if (defPtr->isNodal())
  {
    j["Nodal"] = true;
  }

  // Does the Definition have local units?
  if (!defPtr->localUnits().empty())
  {
    j["Units"] = defPtr->localUnits();
  }

  // Process Local Category Expression
  if (defPtr->localCategories().isSet())
  {
    // If the expression string is not empty save it, else we are dealing with
    // a trivial All Pass/Reject constraint
    if (!defPtr->localCategories().expression().empty())
    {
      j["CategoryExpression"]["Expression"] = defPtr->localCategories().expression();
    }
    else if (defPtr->localCategories().allPass())
    {
      j["CategoryExpression"]["PassMode"] = "All";
    }
    else
    {
      j["CategoryExpression"]["PassMode"] = "None";
    }
  }

  // Inheritance Option - This is always needed
  j["CategoryExpression"]["InheritanceMode"] =
    smtk::common::Categories::combinationModeAsString(defPtr->categoryInheritanceMode());

  // Save Color Information
  if (defPtr->isNotApplicableColorSet())
  {
    const double* rgba = defPtr->notApplicableColor();
    j["NotApplicableColor"] = { rgba[0], rgba[1], rgba[2], rgba[3] };
  }
  if (defPtr->isDefaultColorSet())
  {
    const double* rgba = defPtr->defaultColor();
    j["DefaultColor"] = { rgba[0], rgba[1], rgba[2], rgba[3] };
  }

  auto assocRule = defPtr->localAssociationRule();
  if (assocRule)
  {
    // Create association element if we need to.
    j["AssociationsDef"] = assocRule;
  }

  if (!defPtr->briefDescription().empty())
  {
    j["BriefDescription"] = defPtr->briefDescription();
  }
  if (!defPtr->detailedDescription().empty())
  {
    j["DetailedDescription"] = defPtr->detailedDescription();
  }
  // Now lets process its items
  std::size_t n = defPtr->numberOfItemDefinitions();
  // Does this defPtrinition have items not derived from its base defPtr?
  if (n != defPtr->itemOffset())
  {
    json itemDefs;
    for (std::size_t i = defPtr->itemOffset(); i < n; i++)
    {
      json itemDef;
      smtk::attribute::ItemDefinitionPtr itemDPtr = defPtr->itemDefinition(static_cast<int>(i));
      smtk::attribute::JsonHelperFunction::processItemDefinitionTypeToJson(itemDef, itemDPtr);
      // Same type definitions can occur multiple times
      itemDefs.push_back(itemDef);
    }
    j["ItemDefinitions"] = itemDefs;
  }
  //Write out tag information
  const auto& tags = defPtr->tags();
  if (!tags.empty())
  {
    std::map<std::string, std::set<std::string>> tagInfo;
    for (const auto& tag : tags)
    {
      tagInfo[tag.name()] = tag.values();
    }

    j["Tags"] = tagInfo;
  }

  smtk::attribute::ResourcePtr attResource = defPtr->attributeResource();
  if (!attResource)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "When converting to json, definition " << defPtr->label() << " has an invalid resourcePtr");
    return;
  }

  auto associationRuleForDef =
    attResource->associationRules().associationRulesForDefinitions().find(defPtr->type());
  if (
    associationRuleForDef != attResource->associationRules().associationRulesForDefinitions().end())
  {
    j["AssociationRule"] = associationRuleForDef->second;
  }

  auto dissociationRuleForDef =
    attResource->associationRules().dissociationRulesForDefinitions().find(defPtr->type());
  if (
    dissociationRuleForDef !=
    attResource->associationRules().dissociationRulesForDefinitions().end())
  {
    j["DissociationRule"] = dissociationRuleForDef->second;
  }
}

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j,
  smtk::attribute::DefinitionPtr& defPtr,
  std::set<const smtk::attribute::ItemDefinition*>& convertedAttDefs)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  smtk::attribute::ResourcePtr attResource = defPtr->attributeResource();
  if (attResource == nullptr)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "When converting json, definition " << defPtr->label() << " has an invalid resourcePtr");
    return;
  }
  // Same logic in XmlDocV1Parser::processDefinition
  auto result = j.find("Label");
  if (result != j.end())
  {
    defPtr->setLabel(*result);
  }

  result = j.find("Version");
  if (result != j.end())
  {
    defPtr->setVersion(*result);
  }

  result = j.find("Abstract");
  if (result != j.end())
  {
    defPtr->setIsAbstract(*result);
  }

  result = j.find("AdvanceLevel");
  if (result != j.end())
  {
    defPtr->setLocalAdvanceLevel(*result);
  }
  else
  {
    result = j.find("AdvanceReadLevel");
    if (result != j.end())
    {
      defPtr->setLocalAdvanceLevel(0, *result);
    }
    result = j.find("AdvanceWriteLevel");
    if (result != j.end())
    {
      defPtr->setLocalAdvanceLevel(1, *result);
    }
  }

  result = j.find("Unique");
  if (result != j.end())
  {
    defPtr->setIsUnique(*result);
  }

  result = j.find("IgnoreCategories");
  if (result != j.end())
  {
    defPtr->setIgnoreCategories(*result);
  }

  result = j.find("Units");
  if (result != j.end())
  {
    defPtr->setLocalUnits(*result);
  }

  // Process Category Info ()
  smtk::common::Categories::CombinationMode cmode;
  auto catExp = j.find("CategoryExpression"); // Current Form
  auto catInfo = j.find("CategoryInfo");      // Old Form since 05/24
  auto categories = j.find("Categories");     // Old Deprecated Form

  if (catExp != j.end())
  {
    auto inheritanceMode = catExp->find("InheritanceMode");
    if (inheritanceMode != catExp->end())
    {
      if (smtk::common::Categories::combinationModeFromString(*inheritanceMode, cmode))
      {
        defPtr->setCategoryInheritanceMode(cmode);
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "When converting json, definition "
            << defPtr->label()
            << " has an invalid category inheritance mode = " << *inheritanceMode);
      }
    }
    // Is this a trivial reject / pass constraint
    auto passMode = catExp->find("PassMode");
    if (passMode != catExp->end())
    {
      if (*passMode == "All")
      {
        defPtr->localCategories().setAllPass();
      }
      else if (*passMode == "None")
      {
        defPtr->localCategories().setAllReject();
      }
    }
    else
    {
      auto catExpression = catExp->find("Expression");
      if (catExpression != catExp->end())
      {
        defPtr->localCategories().setExpression(*catExpression);
      }
    }
  }
  else if (catInfo != j.end())
  {
    auto inheritanceMode = catInfo->find("InheritanceMode");
    if (inheritanceMode != catInfo->end())
    {
      if (smtk::common::Categories::combinationModeFromString(*inheritanceMode, cmode))
      {
        defPtr->setCategoryInheritanceMode(cmode);
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "When converting json, definition "
            << defPtr->label()
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
          defPtr->setCategoryInheritanceMode(smtk::common::Categories::CombinationMode::Or);
        }
        else
        {
          defPtr->setCategoryInheritanceMode(smtk::common::Categories::CombinationMode::LocalOnly);
        }
      }
    }
    common::Categories::Set& localCats = defPtr->localCategories();
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
            << defPtr->label() << " has an invalid top level combination mode = " << *combineMode);
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
            << defPtr->label() << " has an invalid inclusion combination mode = " << *combineMode);
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
            << defPtr->label() << " has an invalid exclusion combination mode = " << *combineMode);
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
  else if (categories != j.end()) // Deprecated
  {
    common::Categories::Set& localCats = defPtr->localCategories();
    smtk::common::Categories::CombinationMode cmode;
    auto categoryCheckMode = j.find("categoryCheckMode");
    // If categoryCheckMode is not specified - assume the default value;
    if (categoryCheckMode != j.end())
    {
      if (smtk::common::Categories::combinationModeFromString(*categoryCheckMode, cmode))
      {
        localCats.setInclusionMode(cmode);
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "When converting json, definition "
            << defPtr->label() << " has an invalid InclusionMode = " << *categoryCheckMode);
      }
    }
    for (const auto& category : *categories)
    {
      localCats.insertInclusion(category);
    }
  }

  result = j.find("Nodal");
  if (result != j.end())
  {
    defPtr->setIsNodal(*result);
  }

  // Read old-style association mask first.  Note that the association is set
  // as extensible.
  // It will be overwritten if a new-style AssociationsDef
  // is also provided.
  // Reference: XmlDocV1Parser:: L744

  result = j.find("Associations");
  if (result != j.end())
  {
    smtk::model::BitFlags mask = smtk::model::Entity::specifierStringToFlag(*result);
    defPtr->setLocalAssociationMask(mask);
    defPtr->localAssociationRule()->setIsExtensible(true);
  }

  result = j.find("NotApplicableColor");
  if ((result != j.end()) && (result->size() == 4))
  {
    defPtr->setNotApplicableColor((*result)[0], (*result)[1], (*result)[2], (*result)[3]);
  }

  result = j.find("DefaultColor");
  if ((result != j.end()) && (result->size() == 4))
  {
    defPtr->setDefaultColor((*result)[0], (*result)[1], (*result)[2], (*result)[3]);
  }

  result = j.find("BriefDescription");
  if (result != j.end())
  {
    defPtr->setBriefDescription(*result);
  }

  result = j.find("DetailedDescription");
  if (result != j.end())
  {
    defPtr->setDetailedDescription(*result);
  }

  result = j.find("AssociationsDef");
  if (result != j.end())
  {
    // TODO: Check XmlDocV1Parser 789
    std::string aname;
    auto assocName = result->find("Name");
    if (assocName != result->end())
    {
      aname = assocName->get<std::string>();
    }
    else
    {
      aname = defPtr->type() + "Associations";
    }

    auto assocRule = smtk::attribute::ReferenceItemDefinition::New(aname);
    smtk::attribute::from_json(*result, assocRule, attResource);
    defPtr->setLocalAssociationRule(assocRule);
  }

  result = j.find("ItemDefinitions");
  if (result != j.end())
  {
    // Reference: Check XmlDocV1Parser 789
    for (const auto& idef : *result)
    {
      smtk::attribute::JsonHelperFunction::processItemDefinitionTypeFromJson(
        idef, defPtr, attResource, convertedAttDefs);
    }
  }

  result = j.find("RootName");
  if (result != j.end())
  {
    defPtr->setRootName(*result);
  }

  result = j.find("Tags");
  if (result != j.end())
  {
    std::map<std::string, std::set<std::string>> tagInfo = *result;
    for (const auto& t : tagInfo)
    {
      smtk::attribute::Tag tag(t.first, t.second);
      defPtr->addTag(tag);
    }
  }

  result = j.find("AssociationRule");
  if (result != j.end())
  {
    attResource->associationRules().associationRulesForDefinitions().emplace(
      defPtr->type(), result->get<std::string>());
  }

  result = j.find("DissociationRule");
  if (result != j.end())
  {
    attResource->associationRules().dissociationRulesForDefinitions().emplace(
      defPtr->type(), result->get<std::string>());
  }
}
} // namespace attribute
} // namespace smtk
