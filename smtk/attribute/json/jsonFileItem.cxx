//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonFileItem.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/json/jsonFileSystemItem.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize FileItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::FileItemPtr& itemPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<FileSystemItem>(itemPtr));
  if (itemPtr->recentValues().size() > 0)
  {
    j["RecentValues"] = itemPtr->recentValues();
  }
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::FileItemPtr& itemPtr)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr.get())
  {
    return;
  }
  auto fsysItem = smtk::dynamic_pointer_cast<FileSystemItem>(itemPtr);
  smtk::attribute::from_json(j, fsysItem);

  auto recentVs = j.find("RecentValues");
  if (recentVs != j.end())
  {
    for (const auto& recentV : *recentVs)
    {
      itemPtr->addRecentValue(recentV);
    }
  }
}
}
}
