//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/json/jsonResource.h"

// Define how resources are serialized.
namespace smtk
{
namespace resource
{
void to_json(json& j, const ResourcePtr& resource)
{
  j["type"] = resource->uniqueName();
}

void from_json(const json&, ResourcePtr&)
{
}
}
}
