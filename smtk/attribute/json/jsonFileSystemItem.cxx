//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonFileSystemItem.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/FileSystemItem.h"
#include "smtk/attribute/json/jsonItem.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize FileSystemItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::FileSystemItemPtr& itemPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<Item>(itemPtr));
  std::size_t numRequiredVals = itemPtr->numberOfRequiredValues();
  size_t i, n = itemPtr->numberOfValues();
  if (!n)
  {
    return;
  }

  if (numRequiredVals == 1 && !itemPtr->isExtensible()) // Special Common Case
  {
    if (itemPtr->isSet())
    {
      j["Value"] = itemPtr->value().c_str();
    }
    return;
  }
  json values;
  for (i = 0; i < n; i++)
  {
    if (itemPtr->isSet(i))
    {
      values.push_back(itemPtr->value(i));
    }
    else
    {
      values.push_back(nullptr);
    }
  }
  j["Values"] = values;
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::FileSystemItemPtr& itemPtr)
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
    // The node should have an attribute indicating how many values are
    // associated with the item
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
      if (i >= n)
      {
        continue;
      }
      if (!iter->is_null())
      {
        itemPtr->setValue(i, *iter);
      }
    }
  }
  else if (numRequiredVals == 1)
  {
    try
    {
      itemPtr->setValue(j.at("Value"));
    }
    catch (std::exception& /*e*/)
    {
    }
  }
}
}
}
