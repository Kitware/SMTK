//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonRefItem.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/json/jsonItem.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize refItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::RefItemPtr& itemPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<Item>(itemPtr));
  size_t i(0), nRV = itemPtr->numberOfRequiredValues(), nV = itemPtr->numberOfValues();
  if (!itemPtr->numberOfValues())
  {
    return;
  }
  if (nRV == 1)
  {
    if (itemPtr->isSet())
    {
      j["Val"] = itemPtr->value(i)->name();
    }
    return;
  }

  json values;
  for (i = 0; i < nV; i++)
  {
    if (itemPtr->isSet(i))
    {
      values.push_back(itemPtr->value(i)->name());
    }
    else
    {
      values.push_back(nullptr);
    }
  }
  j["Values"] = values;
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::RefItemPtr& itemPtr,
  const CollectionPtr& colPtr, std::vector<AttRefInfo>& attRefInfos)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr.get())
  {
    return;
  }
  auto temp = smtk::dynamic_pointer_cast<Item>(itemPtr);
  smtk::attribute::from_json(j, temp);

  std::size_t n = itemPtr->numberOfValues();
  std::size_t numRequiredVals = itemPtr->numberOfRequiredValues();
  std::string attName;
  attribute::AttributePtr att;
  AttRefInfo info;

  json values;
  try
  {
    values = j.at("Values");
  }
  catch (std::exception& /*e*/)
  {
  }
  if (!numRequiredVals)
  {
    if (values.is_array() && (values.size() > 0))
    {
      n = values.size();
      itemPtr->setNumberOfValues(values.size());
    }
  }

  if (!n)
  {
    return;
  }
  if (!values.is_null())
  {
    size_t i(0);
    for (auto iter = values.begin(); iter != values.end(); iter++, i++)
    {
      try
      {
        if (i >= n)
        {
          continue;
        }
        if (!iter->is_null())
        {
          attName = *iter;
          att = colPtr->findAttribute(attName);
          if (!att)
          {
            info.item = itemPtr;
            info.pos = static_cast<int>(i);
            info.attName = attName;
            attRefInfos.push_back(info);
          }
          else
          {
            itemPtr->setValue(i, att);
          }
        }
      }
      catch (std::exception& /*e*/)
      {
      }
    }
  }
  else if (numRequiredVals == 1)
  {
    try
    {
      attName = j.at("Val");
      att = colPtr->findAttribute(attName);
      if (!att)
      {
        info.item = itemPtr;
        info.pos = 0;
        info.attName = attName;
        attRefInfos.push_back(info);
      }
      else
      {
        itemPtr->setValue(att);
      }
    }
    catch (std::exception& /*e*/)
    {
    }
  }
}
}
}
