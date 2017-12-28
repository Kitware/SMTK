//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonComponentItemDefinition.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/json/jsonItemDefinition.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/Entity.h"

#include "json.hpp"

#include <string>

/**\brief Provide a way to serialize ComponentItemDefinitionPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(
  nlohmann::json& j, const smtk::attribute::ComponentItemDefinitionPtr& defPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ItemDefinition>(defPtr));
  std::string maskStr = smtk::model::Entity::flagToSpecifierString(defPtr->membershipMask());
  j["MembershipMask"] = maskStr;
  j["NumberOfRequiredValues"] = defPtr->numberOfRequiredValues();
  if (defPtr->isExtensible())
  {
    j["Extensible"] = true;
    if (defPtr->maxNumberOfValues())
    {
      j["MaxNumberOfValues"] = defPtr->maxNumberOfValues();
    }
  }
  if (defPtr->hasValueLabels())
  {
    nlohmann::json valueLabel;
    if (defPtr->usingCommonLabel())
    {
      valueLabel["CommonLabel"] = defPtr->valueLabel(0);
    }
    else
    {
      for (size_t index = 0; index < defPtr->numberOfRequiredValues(); index++)
      {
        valueLabel["Label"].push_back(defPtr->valueLabel(index));
      }
    }
    j["ComponentLabels"] = valueLabel;
  }
}

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j, smtk::attribute::ComponentItemDefinitionPtr& defPtr)
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

  nlohmann::json clabels;
  try
  {
    clabels = j.at("ComponentLabels");
    if (!clabels.is_null())
    {
      // Nested try/catch
      try
      {
        defPtr->setCommonValueLabel(clabels.at("CommonLabel"));
      }
      catch (std::exception& /*e*/)
      {
      }
    }
  }
  catch (std::exception& /*e*/)
  {
  }
  if (!clabels.is_null())
  {
    try
    {
      nlohmann::json labels = clabels.at("Label");
      size_t i(0);
      for (auto iterator = labels.begin(); iterator != labels.end(); iterator++, i++)
      {
        defPtr->setValueLabel(i, (*iterator).get<std::string>());
      }
    }
    catch (std::exception& /*e*/)
    {
    }
  }
}
}
}
