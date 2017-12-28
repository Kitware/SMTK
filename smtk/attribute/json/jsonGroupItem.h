//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonGroupItem_h
#define smtk_attribute_jsonGroupItem_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/json/jsonHelperFunction.h"
#include "smtk/attribute/json/jsonItem.h"

#include "json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize GroupItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::GroupItemPtr& itemPtr);

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::GroupItemPtr& itemPtr,
  const smtk::attribute::CollectionPtr& colPtr, std::vector<ItemExpressionInfo>& itemExpressionInfo,
  std::vector<AttRefInfo>& attRefInfo);
}
}

#endif
