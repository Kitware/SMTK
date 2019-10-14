//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonDateTimeItem.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/json/jsonItem.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize itemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::DateTimeItemPtr& itemPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<Item>(itemPtr));

  size_t numValues = itemPtr->numberOfValues();
  if (numValues == 0)
  {
    return;
  }

  // (else)
  if (numValues == 1)
  {
    if (itemPtr->isSet())
    {
      ::smtk::common::DateTimeZonePair dtz = itemPtr->value();
      j["Value"] = dtz.serialize();
    }
    else
    {
      j["Value"] = "UnsetVal";
    }
    return;
  }

  // (else)
  j["NumberOfValues"] = numValues;
  json values;
  for (std::size_t i = 0; i < numValues; ++i)
  {
    if (itemPtr->isSet(i))
    {
      ::smtk::common::DateTimeZonePair dtz = itemPtr->value();
      values.push_back(dtz.serialize());
    }
    else
    {
      values.push_back(nullptr);
    }
  }
  j["Values"] = values;
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::DateTimeItemPtr& itemPtr)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr.get())
  {
    return;
  }
  auto temp = smtk::dynamic_pointer_cast<Item>(itemPtr);
  smtk::attribute::from_json(j, temp);

  auto numberOfValues = j.find("NumberOfValues");
  if (numberOfValues == j.end())
  {
    // Single Value
    itemPtr->setNumberOfValues(1);
    auto value = j.find("Value");
    if (value != j.end())
    {
      if (*value != "UnsetVal")
      {
        ::smtk::common::DateTimeZonePair dtz;
        dtz.deserialize(*value);
        itemPtr->setValue(dtz);
      }
    }
  }
  else
  {
    // Multiple values
    std::size_t n = *numberOfValues;
    if (n > 1)
    {
      itemPtr->setNumberOfValues(n);
      auto values = j.find("Values");
      if (values != j.end())
      {
        int i(0);
        for (auto iter = values->begin(); iter != values->end(); iter++, i++)
        {
          if (iter->is_null())
          {
            continue;
          }
          ::smtk::common::DateTimeZonePair dtz;
          dtz.deserialize(*iter);
          itemPtr->setValue(i, dtz);
        }
      }
    }
  }
}
}
}
