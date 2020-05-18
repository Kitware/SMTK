//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_json_jsonTessellation_h
#define smtk_model_json_jsonTessellation_h

#include "smtk/CoreExports.h"

#include "smtk/model/Tessellation.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace model
{
using json = nlohmann::json;
// Override how Tessellations are serialized.
using Tessellation = smtk::model::Tessellation;

SMTKCORE_EXPORT void to_json(json& j, const Tessellation& tess);

SMTKCORE_EXPORT void from_json(const json& j, Tessellation& tess);
} // namespace model
} // namespace smtk

#endif
