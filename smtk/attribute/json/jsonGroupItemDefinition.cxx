//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonGroupItemDefinition.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/json/jsonHelperFunction.h"
#include "smtk/attribute/json/jsonItemDefinition.h"

#include "nlohmann/json.hpp"

#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize GroupItemDefinitionPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::GroupItemDefinitionPtr& defPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ItemDefinition>(defPtr));
  j["NumberOfRequiredGroups"] = defPtr->numberOfRequiredGroups();
  if (defPtr->isExtensible())
  {
    j["Extensible"] = true;
    if (defPtr->maxNumberOfGroups())
    {
      j["MaxNumberOfGroups"] = defPtr->maxNumberOfGroups();
    }
  }

  if (defPtr->isConditional())
  {
    j["IsConditional"] = true;
    if (defPtr->minNumberOfChoices())
    {
      j["MinNumberOfChoices"] = defPtr->minNumberOfChoices();
    }
    if (defPtr->maxNumberOfChoices())
    {
      j["MaxNumberOfChoices"] = defPtr->maxNumberOfChoices();
    }
  }

  if (defPtr->hasSubGroupLabels())
  {
    json valueLabel;
    if (defPtr->usingCommonSubGroupLabel())
    {
      valueLabel["CommonLabel"] = defPtr->subGroupLabel(0);
    }
    else
    {
      for (size_t index = 0; index < defPtr->numberOfRequiredGroups(); index++)
      {
        valueLabel["Label"].push_back(defPtr->subGroupLabel(index));
      }
    }
    j["ComponentLabels"] = valueLabel;
  }

  // Now lets process its items
  int i, n = static_cast<int>(defPtr->numberOfItemDefinitions());
  if (n != 0)
  {
    json itemDefs;
    for (i = 0; i < n; i++)
    {
      json itemDef;
      smtk::attribute::JsonHelperFunction::processItemDefinitionTypeToJson(
        itemDef, defPtr->itemDefinition(i));
      // Same type definitions can occur multiple times
      itemDefs.push_back(itemDef);
    }
    j["ItemDefinitions"] = itemDefs;
  }
}

SMTKCORE_EXPORT void from_json(
  const json& j,
  smtk::attribute::GroupItemDefinitionPtr& defPtr,
  const smtk::attribute::ResourcePtr& resPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto itemDef = smtk::dynamic_pointer_cast<ItemDefinition>(defPtr);
  smtk::attribute::from_json(j, itemDef);

  auto result = j.find("NumberOfRequiredGroups");
  if (result != j.end())
  {
    defPtr->setNumberOfRequiredGroups(*result);
  }

  result = j.find("Extensible");
  if (result != j.end())
  {
    defPtr->setIsExtensible(*result);
  }

  result = j.find("IsConditional");
  if (result != j.end())
  {
    defPtr->setIsConditional(*result);
  }

  result = j.find("MinNumberOfChoices");
  if (result != j.end())
  {
    defPtr->setMinNumberOfChoices(*result);
  }

  result = j.find("MaxNumberOfChoices");
  if (result != j.end())
  {
    defPtr->setMaxNumberOfChoices(*result);
  }

  result = j.find("MaxNumberOfGroups");
  if (result != j.end())
  {
    defPtr->setMaxNumberOfGroups(*result);
  }

  result = j.find("ComponentLabels");
  if (result != j.end())
  {
    auto common = result->find("CommonLabel");
    if (common != result->end())
    {
      defPtr->setCommonSubGroupLabel(*common);
    }
    else
    {
      auto labels = result->find("Label");
      int i(0);
      if (labels != result->end())
      {
        for (auto iterator = labels->begin(); iterator != labels->end(); iterator++, i++)
        {
          defPtr->setSubGroupLabel(i, (*iterator).get<std::string>());
        }
      }
    }
  }

  // Now let's process its children items
  result = j.find("ItemDefinitions");
  if (result != j.end())
  {
    for (auto& jIdef : *result)
    {
      smtk::attribute::JsonHelperFunction::processItemDefinitionTypeFromJson(jIdef, defPtr, resPtr);
    }
  }
}
} // namespace attribute
} // namespace smtk
