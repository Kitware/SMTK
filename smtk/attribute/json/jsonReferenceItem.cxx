//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonReferenceItem.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/json/jsonItem.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "smtk/common/json/jsonUUID.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize ReferenceItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::ReferenceItemPtr& itemPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<Item>(itemPtr));
  size_t i = 0, nn = itemPtr->numberOfValues();

  if (!nn)
  {
    return;
  }

  std::vector<json> values(nn);
  for (i = 0; i < nn; i++)
  {
    auto key = itemPtr->objectKey(i);
    values[i] = key;
  }
  j["Values"] = values;
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::ReferenceItemPtr& itemPtr)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr)
  {
    return;
  }
  auto temp = std::static_pointer_cast<Item>(itemPtr);
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
  if (values.is_array() && (values.size() > 0))
  {
    n = values.size();
    if (!itemPtr->setNumberOfValues(values.size()))
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Unable to set the number of values on "
          << itemPtr->name() << " to " << n);
    }
  }
  if (!n)
  {
    return;
  }
  if (values.is_array())
  {
    for (auto iter = values.begin(); iter != values.end(); iter++, i++)
    {
      try
      {
        json val = *iter;
        if (val.is_null())
        {
          continue;
        }
        smtk::common::UUID ruid = val[0];
        smtk::common::UUID cuid = val[1];
        ReferenceItem::Key key = std::make_pair(ruid, cuid);
        itemPtr->setObjectKey(i, key);
      }
      catch (std::exception& /*e*/)
      {
      }
    }
  }
}
}
}
