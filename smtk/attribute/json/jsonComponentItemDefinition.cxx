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
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/json/jsonReferenceItemDefinition.h"

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
  const smtk::attribute::ComponentItemDefinitionPtr& defPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ReferenceItemDefinition>(defPtr));
}

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j,
  smtk::attribute::ComponentItemDefinitionPtr& defPtr,
  const smtk::attribute::ResourcePtr& resPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto refItemDef = smtk::dynamic_pointer_cast<ReferenceItemDefinition>(defPtr);
  smtk::attribute::from_json(j, refItemDef, resPtr);
}
SMTKCORE_EXPORT void processFromRefItemDef(
  const nlohmann::json& j,
  smtk::attribute::ComponentItemDefinitionPtr& defPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto temp = smtk::dynamic_pointer_cast<ItemDefinition>(defPtr);
  smtk::attribute::from_json(j, temp);
  // Has the attribute definition been set?
  // Handle at at constuction time since definition knows about resource
  // Reference: XmlDocV1Parser: L1308
  auto result = j.find("AttDef");
  if (result != j.end())
  {
    std::string qstring = smtk::attribute::Resource::createAttributeQuery(*result);
    defPtr->setAcceptsEntries(smtk::common::typeName<smtk::attribute::Resource>(), qstring, true);
  }

  result = j.find("NumberOfRequiredValues");
  if (result != j.end())
  {
    defPtr->setNumberOfRequiredValues(*result);
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

} // namespace attribute
} // namespace smtk
