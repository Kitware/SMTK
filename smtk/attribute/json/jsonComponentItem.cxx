//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonComponentItem.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/json/jsonReferenceItem.h"
#include "smtk/io/Logger.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize ComponentItemPtr
  * Since ComponentItem has the same data as ReferenceItem, it just calls ReferenceItem json function.
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::ComponentItemPtr& itemPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ReferenceItem>(itemPtr));
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::ComponentItemPtr& itemPtr,
  std::vector<ItemExpressionInfo>& itemExpressionInfo, std::vector<AttRefInfo>& attRefInfo)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr.get())
  {
    return;
  }
  auto refItem = smtk::dynamic_pointer_cast<ReferenceItem>(itemPtr);
  smtk::attribute::from_json(j, refItem, itemExpressionInfo, attRefInfo);
}

SMTKCORE_EXPORT void processFromRefItemSpec(
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

  auto values = j.find("Values");
  if (values != j.end())
  {
    if (itemPtr->isExtensible() && values->is_array())
    {
      n = values->size();
      if (!itemPtr->setNumberOfValues(n))
      {
        smtkErrorMacro(smtk::io::Logger::instance(), "Unable to set the number of values on "
            << itemPtr->name() << " to " << n);
      }
    }

    if (!n)
    {
      return;
    }

    size_t i(0);
    for (auto iter = values->begin(); iter != values->end(); iter++, i++)
    {
      if ((i >= n) || iter->is_null())
      {
        continue;
      }

      attName = iter->get<std::string>();
      att = resPtr->findAttribute(attName);
      if (att == nullptr)
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
  else if (numRequiredVals == 1)
  {
    auto val = j.find("Val");
    if (val != j.end())
    {
      attName = val->get<std::string>();
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
  }
}
}
}
