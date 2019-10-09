//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonValueItem.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/json/jsonHelperFunction.h"
#include "smtk/attribute/json/jsonItem.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize valueItemPtr
  */
namespace smtk
{
namespace attribute
{

SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::ValueItemPtr& itemPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<Item>(itemPtr));
  std::size_t numRequiredVals = itemPtr->numberOfRequiredValues();
  size_t i, n = itemPtr->numberOfValues();
  // If the item can have variable number of values then store how many
  // values it has
  if (itemPtr->isExtensible())
  {
    j["NumberOfValues"] = n;
  }
  if (!n)
  {
    return;
  }
  if (!itemPtr->isDiscrete())
  {
    return;
  }
  if (itemPtr->numberOfChildrenItems())
  {
    size_t index(0);
    json childrenItemsJson;
    std::map<std::string, smtk::attribute::ItemPtr>::const_iterator iter;
    const std::map<std::string, smtk::attribute::ItemPtr>& childrenItems = itemPtr->childrenItems();
    for (iter = childrenItems.begin(); iter != childrenItems.end(); iter++, index++)
    {
      json itemValue, childItemJson;
      smtk::attribute::JsonHelperFunction::processItemTypeToJson(itemValue, iter->second);
      // Same type items can occur multiple times
      childItemJson["Type"] = Item::type2String(iter->second->type());
      childItemJson["ItemValue"] = itemValue;
      std::string childItemName = "Index " + std::to_string(index);
      childrenItemsJson[childItemName] = childItemJson;
    }
    j["ChildrenItems"] = childrenItemsJson;
  }
  if ((numRequiredVals == 1) && !itemPtr->isExtensible()) // Sepecial common case
  {
    j["Discrete"] = true;
    if (itemPtr->isSet())
    {
      j["DiscreteIndex"] = itemPtr->discreteIndex();
    }
    return;
  }
  json discreteValues;
  for (i = 0; i < n; i++)
  {
    if (itemPtr->isSet())
    {
      discreteValues.push_back(itemPtr->discreteIndex(i));
    }
    else
    {
      discreteValues.push_back(nullptr);
    }
  }
  j["DiscreteValues"] = discreteValues;
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::ValueItemPtr& itemPtr,
  std::vector<ItemExpressionInfo>& itemExpressionInfo, std::vector<AttRefInfo>& attRefInfo)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr.get())
  {
    return;
  }
  std::set<const smtk::attribute::ItemDefinition*> convertedAttDefs;
  auto temp = smtk::dynamic_pointer_cast<Item>(itemPtr);
  smtk::attribute::from_json(j, temp);
  std::size_t numRequiredVals = itemPtr->numberOfRequiredValues();
  std::size_t i = 0, n = itemPtr->numberOfValues();
  if (itemPtr->isExtensible())
  {
    try
    {
      n = j.at("NumberOfValues");
      itemPtr->setNumberOfValues(n);
    }
    catch (std::exception& /*e*/)
    {
    }
  }

  if (!itemPtr->isDiscrete())
  {
    return;
  }

  // OK Time to process the children items of this Discrete Item
  json childrenItemsJson;
  try
  {
    childrenItemsJson = j.at("ChildrenItems");
  }
  catch (std::exception& /*e*/)
  {
  }
  if (!childrenItemsJson.is_null())
  {
    std::map<std::string, smtk::attribute::ItemPtr>::const_iterator itemIter;
    // Process each child item in ChildrenItems
    size_t index(0);
    const std::map<std::string, smtk::attribute::ItemPtr>& childrenItems = itemPtr->childrenItems();
    for (itemIter = childrenItems.begin(); itemIter != childrenItems.end(); itemIter++, index++)
    {
      // TODO: For now we assume that json and itemPtr are one to one map and
      // they have the same index in the each lists. Add a index check or name search
      // for different indexes condition
      std::string childItemName = "Index " + std::to_string(index);
      try
      {
        json itemJson = childrenItemsJson.at(childItemName);
        json itemValue = itemJson.at("ItemValue");
        smtk::attribute::ItemPtr subItemPtr = itemIter->second;
        smtk::attribute::JsonHelperFunction::processItemTypeFromJson(
          itemValue, subItemPtr, itemExpressionInfo, attRefInfo, convertedAttDefs);
      }
      catch (std::exception& /*e*/)
      {
      }
    }
  }
  if (!n)
  {
    return;
  }
  // There are 2 possible formats - a general one that must be used when n > 1
  // and a special compact one that could be used for n == 1
  // Lets check the general one first - note we only need to process the values
  // that have been set
  json discreteValuesJson;
  try
  {
    discreteValuesJson = j.at("DiscreteValues");
  }
  catch (std::exception& /*e*/)
  {
  }
  if (!discreteValuesJson.is_null())
  {
    for (auto iter = discreteValuesJson.begin(); iter != discreteValuesJson.end(); iter++, i++)
    {
      if (i >= n || iter->is_null())
      {
        continue;
      }
      itemPtr->setDiscreteIndex(i, *iter);
    }
    return;
  }
  if (numRequiredVals == 1) // Special Common Case
  {
    try
    {
      itemPtr->setDiscreteIndex(j.at("DiscreteIndex"));
    }
    catch (std::exception& /*e*/)
    {
    }
  }
}
}
}
