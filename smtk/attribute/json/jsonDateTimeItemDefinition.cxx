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

#include "json.hpp"

#include <string>

/**\brief Provide a way to serialize DateTimeItemDefinitionPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(
  nlohmann::json& j, const smtk::attribute::DateTimeItemDefinitionPtr& defPtr)
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
  const nlohmann::json& j, smtk::attribute::DateTimeItemDefinitionPtr& defPtr)
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
    defPtr->setNumberOfRequiredValues(j.at("NumberOfRequiredValues"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    defPtr->setDisplayFormat(j.at("DisplayFormat"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    defPtr->setUseTimeZone(j.at("ShowTimeZone"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    defPtr->setEnableCalendarPopup(j.at("ShowCalendarPopup"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    ::smtk::common::DateTimeZonePair dtz;
    dtz.deserialize(j.at("DefaultValue"));
    defPtr->setDefaultValue(dtz);
  }
  catch (std::exception& /*e*/)
  {
  }
}
}
}
