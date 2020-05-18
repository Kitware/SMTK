//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/json/jsonTessellation.h"

#include "smtk/model/Tessellation.h"

#include "nlohmann/json.hpp"

#include <iostream>
#include <string>

#include <exception>

namespace smtk
{
namespace model
{
using json = nlohmann::json;
// Override how Tessellations are serialized.
using Tessellation = smtk::model::Tessellation;
using UUID = smtk::common::UUID;

void to_json(json& j, const Tessellation& tess)
{
  if (tess.coords().empty() && tess.conn().empty())
  {
    j = nullptr;
  }
  else
  {
    j = { { "metadata", { "format version", "3" } },
          { "vertices", tess.coords() },
          { "faces", tess.conn() } };
  }
}

void from_json(const json& j, Tessellation& tess)
{
  tess.reset();
  if (!j.is_null())
  {
    try
    {
      tess.coords() = j.at("vertices").get<std::vector<double>>();
      tess.conn() = j.at("faces").get<std::vector<int>>();
    }
    catch (std::exception&)
    {
      std::cerr << "Failed to deserialize Tessellation" << std::endl;
      tess.reset();
    }
  }
}
} // namespace model
} // namespace smtk
