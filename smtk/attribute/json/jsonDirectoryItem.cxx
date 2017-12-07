//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonDirectoryItem.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/json/jsonFileSystemItem.h"

#include "json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize DirectoryItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::DirectoryItemPtr& itemPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<FileSystemItem>(itemPtr));
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::DirectoryItemPtr& itemPtr)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr.get())
  {
    return;
  }
  auto temp = smtk::dynamic_pointer_cast<FileSystemItem>(itemPtr);
  smtk::attribute::from_json(j, temp);
}
}
}
