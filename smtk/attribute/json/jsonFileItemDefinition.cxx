//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonFileItemDefinition.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/json/jsonFileSystemItemDefinition.h"

#include "nlohmann/json.hpp"

#include <string>

/**\brief Provide a way to serialize FileItemDefinition
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(
  nlohmann::json& j, const smtk::attribute::FileItemDefinitionPtr& defPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<FileSystemItemDefinition>(defPtr));
  std::string fileFilters = defPtr->getFileFilters();
  if (!fileFilters.empty())
  {
    j["FileFilters"] = fileFilters;
  }
}

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j, smtk::attribute::FileItemDefinitionPtr& defPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto temp = smtk::dynamic_pointer_cast<FileSystemItemDefinition>(defPtr);
  smtk::attribute::from_json(j, temp);
}
}
}
