//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smkt_model_json_jsonArrangement_h
#define smkt_model_json_jsonArrangement_h

#include "smtk/CoreExports.h"
#include "smtk/model/Arrangement.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace model
{
using json = nlohmann::json;
SMTKCORE_EXPORT void to_json(json& j, const smtk::model::Arrangement& arr);

SMTKCORE_EXPORT void from_json(const json& j, smtk::model::Arrangement& arr);
} // namespace model
} // namespace smtk

#endif
