//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#pragma once

#include "smtk/common/UUID.h"

#include "nlohmann/json.hpp"

#include <string>

// Override how UUIDs are serialized.
// Without this, they appear as JSON arrays of unsigned integers.
namespace nlohmann
{
template <>
struct adl_serializer<smtk::common::UUID>
{
  static void to_json(json& j, const smtk::common::UUID& opt)
  {
    if (opt.isNull())
    {
      j = nullptr;
    }
    else
    {
      j = opt.toString();
    }
  }

  static void from_json(const json& j, smtk::common::UUID& opt)
  {
    if (j.is_null())
    {
      opt = smtk::common::UUID::null();
    }
    else
    {
      opt = j.get<std::string>();
    }
  }
};
}
