//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonValueItem_h
#define smtk_attribute_jsonValueItem_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/json/jsonComponentItem.h"
#include "smtk/attribute/json/jsonHelperFunction.h"
#include "smtk/attribute/json/jsonItem.h"

#include "smtk/io/Logger.h"

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

SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::ValueItemPtr& itemPtr);

SMTKCORE_EXPORT void from_json(
  const json& j,
  smtk::attribute::ValueItemPtr& itemPtr,
  std::vector<ItemExpressionInfo>& itemExpressionInfo,
  std::vector<AttRefInfo>& attRefInfo);

/**\ A helper function to process Derived type of valueItem and
   * covert it to json
   */
template<typename ItemType>
static void processDerivedValueToJson(json& j, ItemType itemPtr)
{
  if (itemPtr->isDiscrete())
  {
    return;
  }
  size_t i, n = itemPtr->numberOfValues();
  if (!n)
  {
    return;
  }
  if (itemPtr->isExpression())
  {
    j["ExpressionReference"] = itemPtr->expressionReference();
    return;
  }
  if ((itemPtr->numberOfRequiredValues() == 1) && !itemPtr->isExtensible())
  {
    if (itemPtr->isSet())
    {
      j["Val"] = itemPtr->value();
      j["SpecifiedVal"] = itemPtr->valueAsString();
    }
    else
    {
      j["UnsetVal"] = true;
    }
    return;
  }
  json values;
  for (i = 0; i < n; i++)
  {
    json value;
    if (itemPtr->isSet(i))
    {
      value["Val"]["Ith"] = i;
      value["Val"]["Name"] = itemPtr->value(i);
      value["Val"]["Specified"] = itemPtr->valueAsString(i);
    }
    else
    {
      value["UnsetVal"]["Ith"] = i;
    }
    values.push_back(value);
  }
  j["Values"] = values;
}

/**\ A helper function to process json and
   * covert it to derived type of valueItem
   */
template<typename ItemType, typename BasicType>
static void processDerivedValueFromJson(
  const json& j,
  ItemType itemPtr,
  std::vector<ItemExpressionInfo>& itemExpressionInfo,
  std::vector<AttRefInfo>& attRefInfo)
{
  auto resPtr = itemPtr->attribute()->attributeResource();
  if (itemPtr->isDiscrete())
  {
    return;
  }
  std::size_t i, n = itemPtr->numberOfValues();
  std::size_t numRequiredVals = itemPtr->numberOfRequiredValues();
  std::string expName;
  attribute::AttributePtr expAtt;
  bool allowsExpressions = itemPtr->allowsExpressions();
  ItemExpressionInfo info;

  // Is the item using an expression?
  if (allowsExpressions)
  {
    json expression;
    {
      // This is the latest version for expressions
      auto query = j.find("ExpressionReference");
      if (query != j.end())
      {
        std::set<const smtk::attribute::ItemDefinition*> convertedAttDefs;
        ItemPtr expressionItem = itemPtr->expressionReference();
        smtk::attribute::JsonHelperFunction::processItemTypeFromJson(
          *query, expressionItem, itemExpressionInfo, attRefInfo, convertedAttDefs);
        return;
      }
      // This is the older implementation
      query = j.find("Expression");
      if (query != j.end())
      {
        expression = *query;
      }
    }

    if (!expression.is_null())
    {
      auto expNameQuery = j.find("ExpressionName");
      if (expNameQuery != j.end())
      {
        expName = expNameQuery->get<std::string>();
        expAtt = resPtr->findAttribute(expName);
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "Missing ExpressionName for Item: " << itemPtr->name()
                                              << " in Attribute: " << itemPtr->attribute()->name());
      }

      if (!expAtt)
      {
        info.item = itemPtr;
        info.pos = 0;
        info.expName = expName;
        itemExpressionInfo.push_back(info);
      }
      else
      {
        itemPtr->setExpression(expAtt);
      }
      return;
    }
  }

  if (itemPtr->isExtensible())
  {
    // The node should have an attribute indicating how many values are
    // associated with the item

    {
      auto query = j.find("NumberOfValues");
      if (query != j.end())
      {
        n = *query;
        itemPtr->setNumberOfValues(n);
      }
    }
  }
  if (!n)
  {
    return;
  }

  json values;
  {
    auto query = j.find("Values");
    if (query != j.end())
    {
      values = *query;
    }
  }

  if (!values.is_null())
  {
    for (auto iter = values.begin(); iter != values.end(); iter++)
    {
      // Skip over unset values
      auto query = iter->find("UnsetVal");
      if (query != iter->end())
      {
        continue;
      }

      // Do we have a value element?
      query = iter->find("Val");
      if (query != iter->end())
      {
        auto queryIth = query->find("Ith");
        if (queryIth == query->end())
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Could not find index of value location for Item: " << itemPtr->name());
        }
        else
        {
          i = *queryIth;
          // Lets see if we can find a specified or name key to get the value
          auto queryValue = query->find("Specified");
          if (queryValue != query->end())
          {
            std::string currentValue = *queryValue;
            itemPtr->setValueFromString(static_cast<int>(i), currentValue);
          }
          else
          {
            queryValue = query->find("Name");
            if (queryValue != query->end())
            {
              i = *queryIth;
              BasicType currentValue = *queryValue;
              itemPtr->setValue(static_cast<int>(i), currentValue);
            }
            else
            {
              smtkErrorMacro(
                smtk::io::Logger::instance(),
                "Could not find value[" << i << "] location for Item: " << itemPtr->name());
            }
          }
        }
      }

      if (allowsExpressions)
      {
        {
          auto query = iter->find("Expression");
          if (query != iter->end())
          {
            smtkErrorMacro(
              smtk::io::Logger::instance(),
              "Encountered old style expression per element format for Item: "
                << itemPtr->name() << " from Attribute: " << itemPtr->attribute()->name()
                << " ignoring expression setting!");
          }
        }
      }
    }
  }
  else if ((numRequiredVals == 1) && !itemPtr->isExtensible())
  {
    json noVal;
    {
      auto query = j.find("UnsetVal");
      if (query != j.end())
      {
        noVal = *query;
      }
    }

    if (noVal.is_null())
    {
      auto query = j.find("SpecifiedVal");
      if (query != j.end())
      {
        std::string currentValue = *query;
        itemPtr->setValueFromString(0, currentValue);
      }
      else
      {
        auto query = j.find("Val");
        if (query != j.end())
        {
          BasicType currentValue = *query;
          itemPtr->setValue(0, currentValue);
        }
      }
    }
  }
}
} // namespace attribute
} // namespace smtk

#endif
