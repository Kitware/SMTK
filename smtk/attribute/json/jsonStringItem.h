//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonStringItem_h
#define smtk_attribute_jsonStringItem_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/json/jsonValueItem.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize StringItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::StringItemPtr& itemPtr);

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::StringItemPtr& itemPtr,
  const smtk::attribute::CollectionPtr& colPtr, std::vector<ItemExpressionInfo>& itemExpressionInfo,
  std::vector<AttRefInfo>& attRefInfo);
}
}

#endif
