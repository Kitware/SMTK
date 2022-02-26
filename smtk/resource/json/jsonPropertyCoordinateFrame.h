//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_json_jsonPropertyCoordinateFrame_h
#define smtk_resource_json_jsonPropertyCoordinateFrame_h

#include "smtk/CoreExports.h"

#include "smtk/resource/properties/CoordinateFrame.h"

#include "nlohmann/json.hpp"

// Define how CoordinateFrame is serialized.
namespace smtk
{
namespace resource
{
namespace properties
{
using json = nlohmann::json;

SMTKCORE_EXPORT void to_json(json&, const smtk::resource::properties::CoordinateFrame&);

SMTKCORE_EXPORT void from_json(const json&, smtk::resource::properties::CoordinateFrame&);

} // namespace properties
} // namespace resource
} // namespace smtk

#endif // smtk_resource_json_jsonPropertyCoordinateFrame_h
