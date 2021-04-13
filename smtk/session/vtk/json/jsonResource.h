//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_vtk_json_jsonResource_h
#define smtk_session_vtk_json_jsonResource_h

#include "smtk/session/vtk/Exports.h"

#include "smtk/session/vtk/Resource.h"

#include "smtk/model/json/jsonResource.h"

#include "nlohmann/json.hpp"

// Define how vtk resources are serialized.
namespace smtk
{
namespace session
{
namespace vtk
{
using json = nlohmann::json;
SMTKVTKSESSION_EXPORT void to_json(json& j, const smtk::session::vtk::Resource::Ptr& resource);

SMTKVTKSESSION_EXPORT void from_json(const json& j, smtk::session::vtk::Resource::Ptr& resource);
} // namespace vtk
} // namespace session
} // namespace smtk

#endif
