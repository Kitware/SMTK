//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonFileSystemItemDefinition.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/FileSystemItemDefinition.h"
#include "smtk/attribute/json/jsonItemDefinition.h"
#include "smtk/io/Logger.h"

#include "nlohmann/json.hpp"

#include <string>

using json = nlohmann::json;
/**\brief Provide a way to serialize itemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::FileSystemItemDefinitionPtr& defPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<ItemDefinition>(defPtr));
  j["NumberOfRequiredValues"] = defPtr->numberOfRequiredValues();
  if (defPtr->isExtensible())
  {
    j["Extensible"] = true;
    if (defPtr->maxNumberOfValues())
    {
      j["MaxNumberOfValues"] = defPtr->maxNumberOfValues();
    }
  }
  if (defPtr->shouldExist())
  {
    j["ShouldExit"] = true;
  }
  if (defPtr->shouldBeRelative())
  {
    j["ShouldBeRelative"] = true;
  }
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
  if (defPtr->hasDefault())
  {
    j["DefaultValue"] = defPtr->defaultValue();
  }
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::FileSystemItemDefinitionPtr& defPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto itemDef = smtk::dynamic_pointer_cast<ItemDefinition>(defPtr);
  smtk::attribute::from_json(j, itemDef);
  auto result = j.find("NumberOfRequiredValues");
  if (result == j.end())
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "JSON missing NumberOfRequiredValues"
        << "for FileSystemItemDefinition:" << defPtr->name());
    return;
  }
  defPtr->setNumberOfRequiredValues(*result);

  result = j.find("Extensible");
  if (result != j.end())
  {
    defPtr->setIsExtensible(*result);
  }

  result = j.find("MaxNumberOfValues");
  if (result != j.end())
  {
    defPtr->setMaxNumberOfValues(*result);
  }

  result = j.find("ShouldExit");
  if (result != j.end())
  {
    defPtr->setShouldExist(*result);
  }

  result = j.find("ShouldBeRelative");
  if (result != j.end())
  {
    defPtr->setShouldBeRelative(*result);
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

  result = j.find("DefaultValue");
  if (result != j.end())
  {
    defPtr->setDefaultValue(*result);
  }
}
}
}
