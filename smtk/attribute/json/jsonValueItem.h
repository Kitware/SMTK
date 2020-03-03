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
#include "smtk/attribute/Resource.h"
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
  std::vector<ItemExpressionInfo>& itemExpressionInfo, std::vector<AttRefInfo>& attRefInfo);

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
        j["Val"] = itemPtr->value();
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
  std::vector<ItemExpressionInfo>& itemExpressionInfo, std::vector<AttRefInfo>& /*attRefInfo*/)
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
      {
        auto query = iter->find("UnsetVal");
        if (query != iter->end())
        {
          continue;
        }
      }

      {
        auto query = iter->find("Val");
        if (query != iter->end())
        {
          auto queryIth = query->find("Ith");
          auto queryValue = query->find("Name");
          if (queryIth != query->end() && queryValue != query->end())
          {
            i = *queryIth;
            BasicType currentValue = *queryValue;
            itemPtr->setValue(static_cast<int>(i), currentValue);
          }
        }
      }

      if (allowsExpressions)
      {
        {
          auto query = iter->find("Expression");
          if (query != iter->end())
          {
            auto queryName = query->find("Name");
            if (queryName != query->end())
            {
              expName = *queryName;
              expAtt = resPtr->findAttribute(expName);
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
      json expression;
      {
        auto query = j.find("Expression");
        if (query != j.end())
        {
          expression = *query;
        }
      }

      if (allowsExpressions && !expression.is_null())
      {
        auto expNameQuery = j.find("ExpressionName");
        if (expNameQuery != j.end())
        {
          expName = *expNameQuery;
          expAtt = resPtr->findAttribute(expName);
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

      {
        auto query = j.find("Val");
        if (query != j.end())
        {
          BasicType currentValue = *query;
          itemPtr->setValue(currentValue);
        }
      }
    }
  }
}
}
}

#endif
