//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/json/jsonSurrogate.h"

#include "smtk/resource/Surrogate.h"

#include "smtk/common/json/jsonUUID.h"

namespace smtk
{
namespace resource
{
void to_json(json& j, const Surrogate& surrogate)
{
  // j["index"] = surrogate.index();
  j["type"] = surrogate.typeName();
  // j["id"] = surrogate.id();
  // j["location"] = surrogate.location();
}

Surrogate from_json(const json& j)
{
  return Surrogate(j["index"], j["type"], j["id"], j["location"]);
}
}
}
