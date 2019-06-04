//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonReferenceItemDefinition.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/attribute/json/jsonItemDefinition.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/Entity.h"

#include "nlohmann/json.hpp"

#include <string>

/**\brief Provide a way to serialize ReferenceItemDefinitionPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(
  nlohmann::json& j, const smtk::attribute::ReferenceItemDefinitionPtr& defPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ItemDefinition>(defPtr));
  nlohmann::json accept;
  for (auto& acceptable : defPtr->acceptableEntries())
  {
    accept.push_back(acceptable.first);
    accept.push_back(acceptable.second);
  }
  j["Accepts"] = accept;
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
    j["ReferenceLabels"] = valueLabel;
  }
  if (defPtr->holdReference())
  {
    j["HoldReference"] = true;
  }
}

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j, smtk::attribute::ReferenceItemDefinitionPtr& defPtr)
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
    nlohmann::json accept = j.at("Accepts");
    for (auto iterator = accept.begin(); iterator != accept.end(); ++iterator)
    {
      auto acc1 = (*iterator).get<std::string>();
      ++iterator;
      auto acc2 = (*iterator).get<std::string>();
      defPtr->setAcceptsEntries(acc1, acc2, true);
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
    clabels = j.at("ReferenceLabels");
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
  try
  {
    defPtr->setHoldReference(j.at("HoldReference"));
  }
  catch (std::exception& /*e*/)
  {
  }
}
}
}
