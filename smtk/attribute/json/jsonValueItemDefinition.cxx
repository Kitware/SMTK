//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonValueItemDefinition.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/json/jsonComponentItemDefinition.h"
#include "smtk/attribute/json/jsonHelperFunction.h"
#include "smtk/attribute/json/jsonItemDefinition.h"

#include "nlohmann/json.hpp"

#include <string>
using json = nlohmann::json;

/**\brief Provide a way to serialize itemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(
  nlohmann::json& j,
  const smtk::attribute::ValueItemDefinitionPtr& defPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ItemDefinition>(defPtr));
  j["NumberOfRequiredValues"] = defPtr->numberOfRequiredValues();
  if (defPtr->isExtensible())
  {
    j["Extensible"] = true;
    if (defPtr->maxNumberOfValues())
    {
      j["MaxNumberOfValues"] = defPtr->maxNumberOfValues();
    }
  }
  if (defPtr->hasValueLabels())
  {
    json valueLabel;
    if (defPtr->usingCommonLabel())
    {
      valueLabel["CommonLabel"] = defPtr->valueLabel(0);
    }
    else
    {
      for (size_t index = 0; index < defPtr->numberOfRequiredValues(); index++)
      {
        valueLabel["Label"].push_back(defPtr->valueLabel(index));
      }
    }
    j["ComponentLabels"] = valueLabel;
  }
  if (defPtr->allowsExpressions())
  {
    j["ExpressionInformation"] = defPtr->expressionInformation();
  }
  if (!defPtr->units().empty())
  {
    j["Units"] = defPtr->units();
  }
  // Now let's process its children Items
  if (!defPtr->numberOfChildrenItemDefinitions())
  {
    return;
  }
  json childDefs;
  std::map<std::string, smtk::attribute::ItemDefinitionPtr>::const_iterator iter;
  for (iter = defPtr->childrenItemDefinitions().begin();
       iter != defPtr->childrenItemDefinitions().end();
       ++iter)
  {
    json childDef;
    smtk::attribute::JsonHelperFunction::processItemDefinitionTypeToJson(childDef, iter->second);
    // Same type definitions can occur multiple times
    childDefs.push_back(childDef);
  }
  j["ChildrenDefinitions"] = childDefs;
}

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j,
  smtk::attribute::ValueItemDefinitionPtr& defPtr,
  const smtk::attribute::ResourcePtr& resPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto itemDef = smtk::dynamic_pointer_cast<ItemDefinition>(defPtr);
  smtk::attribute::from_json(j, itemDef);
  auto result = j.find("NumberOfRequiredValues");
  if (result != j.end())
  {
    defPtr->setNumberOfRequiredValues(*result);
  }

  result = j.find("Extensible");
  if (result != j.end())
  {
    defPtr->setIsExtensible(*result);
  }

  result = j.find("MaxNumberOfValues");
  if (result != j.end())
  {
    defPtr->setMaxNumberOfValues(*result);
  }

  result = j.find("ComponentLabels");
  if (result != j.end())
  {
    auto common = result->find("CommonLabel");
    if (common != result->end())
    {
      defPtr->setCommonValueLabel(*common);
    }
    else
    {
      auto labels = result->find("Label");
      int i(0);
      if (labels != result->end())
      {
        for (auto iterator = labels->begin(); iterator != labels->end(); iterator++, i++)
        {
          defPtr->setValueLabel(i, (*iterator).get<std::string>());
        }
      }
    }
  }

  result = j.find("ExpressionType");
  if (result != j.end())
  {
    defPtr->setExpressionType(*result);
  }

  result = j.find("ExpressionInformation");
  if (result != j.end())
  {
    smtk::attribute::from_json(*result, defPtr->expressionInformation(), resPtr);
  }

  result = j.find("Units");
  if (result != j.end())
  {
    defPtr->setUnits(*result);
  }

  // Now let's process its children items
  result = j.find("ChildrenDefinitions");
  if (result != j.end())
  {
    for (const auto& jIdef : *result)
    {
      smtk::attribute::JsonHelperFunction::processItemDefinitionTypeFromJson(jIdef, defPtr, resPtr);
    }
  }
}
} // namespace attribute
} // namespace smtk
