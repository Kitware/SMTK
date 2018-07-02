//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_bridge_vtk_json_jsonResource_h
#define smtk_bridge_vtk_json_jsonResource_h

#include "smtk/bridge/vtk/Exports.h"

#include "smtk/bridge/vtk/Resource.h"

#include "smtk/model/json/jsonResource.h"

#include "nlohmann/json.hpp"

// Define how vtk resources are serialized.
namespace smtk
{
namespace bridge
{
namespace vtk
{
using json = nlohmann::json;
SMTKVTKSESSION_EXPORT void to_json(json& j, const smtk::bridge::vtk::Resource::Ptr& resource);

SMTKVTKSESSION_EXPORT void from_json(const json& j, smtk::bridge::vtk::Resource::Ptr& resource);
}
}
}

#endif
