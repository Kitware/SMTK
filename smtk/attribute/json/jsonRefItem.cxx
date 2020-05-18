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

#include "smtk/io/Logger.h"

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
  const json& j,
  smtk::attribute::ComponentItemPtr& itemPtr,
  std::vector<AttRefInfo>& attRefInfos)
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
  std::string attName;
  attribute::AttributePtr att;
  AttRefInfo info;

  auto values = j.find("Values");
  if ((values != j.end()) && values->is_array())
  {
    if (itemPtr->isExtensible())
    {
      n = values->size();
      itemPtr->setNumberOfValues(n);
    }

    if (!n)
    {
      return;
    }
    size_t i(0);
    for (auto iter = values->begin(); iter != values->end(); iter++, i++)
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
  }
  else if (numRequiredVals == 1)
  {
    auto attName = j.find("Val");
    if (attName != j.end())
    {
      att = resPtr->findAttribute(*attName);
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
    else
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(), "Missing Val for old RefItem item:" << itemPtr->name());
    }
  }
}
} // namespace attribute
} // namespace smtk
