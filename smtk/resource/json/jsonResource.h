//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_json_jsonResource_h
#define smtk_resource_json_jsonResource_h

#include "smtk/resource/Resource.h"

#include "nlohmann/json.hpp"

// Define how resources are serialized.
namespace smtk
{
namespace resource
{
using json = nlohmann::json;

void to_json(json&, const ResourcePtr&);

void from_json(const json&, ResourcePtr&);
}
}

#endif
