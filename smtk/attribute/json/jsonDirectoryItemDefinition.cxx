//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonDirectoryItemDefinition.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/json/jsonFileSystemItemDefinition.h"

#include "nlohmann/json.hpp"

#include <string>

/**\brief Provide a way to serialize DirectoryItemDefinition
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(
  nlohmann::json& j, const smtk::attribute::DirectoryItemDefinitionPtr& defPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<FileSystemItemDefinition>(defPtr));
}

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j, smtk::attribute::DirectoryItemDefinitionPtr& defPtr)
{
  // The caller should make sure that defPtr is valid since it's not default constructible
  if (!defPtr.get())
  {
    return;
  }
  auto fsysDef = smtk::dynamic_pointer_cast<FileSystemItemDefinition>(defPtr);
  smtk::attribute::from_json(j, fsysDef);
  //QUESTION: xml parser does not serialize default value and extensibility
}
}
}
