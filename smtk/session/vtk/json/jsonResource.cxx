//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/vtk/json/jsonResource.h"

#include "smtk/session/vtk/Resource.h"

#include "smtk/model/json/jsonResource.h"

// Define how vtk resources are serialized.
namespace smtk
{
namespace session
{
namespace vtk
{
using json = nlohmann::json;
void to_json(json& j, const smtk::session::vtk::Resource::Ptr& resource)
{
  smtk::model::to_json(j, std::static_pointer_cast<smtk::model::Resource>(resource));
}

void from_json(const json& j, smtk::session::vtk::Resource::Ptr& resource)
{
  if (resource == nullptr)
  {
    resource = smtk::session::vtk::Resource::create();
  }
  auto temp = std::static_pointer_cast<smtk::model::Resource>(resource);
  smtk::model::from_json(j, temp);
}
} // namespace vtk
} // namespace session
} // namespace smtk
