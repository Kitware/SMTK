//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonModelEntityItem.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/json/jsonItem.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize ModelEntityItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::ModelEntityItemPtr& itemPtr)
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
    if (itemPtr->isSet())
    {
      j["Val"] = itemPtr->value(i).entity().toString();
    }
    return;
  }
  json values;
  for (i = 0; i < n; i++)
  {
    if (itemPtr->isSet(i))
    {

      auto uuid = itemPtr->value(i).entity().toString();
      values.push_back(uuid);
    }
    else
    {
      values.push_back(nullptr);
    }
  }
  j["Values"] = values;
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::ModelEntityItemPtr& itemPtr)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr.get())
  {
    return;
  }
  auto temp = smtk::dynamic_pointer_cast<Item>(itemPtr);
  smtk::attribute::from_json(j, temp);

  std::size_t n = itemPtr->numberOfValues();
  smtk::common::UUID uid;
  smtk::model::ManagerPtr mmgr = itemPtr->attribute()->attributeResource()->refModelManager();
  std::size_t numRequiredVals = itemPtr->numberOfRequiredValues();
  json values;
  try
  {
    values = j.at("Values");
  }
  catch (std::exception& /*e*/)
  {
  }
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
  if (!values.is_null())
  {
    int i(0);
    for (auto iter = values.begin(); iter != values.end(); iter++, i++)
    {
      if (iter->is_null())
      { // Ignore empty value
        continue;
      }
      std::string uuid = *iter;
      uid = smtk::common::UUID(uuid);
      itemPtr->setValue(i, smtk::model::EntityRef(mmgr, uid));
    }
  }
  else if (numRequiredVals == 1)
  {
    try
    {
      std::string uuid = j.at("Val");
      uid = smtk::common::UUID(uuid);
      itemPtr->setValue(smtk::model::EntityRef(mmgr, uid));
    }
    catch (std::exception& /*e*/)
    {
    }
  }
}
}
}
