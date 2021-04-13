//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonResourceItemDefinition.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/attribute/json/jsonReferenceItemDefinition.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/Entity.h"

#include "nlohmann/json.hpp"

#include <string>

/**\brief Provide a way to serialize ResourceItemDefinitionPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(
  nlohmann::json& j,
  const smtk::attribute::ResourceItemDefinitionPtr& defPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ReferenceItemDefinition>(defPtr));
}

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j,
  smtk::attribute::ResourceItemDefinitionPtr& defPtr,
  const smtk::attribute::ResourcePtr& resPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto refDef = smtk::dynamic_pointer_cast<ReferenceItemDefinition>(defPtr);
  smtk::attribute::from_json(j, refDef, resPtr);
}
} // namespace attribute
} // namespace smtk
