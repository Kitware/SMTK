//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonValueItemDefinition_h
#define smtk_attribute_jsonValueItemDefinition_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/json/jsonHelperFunction.h"
#include "smtk/attribute/json/jsonItemDefinition.h"

#include "nlohmann/json.hpp"

#include <string>
using json = nlohmann::json;

/**\brief Provide a way to serialize itemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(
  nlohmann::json& j, const smtk::attribute::ValueItemDefinitionPtr& defPtr);

SMTKCORE_EXPORT void from_json(const nlohmann::json& j,
  smtk::attribute::ValueItemDefinitionPtr& defPtr, const smtk::attribute::ResourcePtr& resPtr);

/**\ A helper function to process Derived type of valueItemDefinition and
   * covert it to json
   */
template <typename ItemDefType>
static void processDerivedValueDefToJson(json& j, ItemDefType defPtr)
{
  if (defPtr->isDiscrete())
  { // Three pieces: value list, structure list and DefaultIndex
    json discreteInfoJson;
    size_t n = defPtr->numberOfDiscreteValues();
    std::string enumName;
    std::vector<std::string> conditionalItems;
    std::set<std::string> categoryValues;
    // Loop through all discrete values
    for (size_t i = 0; i < n; i++)
    {
      enumName = defPtr->discreteEnum(i);
      // Check conditional items
      conditionalItems = defPtr->conditionalItems(enumName);
      categoryValues = defPtr->enumCategories(enumName);
      json valueJson, structureJson, resultJson;
      if (conditionalItems.size() || categoryValues.size())
      {
        // Structure enums
        // TODO: Simplifiy the logic here
        valueJson["Enum"][enumName] = defPtr->discreteValue(i);
        structureJson["Value"] = valueJson;
        // Structure Items
        if (conditionalItems.size())
        {
          structureJson["Items"] = conditionalItems;
        }
        if (categoryValues.size())
        {
          structureJson["Categories"] = categoryValues;
        }
        if (defPtr->hasEnumAdvanceLevel(enumName))
        {
          structureJson["AdvanceLevel"] = defPtr->enumAdvanceLevel(enumName);
        }
        resultJson["Structure"] = structureJson;
      }
      else
      {
        valueJson["Enum"][enumName] = defPtr->discreteValue(i);
        if (defPtr->hasEnumAdvanceLevel(enumName))
        {
          valueJson["AdvanceLevel"] = defPtr->enumAdvanceLevel(enumName);
        }
        resultJson["Value"] = valueJson;
      }
      discreteInfoJson["ValueAndStructure"].push_back(resultJson);
    }
    if (defPtr->hasDefault())
    {
      discreteInfoJson["DefaultIndex"] = defPtr->defaultDiscreteIndex();
    }
    j["DiscreteInfo"] = discreteInfoJson;
    return;
  }

  // Does this def have a default value
  if (defPtr->hasDefault())
  {
    // Ignore the notion of seperator in XmlV2StringWriter::L195
    //
    j["DefaultValue"] = defPtr->defaultValues();
  }
  // Does this node have a range?
  if (defPtr->hasRange())
  {
    json rangeInfoJson;
    if (defPtr->hasMinRange())
    {
      rangeInfoJson["Min"]["Inclusive"] = defPtr->minRangeInclusive();
      rangeInfoJson["Min"]["Value"] = defPtr->minRange();
    }
    if (defPtr->hasMaxRange())
    {
      rangeInfoJson["Max"]["Inclusive"] = defPtr->maxRangeInclusive();
      rangeInfoJson["Max"]["Value"] = defPtr->maxRange();
    }
    j["RangeInfo"] = rangeInfoJson;
  }
}

/**\ A helper function to process json and
   * covert it to derived type of valueItemDefinition
   */
template <typename ItemDefType, typename BasicType>
static void processDerivedValueDefFromJson(
  const json& j, ItemDefType defPtr, const smtk::attribute::ResourcePtr& /*resPtr*/)

{
  auto discreteInfo = j.find("DiscreteInfo");
  if (discreteInfo != j.end())
  {
    auto valueAndStructure = discreteInfo->find("ValueAndStructure");

    if (valueAndStructure == discreteInfo->end())
    {
      std::cerr << "Item Definition: " << defPtr->name()
                << " is missing Discrete Value/Structure Info!\n";
      return;
    }

    int i = 0;
    for (auto iter = valueAndStructure->begin(); iter != valueAndStructure->end(); iter++, i++)
    {
      // Are we dealing with a structure based enum?
      auto structure = iter->find("Structure");
      if (structure != iter->end())
      {
        auto advanceLevel = structure->find("AdvanceLevel");
        auto value = structure->find("Value");
        if (value == structure->end())
        {
          std::cerr << "Item Definition: " << defPtr->name() << " is missing Value!\n";
          continue;
        }
        auto enumValue = value->find("Enum");
        if (enumValue == value->end())
        {
          std::cerr << "Item Definition: " << defPtr->name() << " is missing Enum!\n";
          continue;
        }
        auto itemsValue = structure->find("Items");     // list of conditional Items
        auto catsValue = structure->find("Categories"); // list of categories for enum
        // Should just iterate once
        for (auto currentEnum = enumValue->begin(); currentEnum != enumValue->end(); currentEnum++)
        {
          // Data and discreteEnum
          BasicType currentEnumValue = currentEnum.value();
          defPtr->addDiscreteValue(currentEnumValue, currentEnum.key());
          std::string discreteEnum = defPtr->discreteEnum(i);
          if (itemsValue != structure->end())
          {
            for (auto currentItem : *itemsValue)
            {
              defPtr->addConditionalItem(discreteEnum, currentItem);
            }
          }
          if (catsValue != structure->end())
          {
            defPtr->setEnumCategories(discreteEnum, *catsValue);
          }
          if (advanceLevel != structure->end())
          {
            defPtr->setEnumAdvanceLevel(discreteEnum, *advanceLevel);
          }
        }
      }
      else // We are dealing with a simple enum (no children items or categories)
      {
        auto value = iter->find("Value");
        if (value == iter->end())
        {
          std::cerr << "Item Definition: " << defPtr->name() << " is missing Value!\n";
          continue;
        }
        auto enumValue = value->find("Enum");
        if (enumValue == value->end())
        {
          std::cerr << "Item Definition: " << defPtr->name() << " is missing Enum!\n";
          continue;
        }
        auto advanceLevel = value->find("AdvanceLevel");

        for (auto currentEnum = enumValue->begin(); currentEnum != enumValue->end(); currentEnum++)
        {
          // Data and discreteEnum
          BasicType currentEnumValue = currentEnum.value();
          defPtr->addDiscreteValue(currentEnumValue, currentEnum.key());
          if (advanceLevel != value->end())
          {
            defPtr->setEnumAdvanceLevel(currentEnum.key(), *advanceLevel);
          }
        }
      }
    }
    auto defaultIndex = discreteInfo->find("DefaultIndex");
    if (defaultIndex != discreteInfo->end())
    {
      defPtr->setDefaultDiscreteIndex(*defaultIndex);
    }
  }
  else // Ok this is a non-discrete definition
  {
    auto defaultVal = j.find("DefaultValue");
    if (defaultVal != j.end())
    {
      if (defaultVal->is_array())
      {
        std::vector<BasicType> values = *defaultVal;
        defPtr->setDefaultValue(values);
      }
      else
      { // Single value condition
        BasicType value = *defaultVal;
        defPtr->setDefaultValue(value);
      }
    }
    auto rangeInfo = j.find("RangeInfo");
    if (rangeInfo != j.end())
    {
      auto minInfo = rangeInfo->find("Min");
      if (minInfo != rangeInfo->end())
      {
        auto val = minInfo->find("Value");
        auto inclusive = minInfo->find("Inclusive");
        if ((val != minInfo->end()) && (inclusive != minInfo->end()))
        {
          defPtr->setMinRange(*val, *inclusive);
        }
      }
      auto maxInfo = rangeInfo->find("Max");
      if (maxInfo != rangeInfo->end())
      {
        auto val = maxInfo->find("Value");
        auto inclusive = maxInfo->find("Inclusive");
        if ((val != maxInfo->end()) && (inclusive != maxInfo->end()))
        {
          defPtr->setMaxRange(*val, *inclusive);
        }
      }
    }
  }
}
}
}

#endif
