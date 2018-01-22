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
#include "smtk/attribute/Collection.h"
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

SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::ValueItemPtr& itemPtr);

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::ValueItemPtr& itemPtr,
  const smtk::attribute::CollectionPtr& colPtr, std::vector<ItemExpressionInfo>& itemExpressionInfo,
  std::vector<AttRefInfo>& attRefInfo);

/**\ A helper function to process Derived type of valueItem and
   * covert it to json
   */
template <typename ItemType>
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
  if ((itemPtr->numberOfRequiredValues() == 1) && !itemPtr->isExtensible())
  {
    if (itemPtr->isSet())
    {
      if (itemPtr->isExpression())
      {
        j["Expression"] = true;
        j["ExpressionName"] = itemPtr->expression()->name();
      }
      else
      {
        j["ExpressionName"] = itemPtr->value();
      }
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
      if (itemPtr->isExpression(i))
      {
        value["Expression"]["Ith"] = i;
        value["Expression"]["Name"] = itemPtr->expression(i)->name();
      }
      else
      {
        value["Val"]["Ith"] = i;
        value["Val"]["Name"] = itemPtr->value(i);
      }
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
template <typename ItemType, typename BasicType>
static void processDerivedValueFromJson(const json& j, ItemType itemPtr,
  const smtk::attribute::CollectionPtr& colPtr, std::vector<ItemExpressionInfo>& itemExpressionInfo,
  std::vector<AttRefInfo>& /*attRefInfo*/)
{
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
  if (itemPtr->isExtensible())
  {
    // The node should have an attribute indicating how many values are
    // associated with the item
    try
    {
      n = j.at("NumberOfValues");
      itemPtr->setNumberOfValues(n);
    }
    catch (std::exception& /*e*/)
    {
    }
  }
  if (!n)
  {
    return;
  }
  json values;
  try
  {
    values = j.at("Values");
  }
  catch (std::exception& /*e*/)
  {
  }
  if (!values.is_null())
  {
    for (auto iter = values.begin(); iter != values.end(); iter++)
    {
      try
      {
        json dumy = iter->at("UnsetVal");
        continue;
      }
      catch (std::exception& /*e*/)
      {
      }
      try
      {
        i = iter->at("Val").at("Ith");
        BasicType currentValue = iter->at("Val").at("Name");
        itemPtr->setValue(static_cast<int>(i), currentValue);
      }
      catch (std::exception& /*e*/)
      {
      }
      if (allowsExpressions)
      {
        try
        {
          expName = iter->at("Expression").at("Name");
          expAtt = colPtr->findAttribute(expName);
          if (!expAtt)
          {
            info.item = itemPtr;
            info.pos = static_cast<int>(i);
            info.expName = expName;
            itemExpressionInfo.push_back(info);
          }
          else
          {
            itemPtr->setExpression(static_cast<int>(i), expAtt);
          }
        }
        catch (std::exception& /*e*/)
        {
        }
      }
    }
  }
  else if ((numRequiredVals == 1) && !itemPtr->isExtensible())
  {
    json noVal;
    try
    {
      noVal = j.at("UnsetVal");
    }
    catch (std::exception& /*e*/)
    {
    }
    if (!noVal.is_null())
    {
      json expression;
      try
      {
        expression = j.at("Expression");
      }
      catch (std::exception& /*e*/)
      {
      }
      if (allowsExpressions && !expression.is_null())
      {
        try
        {
          expName = j.at("ExpressionName");
          expAtt = colPtr->findAttribute(expName);
        }
        catch (std::exception& /*e*/)
        {
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
      }
      else
      {
        try
        {
          BasicType currentValue = j.at("ExpressionName");
          itemPtr->setValue(currentValue);
        }
        catch (std::exception& /*e*/)
        {
        }
      }
    }
  }
}
}
}

#endif
