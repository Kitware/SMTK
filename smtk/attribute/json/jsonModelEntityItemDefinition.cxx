//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonModelEntityItemDefinition.h"

#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/json/jsonItemDefinition.h"
#include "smtk/io/Logger.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/Entity.h"

#include "nlohmann/json.hpp"

#include <string>
using json = nlohmann::json;

/**\brief Provide a way to serialize ModelEntityItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::ModelEntityItemDefinitionPtr& defPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ItemDefinition>(defPtr));
  j["Name"] = defPtr->name();
  std::string maskStr = smtk::model::Entity::flagToSpecifierString(defPtr->membershipMask());
  j["MembershipMask"] = maskStr;
  j["NumberOfRequriedValues"] = defPtr->numberOfRequiredValues();
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
    json valueLabel;
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

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::ModelEntityItemDefinitionPtr& defPtr)
{
  if (!defPtr)
  {
    return;
  }
  auto itemDef = smtk::dynamic_pointer_cast<ItemDefinition>(defPtr);
  smtk::attribute::from_json(j, itemDef);
  auto result = j.find("MembershipMask");
  if (result == j.end())
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "JSON MembershipMask missing "
        << "for ModelEntityDefinition:" << defPtr->type());
    return;
  }
  else
  {
    smtk::model::BitFlags flags = smtk::model::Entity::specifierStringToFlag(*result);
    defPtr->setMembershipMask(flags);
  }

  result = j.find("NumberOfRequriedValues");
  if (result == j.end())
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "JSON NumberOfRequriedValues missing "
        << "for ModelEntityDefinition:" << defPtr->type());
    return;
  }
  else
  {
    defPtr->setNumberOfRequiredValues(*result);
  }

  result = j.find("Extensible");
  if (result != j.end())
  {
    defPtr->setIsExtensible(*result);
  }
  result = j.find("MaxNumberOfValues");
  if (result != j.end())
  {
    defPtr->setMaxNumberOfValues(*result);
  }

  result = j.find("ComponentLabels");
  if (result != j.end())
  {
    auto common = result->find("CommonLabel");
    if (common != result->end())
    {
      defPtr->setCommonValueLabel(*common);
    }
    else
    {
      auto labels = result->find("Label");
      int i(0);
      if (labels != result->end())
      {
        for (auto iterator = labels->begin(); iterator != labels->end(); iterator++, i++)
        {
          defPtr->setValueLabel(i, (*iterator).get<std::string>());
        }
      }
    }
  }
}
}
}
