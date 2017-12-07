//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonMeshSelectionItemDefinition.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"
#include "smtk/attribute/json/jsonItemDefinition.h"
#include "smtk/model/Entity.h"

#include "json.hpp"

#include <string>

/**\brief Provide a way to serialize itemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(
  nlohmann::json& j, const smtk::attribute::MeshSelectionItemDefinitionPtr& defPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ItemDefinition>(defPtr));
  j["ModelEntityRef"] = defPtr->refModelEntityName();
  std::string maskStr = smtk::model::Entity::flagToSpecifierString(defPtr->membershipMask());
  j["MembershipMask"] = maskStr;
}

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j, smtk::attribute::MeshSelectionItemDefinitionPtr& defPtr)
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
    defPtr->setRefModelEntityName(j.at("ModelEntityRef"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    std::string mmask = j.at("MembershipMask");
    if (!mmask.empty())
    {
      smtk::model::BitFlags flags = smtk::model::Entity::specifierStringToFlag(mmask);
      defPtr->setMembershipMask(flags);
    }
  }
  catch (std::exception& /*e*/)
  {
  }
}
}
}
