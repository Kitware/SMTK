//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonResourceItem.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/json/jsonReferenceItem.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize ResourceItemPtr
  * Since ResourceItem has the same data as ReferenceItem, it just calls ReferenceItem json function.
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::ResourceItemPtr& itemPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ReferenceItem>(itemPtr));
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::ResourceItemPtr& itemPtr,
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
}
}
