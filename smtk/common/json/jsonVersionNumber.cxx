//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonVersionNumber.h"

#include <string>

using json = nlohmann::json;
// Override how VersionNumbers are serialized.
// Without this, they appear as JSON arrays of unsigned integers.
namespace smtk
{
namespace common
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::common::VersionNumber& v)
{
  j = v.string();
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::common::VersionNumber& v)
{
  v = VersionNumber(j.get<std::string>());
}

} // namespace common
} // namespace smtk
