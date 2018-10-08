//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_json_jsonCollection_h
#define __smtk_mesh_json_jsonCollection_h

#include "smtk/mesh/core/Collection.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace mesh
{
namespace json
{
void to_json(nlohmann::json&, const CollectionPtr&);

void from_json(const nlohmann::json&, CollectionPtr&);
}
}
}

#endif
