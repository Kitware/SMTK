//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonModelEntityItemDefinition_h
#define smtk_attribute_jsonModelEntityItemDefinition_h

#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/json/jsonItemDefinition.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/Entity.h"

#include "json.hpp"

#include <string>
using json = nlohmann::json;

/**\brief Provide a way to serialize ModelEntityItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::ModelEntityItemDefinitionPtr& defPtr);

SMTKCORE_EXPORT void from_json(
  const json& j, smtk::attribute::ModelEntityItemDefinitionPtr& defPtr);
}
}

#endif
