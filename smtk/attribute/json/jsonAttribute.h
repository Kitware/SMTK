//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonAttribute_h
#define smtk_attribute_jsonAttribute_h

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ReferenceItem.h"

#include "smtk/attribute/json/jsonHelperFunction.h"
#include "smtk/attribute/json/jsonItem.h"
#include "smtk/attribute/json/jsonReferenceItem.h"
#include "smtk/io/Logger.h"

#include "smtk/PublicPointerDefs.h"

#include "nlohmann/json.hpp"

#include <iosfwd>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize valueItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::AttributePtr& att);

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::AttributePtr& att,
  std::vector<smtk::attribute::ItemExpressionInfo>& itemExpressionInfo,
  std::vector<smtk::attribute::AttRefInfo>& attRefInfo,
  const std::set<const smtk::attribute::ItemDefinition*>& convertedAttDefs);
}
}

#endif
