//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonFileItemDefinition_h
#define smtk_attribute_jsonFileItemDefinition_h

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
  nlohmann::json& j, const smtk::attribute::FileItemDefinitionPtr& defPtr);

SMTKCORE_EXPORT void from_json(
  const nlohmann::json& j, smtk::attribute::FileItemDefinitionPtr& defPtr);
}
}

#endif
