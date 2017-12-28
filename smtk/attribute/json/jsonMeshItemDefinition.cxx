//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonMeshItemDefinition.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/MeshItemDefinition.h"
#include "smtk/attribute/json/jsonItemDefinition.h"

#include "json.hpp"

#include <string>

/**\brief Provide a way to serialize itemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(
  nlohmann::json& j, const smtk::attribute::MeshItemDefinitionPtr& defPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ItemDefinition>(defPtr));
  j["NumberOfRequiredValues"] = defPtr->numberOfRequiredValues();
  if (defPtr->isExtensible())
  {
    j["Extensible"] = true;
    if (defPtr->maxNumberOfValues())
    {
      j["MaxNumberOfValues"] = defPtr->maxNumberOfValues();
    }
  }
}

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j, smtk::attribute::MeshItemDefinitionPtr& defPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto temp = smtk::dynamic_pointer_cast<ItemDefinition>(defPtr);
  smtk::attribute::from_json(j, temp);
  try
  {
    defPtr->setNumberOfRequiredValues(j.at("NumberOfRequiredValues"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    defPtr->setIsExtensible(j.at("Extensible"));
    defPtr->setMaxNumberOfValues(j.at("MaxNumberOfValues"));
  }
  catch (std::exception& /*e*/)
  {
  }
}
}
}
