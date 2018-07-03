//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonRefItemDefinition_h
#define smtk_attribute_jsonRefItemDefinition_h

#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/Resource.h"

#include "smtk/attribute/json/jsonItemDefinition.h"

#include "nlohmann/json.hpp"
#include "smtk/PublicPointerDefs.h"

#include <string>
using json = nlohmann::json;

/**\brief Provide a way to serialize RefItemDefinition
  */
namespace smtk
{
namespace attribute
{
using AttRefDefInfo = std::pair<smtk::attribute::RefItemDefinitionPtr, std::string>;

SMTKCORE_EXPORT void to_json(
  nlohmann::json& j, const smtk::attribute::RefItemDefinitionPtr& defPtr);

SMTKCORE_EXPORT void from_json(const nlohmann::json& j,
  smtk::attribute::RefItemDefinitionPtr& defPtr, const smtk::attribute::ResourcePtr& colPtr,
  std::vector<AttRefDefInfo>& expressionDefInfo);
}
}

#endif
