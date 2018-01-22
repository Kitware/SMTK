//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonMeshSelectionItemDefinition_h
#define smtk_attribute_jsonMeshSelectionItemDefinition_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"
#include "smtk/attribute/json/jsonItemDefinition.h"
#include "smtk/model/Entity.h"

#include "nlohmann/json.hpp"

#include <string>

/**\brief Provide a way to serialize itemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(
  nlohmann::json& j, const smtk::attribute::MeshSelectionItemDefinitionPtr& defPtr);

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j, smtk::attribute::MeshSelectionItemDefinitionPtr& defPtr);
}
}

#endif
