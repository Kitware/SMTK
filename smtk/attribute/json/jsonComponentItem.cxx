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
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/json/jsonItem.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize ComponentItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::ComponentItemPtr& itemPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<Item>(itemPtr));
  size_t i = 0, n = itemPtr->numberOfValues();
  std::size_t numRequiredVals = itemPtr->numberOfRequiredValues();

  if (!n)
  {
    return;
  }

  if ((numRequiredVals == 1) && (!itemPtr->isExtensible()))
  {
    auto rsrcPtr = itemPtr->value(i)->resource();
    if (itemPtr->isSet() && rsrcPtr)
    {
      json val;
      // Resource and component pair
      val.push_back(rsrcPtr->id().toString());
      val.push_back(itemPtr->value(i)->id().toString());
      j["Val"].push_back(val);
    }
    else
    {
      j["Val"].push_back(nullptr);
    }
    return;
  }

  json values;
  for (i = 0; i < n; i++)
  {
    auto rsrcPtr = itemPtr->value(i)->resource();
    json val;
    if (itemPtr->isSet(i) && rsrcPtr)
    {
      val.push_back(rsrcPtr->id().toString());
      val.push_back(itemPtr->value(i)->id().toString());
    }
    else
    {
      val = nullptr;
    }
    values.push_back(val);
  }
  j["Values"] = values;
}

SMTKCORE_EXPORT void from_json(
  const json& j, smtk::attribute::ComponentItemPtr& itemPtr, smtk::attribute::CollectionPtr colPtr)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr.get())
  {
    return;
  }
  auto temp = smtk::dynamic_pointer_cast<Item>(itemPtr);
  smtk::attribute::from_json(j, temp);

  json values;
  try
  {
    values = j.at("Values");
  }
  catch (std::exception& /*e*/)
  {
  }

  std::size_t i(0), n = itemPtr->numberOfValues();
  std::size_t numRequiredVals = itemPtr->numberOfRequiredValues();
  if (!numRequiredVals || itemPtr->isExtensible())
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
  auto rsrcMgr = colPtr->manager();
  if (!rsrcMgr && itemPtr->numberOfValues() > 0)
  {
    // TODO: handle check logic in XmlDocV3Parser::249
    return;
  }
  if (!values.is_null())
  {
    for (auto iter = values.begin(); iter != values.end(); iter++, i++)
    {
      try
      {
        if (i >= n)
        {
          continue;
        }
        json val = *iter;
        if (val.is_null())
        {
          continue;
        }
        std::string rsrcFromJson = val[i][0];
        std::string compFromJson = val[i][1];
        auto ruid = smtk::common::UUID(rsrcFromJson);
        auto rsrc = rsrcMgr->get(ruid);
        auto cuid = smtk::common::UUID(compFromJson);
        auto componentPtr = rsrc->find(cuid);
        itemPtr->setValue(static_cast<int>(i), componentPtr);
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
      json val = j.at("Val");
      if (!val[0].is_null())
      {
        std::string rsrcFromJson = val[0][0];
        std::string compFromJson = val[0][1];
        auto ruid = smtk::common::UUID(rsrcFromJson);
        auto rsrc = rsrcMgr->get(ruid);
        auto cuid = smtk::common::UUID(compFromJson);
        auto componentPtr = rsrc->find(cuid);
        itemPtr->setValue(0, componentPtr);
      }
    }
    catch (std::exception& /*e*/)
    {
    }
  }
}
}
}
