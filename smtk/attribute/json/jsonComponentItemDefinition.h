//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonComponentItemDefinition_h
#define smtk_attribute_jsonComponentItemDefinition_h

#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/json/jsonItemDefinition.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/Entity.h"

#include "nlohmann/json.hpp"

#include <string>

/**\brief Provide a way to serialize ComponentItemDefinitionPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(
  nlohmann::json& j,
  const smtk::attribute::ComponentItemDefinitionPtr& defPtr);

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j,
  smtk::attribute::ComponentItemDefinitionPtr& defPtr,
  const smtk::attribute::ResourcePtr& resPtr);

SMTKCORE_EXPORT void processFromRefItemDef(
  const nlohmann::json& j,
  smtk::attribute::ComponentItemDefinitionPtr& defPtr);
} // namespace attribute
} // namespace smtk

#endif
