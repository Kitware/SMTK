//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonDefinition_h
#define smtk_attribute_jsonDefinition_h

#include "smtk/PublicPointerDefs.h"

#include "json.hpp"

#include "smtk/attribute/Collection.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/RefItem.h"

#include "smtk/attribute/json/jsonAttribute.h"
#include "smtk/attribute/json/jsonHelperFunction.h"
#include "smtk/attribute/json/jsonItem.h"
#include "smtk/attribute/json/jsonModelEntityItemDefinition.h"

#include <exception>
#include <string>

namespace smtk
{
namespace attribute
{
using ItemExpressionDefInfo = std::pair<smtk::attribute::ValueItemDefinitionPtr, std::string>;

using AttRefDefInfo = std::pair<smtk::attribute::RefItemDefinitionPtr, std::string>;

SMTKCORE_EXPORT void to_json(nlohmann::json& j, const smtk::attribute::DefinitionPtr& defPtr);

SMTKCORE_EXPORT void from_json(const nlohmann::json& j, smtk::attribute::DefinitionPtr& defPtr,
  std::vector<ItemExpressionDefInfo>& expressionDefInfo, std::vector<AttRefDefInfo>& attRefDefInfo);
}
}

#endif
