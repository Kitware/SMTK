//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonDoubleItemDefinition.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/DoubleItemDefinition.h"

#include "nlohmann/json.hpp"
#include "smtk/attribute/json/jsonValueItemDefinition.h"

#include <string>
using json = nlohmann::json;

/**\brief Provide a way to serialize itemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(
  nlohmann::json& j,
  const smtk::attribute::DoubleItemDefinitionPtr& defPtr)
{
  // No need to call ItemDefinition's to_json function since ValueItemDefinition's
  // to_json function would take care of it
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ValueItemDefinition>(defPtr));
  smtk::attribute::processDerivedValueDefToJson(j, defPtr);
  if ((!defPtr->isDiscrete()) && defPtr->hasDefault())
  {
    // Add the Defaults as strings since they may contain units. Note that we still
    // store the default values as doubles as well for backward compatibility
    j["DefaultValueAsString"] = defPtr->defaultValuesAsStrings();
  }
}

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j,
  smtk::attribute::DoubleItemDefinitionPtr& defPtr,
  const smtk::attribute::ResourcePtr& resPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto valDef = smtk::dynamic_pointer_cast<ValueItemDefinition>(defPtr);
  smtk::attribute::from_json(j, valDef, resPtr);
  smtk::attribute::processDerivedValueDefFromJson<smtk::attribute::DoubleItemDefinitionPtr, double>(
    j, defPtr, resPtr);
  if (!defPtr->isDiscrete())
  {
    auto defaultVal = j.find("DefaultValueAsString");
    if (defaultVal != j.end())
    {
      std::vector<std::string> values = *defaultVal;
      if (!defPtr->setDefaultValueAsString(values))
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "When converting json, doubleItemDefinition "
            << defPtr->name() << " could not set its defaults from strings");
      }
    }
  }
}
} // namespace attribute
} // namespace smtk
