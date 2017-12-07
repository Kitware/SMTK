//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_io_jsonUUID_h
#define smtk_io_jsonUUID_h

#include "smtk/common/UUID.h"

#include "json.hpp"

#include <string>

using json = nlohmann::json;
// Override how UUIDs are serialized.
// Without this, they appear as JSON arrays of unsigned integers.
namespace smtk
{
namespace common
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::common::UUID& opt);

SMTKCORE_EXPORT void from_json(const json& j, smtk::common::UUID& opt);
}
}

#endif
