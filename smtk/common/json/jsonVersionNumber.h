//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_json_jsonVersionNumber_h
#define smtk_common_json_jsonVersionNumber_h

#include "smtk/common/VersionNumber.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;
// Override how VersionNumber numbers are serialized.
// Without this, they appear as JSON arrays of unsigned integers.
namespace smtk
{
namespace common
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::common::VersionNumber& opt);

SMTKCORE_EXPORT void from_json(const json& j, smtk::common::VersionNumber& opt);
} // namespace common
} // namespace smtk

#endif
