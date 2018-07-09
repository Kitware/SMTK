//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/vtk/json/jsonResource.h"

#include "smtk/bridge/vtk/Resource.h"

#include "smtk/model/json/jsonResource.h"

// Define how vtk resources are serialized.
namespace smtk
{
namespace bridge
{
namespace vtk
{
using json = nlohmann::json;
void to_json(json& j, const smtk::bridge::vtk::Resource::Ptr& resource)
{
  smtk::model::to_json(j, std::static_pointer_cast<smtk::model::Resource>(resource));
}

void from_json(const json& j, smtk::bridge::vtk::Resource::Ptr& resource)
{
  if (resource == nullptr)
  {
    resource = smtk::bridge::vtk::Resource::create();
  }
  auto temp = std::static_pointer_cast<smtk::model::Resource>(resource);
  smtk::model::from_json(j, temp);
}
}
}
}
