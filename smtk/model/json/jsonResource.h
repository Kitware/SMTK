//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_json_jsonResource_h
#define smtk_model_json_jsonResource_h

#include "smtk/CoreExports.h"

#include "smtk/common/json/jsonUUID.h"
#include "smtk/model/json/jsonArrangement.h"
#include "smtk/model/json/jsonTessellation.h"

#include "smtk/model/Resource.h"

#include "nlohmann/json.hpp"

// Define how model collections are serialized.
namespace smtk
{
namespace model
{
using json = nlohmann::json;

SMTKCORE_EXPORT void to_json(json& j, const ResourcePtr& mresource);

SMTKCORE_EXPORT void from_json(const json& j, ResourcePtr& mresource);
}
}

#endif
