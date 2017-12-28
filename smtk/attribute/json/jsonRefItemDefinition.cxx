//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonRefItemDefinition.h"

#include "smtk/attribute/Collection.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/RefItemDefinition.h"

#include "smtk/attribute/json/jsonItemDefinition.h"

#include "json.hpp"
#include "smtk/PublicPointerDefs.h"

#include <string>
using json = nlohmann::json;

/**\brief Provide a way to serialize RefItemDefinition
  */
namespace smtk
{
namespace attribute
{
using AttRefDefInfo = std::pair<smtk::attribute::RefItemDefinitionPtr, std::string>;

SMTKCORE_EXPORT void to_json(nlohmann::json& j, const smtk::attribute::RefItemDefinitionPtr& defPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ItemDefinition>(defPtr));
  attribute::DefinitionPtr adp = defPtr->attributeDefinition();
  if (adp)
  {
    j["AttDef"] = adp->type();
  }
  j["NumberOfRequiredValues"] = defPtr->numberOfRequiredValues();
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

SMTKCORE_EXPORT void from_json(const nlohmann::json& j,
  smtk::attribute::RefItemDefinitionPtr& defPtr, const smtk::attribute::CollectionPtr& colPtr,
  std::vector<AttRefDefInfo>& expressionDefInfo)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto temp = smtk::dynamic_pointer_cast<ItemDefinition>(defPtr);
  smtk::attribute::from_json(j, temp);
  // Has the attribute definition been set?
  // Handle at at constuction time since definition knows about collection
  // Reference: XmlDocV1Parser: L1308
  try
  {
    std::string etype = j.at("AttDef");
    DefinitionPtr adef = colPtr->findDefinition(etype);
    if (adef)
    {
      defPtr->setAttributeDefinition(adef);
    }
    else
    {
      // We need to queue up this item to be assigned its definition later
      expressionDefInfo.push_back(AttRefDefInfo(defPtr, etype));
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

  json clabels;
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
      json labels = clabels.at("Label");
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
}
}
