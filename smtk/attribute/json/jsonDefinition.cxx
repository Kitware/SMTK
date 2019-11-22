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
  { // true is the defPtrault
    j["Unique"] = true;
  }
  else
  {
    j["Unique"] = false;
  }
  if (defPtr->rootName() != defPtr->type())
  {
    j["RootName"] = defPtr->rootName();
  }
  if (defPtr->isNodal())
  {
    j["Nodal"] = true;
  }
  if (!defPtr->localCategories().empty())
  {
    j["Categories"] = defPtr->localCategories();
  }
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
}

SMTKCORE_EXPORT void from_json(const nlohmann::json& j, smtk::attribute::DefinitionPtr& defPtr,
  std::set<const smtk::attribute::ItemDefinition*>& convertedAttDefs)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  smtk::attribute::ResourcePtr colPtr =
    std::dynamic_pointer_cast<smtk::attribute::Resource>(defPtr->resource());
  if (colPtr == nullptr)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "When converting json, definition "
        << defPtr->label() << " has an invalid resourcePtr");
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

  auto categories = j.find("Categories");
  if (categories != j.end())
  {
    for (const auto& category : *categories)
    {
      defPtr->addLocalCategory(category);
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
      aname = *assocName;
    }
    else
    {
      aname = defPtr->type() + "Associations";
    }

    auto assocRule = smtk::attribute::ReferenceItemDefinition::New(aname);
    smtk::attribute::from_json(*result, assocRule);
    defPtr->setLocalAssociationRule(assocRule);
  }

  result = j.find("ItemDefinitions");
  if (result != j.end())
  {
    smtk::attribute::ResourcePtr resource =
      std::dynamic_pointer_cast<smtk::attribute::Resource>(defPtr->resource());
    // Reference: Check XmlDocV1Parser 789
    for (auto& idef : *result)
    {
      smtk::attribute::JsonHelperFunction::processItemDefinitionTypeFromJson(
        idef, defPtr, resource, convertedAttDefs);
    }
  }

  result = j.find("RootName");
  if (result != j.end())
  {
    defPtr->setRootName(*result);
  }
}
}
}
