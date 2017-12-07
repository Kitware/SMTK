//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonMeshItem_h
#define smtk_attribute_jsonMeshItem_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/json/jsonItem.h"
#include "smtk/common/UUID.h"
#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"

#include "json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize MeshItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::MeshItemPtr& itemPtr);

SMTKCORE_EXPORT void from_json(
  const json& j, smtk::attribute::MeshItemPtr& itemPtr, smtk::attribute::CollectionPtr colPtr);
}
}

#endif
