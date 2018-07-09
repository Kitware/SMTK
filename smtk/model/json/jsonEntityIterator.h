//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_json_jsonEntityIterator_h
#define smtk_model_json_jsonEntityIterator_h

#include "smtk/CoreExports.h"

#include "smtk/model/json/jsonArrangement.h"
#include "smtk/model/json/jsonTessellation.h"

#include "smtk/common/json/jsonUUID.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/EntityIterator.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace model
{
using json = nlohmann::json;
// Define how model-entity iterators are serialized.
using EntityIterator = smtk::model::EntityIterator;

SMTKCORE_EXPORT void to_json(json& j, const EntityIterator& iter);
}
}

#endif
