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
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Resource.h"
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
SMTKCORE_EXPORT void processFromRefItem(
  const json& j, smtk::attribute::ComponentItemPtr& itemPtr, std::vector<AttRefInfo>& attRefInfos)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr.get())
  {
    return;
  }
  auto resPtr = itemPtr->attribute()->attributeResource();
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
          att = resPtr->findAttribute(attName);
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
      att = resPtr->findAttribute(attName);
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
