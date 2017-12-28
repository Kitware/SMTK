//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonMeshSelectionItem_h
#define smtk_attribute_jsonMeshSelectionItem_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/json/jsonItem.h"
#include "smtk/common/UUID.h"

#include "json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize MeshSelectionItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::MeshSelectionItemPtr& itemPtr);

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::MeshSelectionItemPtr& itemPtr);
}
}

#endif
