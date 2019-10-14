//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_json_jsonProperties_h
#define smtk_common_json_jsonProperties_h

#include "smtk/CoreExports.h"

#include "smtk/common/Properties.h"

#include "nlohmann/json.hpp"

// Define how properties are serialized.
namespace smtk
{
namespace common
{
SMTKCORE_EXPORT void to_json(nlohmann::json& j, const PropertiesContainer& properties);
SMTKCORE_EXPORT void from_json(const nlohmann::json& j, PropertiesContainer& properties);
}
}

#endif
