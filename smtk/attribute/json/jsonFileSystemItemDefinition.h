//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonFileSystemItemDefinition_h
#define smtk_attribute_jsonFileSystemItemDefinition_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/FileSystemItemDefinition.h"
#include "smtk/attribute/json/jsonItemDefinition.h"

#include "json.hpp"

#include <string>

using json = nlohmann::json;
/**\brief Provide a way to serialize itemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::FileSystemItemDefinitionPtr& defPtr);

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::FileSystemItemDefinitionPtr& defPtr);
}
}

#endif
