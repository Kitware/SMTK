//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/json/jsonArrangement.h"

#include "smtk/common/UUID.h"
#include "smtk/model/Arrangement.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace model
{
using json = nlohmann::json;
void to_json(json& j, const smtk::model::Arrangement& arr)
{
  if (arr.details().empty())
  {
    j = nullptr;
  }
  else
  {
    j = arr.details();
  }
}
void from_json(const json& j, smtk::model::Arrangement& arr)
{
  arr = smtk::model::Arrangement();
  if (!j.is_null())
  {
    arr.details() = j.get<std::vector<int>>();
  }
}
} // namespace model
} // namespace smtk
