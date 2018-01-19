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
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/ValueItemDefinition.h"
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
using ItemExpressionDefInfo = std::pair<smtk::attribute::ValueItemDefinitionPtr, std::string>;

using AttRefDefInfo = std::pair<smtk::attribute::RefItemDefinitionPtr, std::string>;

SMTKCORE_EXPORT void to_json(
  nlohmann::json& j, const smtk::attribute::ValueItemDefinitionPtr& defPtr)
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
    attribute::DefinitionPtr exp = defPtr->expressionDefinition();
    if (exp)
    {
      j["ExpressionType"] = exp->type();
    }
  }
  if (!defPtr->units().empty())
  {
    j["Units"] = defPtr->units();
  }
  // Now let's prodcess its children Items
  if (!defPtr->numberOfChildrenItemDefinitions())
  {
    return;
  }
  json childDefs;
  std::map<std::string, smtk::attribute::ItemDefinitionPtr>::const_iterator iter;
  for (iter = defPtr->childrenItemDefinitions().begin();
       iter != defPtr->childrenItemDefinitions().end(); ++iter)
  {
    json childDef;
    smtk::attribute::JsonHelperFunction::processItemDefinitionTypeToJson(childDef, iter->second);
    // Same type definitions can occur multiple times
    childDefs[Item::type2String(iter->second->type())].push_back(childDef);
  }
  j["ChildrenDefinitions"] = childDefs;
}

SMTKCORE_EXPORT void from_json(const nlohmann::json& j,
  smtk::attribute::ValueItemDefinitionPtr& defPtr, const smtk::attribute::CollectionPtr& colPtr,
  std::vector<ItemExpressionDefInfo>& expressionDefInfo, std::vector<AttRefDefInfo>& attRefDefInfo)
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
    defPtr->setNumberOfRequiredValues(j.at("NumberOfRequiredValues"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    defPtr->setIsExtensible(j.at("Extensible"));
    defPtr->setMaxNumberOfValues(j.at("MaxNumberOfValues"));
  }
  catch (std::exception& /*e*/)
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
        defPtr->setCommonValueLabel(clabels.at("CommonLabel"));
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
        defPtr->setValueLabel(i, (*iterator).get<std::string>());
      }
    }
    catch (std::exception& /*e*/)
    {
    }
  }
  try
  {
    std::string etype = j.at("ExpressionType");
    DefinitionPtr adef = colPtr->findDefinition(etype);
    if (adef)
    {
      defPtr->setExpressionDefinition(adef);
    }
    else
    {
      // We need to queue up this item to be assigned its definition later
      expressionDefInfo.push_back(ItemExpressionDefInfo(defPtr, etype));
    }
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    defPtr->setUnits(j.at("Units"));
  }
  catch (std::exception& /*e*/)
  {
  }
  // Now let's process its children items
  try
  {
    json childrenDefs = j.at("ChildrenDefinitions");
    for (json::iterator iter = childrenDefs.begin(); iter != childrenDefs.end(); iter++)
    {
      smtk::attribute::JsonHelperFunction::processItemDefinitionTypeFromJson(
        iter, defPtr, colPtr, expressionDefInfo, attRefDefInfo);
    }
  }
  catch (std::exception& /*e*/)
  {
  }
}
}
}
