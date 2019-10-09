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
    // Loop through all discrete values
    for (size_t i = 0; i < n; i++)
    {
      enumName = defPtr->discreteEnum(i);
      // Check conditional items
      conditionalItems = defPtr->conditionalItems(enumName);
      json valueJson, structureJson, resultJson;
      if (conditionalItems.size())
      {
        // Structure enums
        // TODO: Simplifiy the logic here
        valueJson["Enum"][enumName] = defPtr->discreteValue(i);
        structureJson["Value"] = valueJson;
        // Structure Items
        structureJson["Items"] = conditionalItems;
        resultJson["Structure"] = structureJson;
      }
      else
      {
        valueJson["Enum"][enumName] = defPtr->discreteValue(i);
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
  json discreteInfo, valueAndStructure;
  try
  {
    discreteInfo = j.at("DiscreteInfo");
    valueAndStructure = discreteInfo.at("ValueAndStructure");
  }
  catch (std::exception& /*e*/)
  {
  }
  int i = 0;
  for (json::iterator iter = valueAndStructure.begin(); iter != valueAndStructure.end();
       iter++, i++)
  {
    json vAS = *iter;
    try
    {
      json structure = vAS.at("Structure");
      json enumValue = structure.at("Value").at("Enum"); // object
      json itemsValue = structure.at("Items");           // list
      // Should just iterate once
      for (auto currentEnum = enumValue.begin(); currentEnum != enumValue.end(); currentEnum++)
      {
        // Data and discreteEnum
        BasicType currentEnumValue = currentEnum.value();
        defPtr->addDiscreteValue(currentEnumValue, currentEnum.key());
        std::string discreteEnum = defPtr->discreteEnum(i);
        for (auto currentItem : itemsValue)
        {
          defPtr->addConditionalItem(discreteEnum, currentItem);
        }
      }
    }
    catch (std::exception& /*e*/)
    {
    }
    try
    {
      json enumValue = vAS.at("Value").at("Enum"); // object
      for (auto currentEnum = enumValue.begin(); currentEnum != enumValue.end(); currentEnum++)
      {
        // Data and discreteEnum
        BasicType currentEnumValue = currentEnum.value();
        defPtr->addDiscreteValue(currentEnumValue, currentEnum.key());
      }
    }
    catch (std::exception& /*e*/)
    {
    }
  }
  try
  {
    defPtr->setDefaultDiscreteIndex(discreteInfo.at("DefaultIndex"));
  }
  catch (std::exception& /*e*/)
  {
  }

  if (!discreteInfo.is_null())
  { // No need to check DefaultValue and RangeInfo
    return;
  }
  try
  {
    if (j.at("DefaultValue").is_array())
    {
      std::vector<BasicType> values = j.at("DefaultValue");
      defPtr->setDefaultValue(values);
    }
    else
    { // Single value condition
      BasicType value = j.at("DefaultValue");
      defPtr->setDefaultValue(value);
    }
  }
  catch (std::exception& /*e*/)
  {
  }

  json rangeInfo;
  try
  {
    rangeInfo = j.at("RangeInfo");
  }
  catch (std::exception& /*e*/)
  {
  }
  if (!rangeInfo.is_null())
  {
    // Check min
    try
    {
      defPtr->setMinRange(rangeInfo.at("Min").at("Value"), rangeInfo.at("Min").at("Inclusive"));
    }
    catch (std::exception& /*e*/)
    {
    }
    // Check max
    try
    {
      defPtr->setMaxRange(rangeInfo.at("Max").at("Value"), rangeInfo.at("Max").at("Inclusive"));
    }
    catch (std::exception& /*e*/)
    {
    }
  }
}
}
}

#endif
