//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_mesh_json_jsonHandleRange_h
#define smtk_mesh_json_jsonHandleRange_h

#include "smtk/mesh/core/Handle.h"

#include "nlohmann/json.hpp"

// Define how resources are serialized.
namespace smtk
{
namespace resource
{
using json = nlohmann::json;

void to_json(json&, const smtk::mesh::HandleRange&);

void from_json(const json&, smtk::mesh::HandleRange&);
}
}

#endif
