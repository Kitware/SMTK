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
#include "smtk/attribute/json/jsonHelperFunction.h"
#include "smtk/attribute/json/jsonItemDefinition.h"

#include "smtk/io/Logger.h"

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
  j["EnforceCategories"] = defPtr->enforcesCategories();
  j["Accepts"] = accept;
  nlohmann::json reject;
  for (auto& rejected : defPtr->rejectedEntries())
  {
    reject.push_back(rejected.first);
    reject.push_back(rejected.second);
  }
  j["Rejects"] = reject;
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
  j["Role"] = defPtr->role();

  // Now let's process its children Items
  if (!defPtr->numberOfChildrenItemDefinitions())
  {
    return;
  }

  json childDefs;
  for (const auto& childInfo : defPtr->childrenItemDefinitions())
  {
    json childDef;
    smtk::attribute::JsonHelperFunction::processItemDefinitionTypeToJson(
      childDef, childInfo.second);
    // Same type definitions can occur multiple times
    childDefs.push_back(childDef);
  }
  j["ChildrenDefinitions"] = childDefs;
  j["ResourceQueries"] = defPtr->resourceQueries();
  j["ComponentQueries"] = defPtr->componentQueries();
  j["ConditionalInfo"] = defPtr->conditionalInformation();
}

SMTKCORE_EXPORT void from_json(const nlohmann::json& j,
  smtk::attribute::ReferenceItemDefinitionPtr& defPtr, const smtk::attribute::ResourcePtr& resPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto basicItem = smtk::dynamic_pointer_cast<ItemDefinition>(defPtr);
  smtk::attribute::from_json(j, basicItem);
  auto enforceCats = j.find("EnforceCategories");
  if (enforceCats != j.end())
  {
    defPtr->setEnforcesCategories(*enforceCats);
  }
  auto accept = j.find("Accepts");
  if (accept == j.end())
  {
    smtkErrorMacro(smtk::io::Logger::instance(),
      "Can not find Accept key for ReferenceItemDefinition:" << defPtr->name());
    return;
  }
  for (auto iterator = accept->begin(); iterator != accept->end(); ++iterator)
  {
    auto acc1 = (*iterator).get<std::string>();
    ++iterator;
    auto acc2 = (*iterator).get<std::string>();
    defPtr->setAcceptsEntries(acc1, acc2, true);
  }

  auto reject = j.find("Rejects");
  if (reject != j.end())
  {
    for (auto iterator = reject->begin(); iterator != reject->end(); ++iterator)
    {
      auto acc1 = (*iterator).get<std::string>();
      ++iterator;
      auto acc2 = (*iterator).get<std::string>();
      defPtr->setRejectsEntries(acc1, acc2, true);
    }
  }

  auto numberOfRequiredValues = j.find("NumberOfRequiredValues");
  if (numberOfRequiredValues == j.end())
  {
    smtkErrorMacro(smtk::io::Logger::instance(),
      "Can not find NumberOfRequiredValues key for ReferenceItemDefinition:" << defPtr->name());
    return;
  }
  defPtr->setNumberOfRequiredValues(*numberOfRequiredValues);

  auto result = j.find("Extensible");
  if (result != j.end())
  {
    defPtr->setIsExtensible(*result);
  }

  result = j.find("MaxNumberOfValues");
  if (result != j.end())
  {
    defPtr->setMaxNumberOfValues(*result);
  }

  result = j.find("HoldReference");
  if (result != j.end())
  {
    defPtr->setHoldReference(*result);
  }

  result = j.find("Role");
  if (result != j.end())
  {
    defPtr->setRole(*result);
  }

  result = j.find("ReferenceLabels");
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
  // Now let's process its children items
  result = j.find("ChildrenDefinitions");
  if (result == j.end())
  {
    return; // no children info
  }

  for (auto& jIdef : *result)
  {
    smtk::attribute::JsonHelperFunction::processItemDefinitionTypeFromJson(jIdef, defPtr, resPtr);
  }
  std::vector<std::string> compQueries, resourceQueries;
  std::vector<std::vector<std::string> > conditionalInfo;

  result = j.find("ResourceQueries");
  if (result == j.end())
  {
    // We are missing information!
    smtkErrorMacro(smtk::io::Logger::instance(),
      "Can not find Resource Queries for ReferenceItemDefinition:" << defPtr->name());
    return;
  }
  resourceQueries = result->get<std::vector<std::string> >();

  result = j.find("ComponentQueries");
  if (result == j.end())
  {
    // We are missing information!
    smtkErrorMacro(smtk::io::Logger::instance(),
      "Can not find Component Queries for ReferenceItemDefinition:" << defPtr->name());
    return;
  }
  compQueries = result->get<std::vector<std::string> >();

  result = j.find("ConditionalInfo");
  if (result == j.end())
  {
    // We are missing information!
    smtkErrorMacro(smtk::io::Logger::instance(),
      "Can not find Conditional Information for ReferenceItemDefinition:" << defPtr->name());
    return;
  }
  conditionalInfo = result->get<std::vector<std::vector<std::string> > >();
  std::size_t i, n = conditionalInfo.size();
  if ((resourceQueries.size() != n) || (compQueries.size() != n))
  {
    // We have inconsistent information!
    smtkErrorMacro(smtk::io::Logger::instance(),
      "Conditional Information for ReferenceItemDefinition:" << defPtr->name()
                                                             << " have different sizes");
    return;
  }
  for (i = 0; i < n; i++)
  {
    defPtr->addConditional(resourceQueries[i], compQueries[i], conditionalInfo[i]);
  }
}
}
}
