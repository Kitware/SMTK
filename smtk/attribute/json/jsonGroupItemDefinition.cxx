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
using ItemExpressionDefInfo = std::pair<smtk::attribute::ValueItemDefinitionPtr, std::string>;

using AttRefDefInfo = std::pair<smtk::attribute::RefItemDefinitionPtr, std::string>;

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

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::GroupItemDefinitionPtr& defPtr,
  const smtk::attribute::ResourcePtr& resPtr, std::vector<ItemExpressionDefInfo>& expressionDefInfo,
  std::vector<AttRefDefInfo>& attRefDefInfo)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto temp = smtk::dynamic_pointer_cast<ItemDefinition>(defPtr);
  smtk::attribute::from_json(j, temp);

  try
  {
    defPtr->setNumberOfRequiredGroups(j.at("NumberOfRequiredGroups"));
  }
  catch (std::exception)
  {
  }
  try
  {
    defPtr->setIsExtensible(j.at("Extensible"));
  }
  catch (std::exception)
  {
  }
  try
  {
    defPtr->setMaxNumberOfGroups(j.at("MaxNumberOfGroups"));
  }
  catch (std::exception)
  {
  }
  json clabels;
  try
  {
    clabels = j.at("ComponentLabels");
    if (!clabels.is_null())
    {
      // Nested try/catch
      try
      {
        defPtr->setCommonSubGroupLabel(clabels.at("CommonLabel"));
      }
      catch (std::exception& /*e*/)
      {
      }
    }
  }
  catch (std::exception& /*e*/)
  {
  }
  if (!clabels.is_null())
  {
    try
    {
      json labels = clabels.at("Label");
      int i(0);
      for (auto iterator = labels.begin(); iterator != labels.end(); iterator++, i++)
      {
        defPtr->setSubGroupLabel(i, (*iterator).get<std::string>());
      }
    }
    catch (std::exception& /*e*/)
    {
    }
  }

  // Now let's process its children items
  try
  {
    json childrenDefs = j.at("ItemDefinitions");
    for (json::iterator iter = childrenDefs.begin(); iter != childrenDefs.end(); iter++)
    {
      smtk::attribute::JsonHelperFunction::processItemDefinitionTypeFromJson(
        iter, defPtr, resPtr, expressionDefInfo, attRefDefInfo);
    }
  }
  catch (std::exception& /*e*/)
  {
  }
}
}
}
