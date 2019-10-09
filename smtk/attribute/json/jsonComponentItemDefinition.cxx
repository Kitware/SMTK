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
#include "smtk/mesh/core/Resource.h"
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
  nlohmann::json& j, const smtk::attribute::ComponentItemDefinitionPtr& defPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ReferenceItemDefinition>(defPtr));
}

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j, smtk::attribute::ComponentItemDefinitionPtr& defPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto temp = smtk::dynamic_pointer_cast<ReferenceItemDefinition>(defPtr);
  smtk::attribute::from_json(j, temp);
}
SMTKCORE_EXPORT void processFromRefItemDef(
  const nlohmann::json& j, smtk::attribute::ComponentItemDefinitionPtr& defPtr)
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
  try
  {
    std::string etype = j.at("AttDef");
    std::string qstring = smtk::attribute::Resource::createAttributeQuery(etype);
    defPtr->setAcceptsEntries(smtk::common::typeName<smtk::attribute::Resource>(), qstring, true);
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
      int i(0);
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
SMTKCORE_EXPORT void processFromMeshItemDef(
  const nlohmann::json& j, smtk::attribute::ComponentItemDefinitionPtr& defPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  // Create the appropriate query for mesh sets
  defPtr->setAcceptsEntries(smtk::common::typeName<smtk::mesh::Resource>(), "meshset", true);

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
