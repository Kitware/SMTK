//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_json_jsonEntity_h
#define smtk_model_json_jsonEntity_h

#include "smtk/CoreExports.h"

#include "smtk/common/json/jsonUUID.h"

#include "smtk/model/Entity.h"
#include "smtk/model/Resource.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

namespace smtk
{
namespace model
{
// Define how Entity records are serialized.
using json = nlohmann::json;
SMTKCORE_EXPORT void to_json(json& j, const smtk::model::EntityPtr& ent);

SMTKCORE_EXPORT void from_json(const json& j, smtk::model::EntityPtr& ent);

using UUIDsToEntities = smtk::model::UUIDsToEntities;

SMTKCORE_EXPORT void to_json(json& j, const UUIDsToEntities& emap);

SMTKCORE_EXPORT void from_json(const json& j, UUIDsToEntities& emap);
} // namespace model
} // namespace smtk
#endif
