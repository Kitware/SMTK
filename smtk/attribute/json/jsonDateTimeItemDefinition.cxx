//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonDateTimeItemDefinition.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/DateTimeItemDefinition.h"
#include "smtk/attribute/json/jsonItemDefinition.h"

#include "nlohmann/json.hpp"

#include <string>

/**\brief Provide a way to serialize DateTimeItemDefinitionPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(
  nlohmann::json& j,
  const smtk::attribute::DateTimeItemDefinitionPtr& defPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ItemDefinition>(defPtr));
  j["NumberOfRequiredValues"] = defPtr->numberOfRequiredValues();
  std::string format = defPtr->displayFormat();
  if (!format.empty())
  {
    j["DisplayFormat"] = format;
  }
  j["ShowTimeZone"] = defPtr->useTimeZone();
  j["ShowCalendarPopup"] = defPtr->useCalendarPopup();
  if (defPtr->hasDefault())
  {
    ::smtk::common::DateTimeZonePair dtz = defPtr->defaultValue();
    j["DefaultValue"] = dtz.serialize();
  }
  // QUESTION XmlV3StringWriter does not serialize range info
}

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j,
  smtk::attribute::DateTimeItemDefinitionPtr& defPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto itemDef = smtk::dynamic_pointer_cast<ItemDefinition>(defPtr);
  smtk::attribute::from_json(j, itemDef);

  auto result = j.find("NumberOfRequiredValues");
  if (result != j.end())
  {
    defPtr->setNumberOfRequiredValues(*result);
  }

  result = j.find("DisplayFormat");
  if (result != j.end())
  {
    defPtr->setDisplayFormat(*result);
  }

  result = j.find("ShowTimeZone");
  if (result != j.end())
  {
    defPtr->setUseTimeZone(*result);
  }

  result = j.find("ShowCalendarPopup");
  if (result != j.end())
  {
    defPtr->setEnableCalendarPopup(*result);
  }

  result = j.find("DefaultValue");
  if (result != j.end())
  {
    ::smtk::common::DateTimeZonePair dtz;
    dtz.deserialize(*result);
    defPtr->setDefaultValue(dtz);
  }
}
} // namespace attribute
} // namespace smtk
