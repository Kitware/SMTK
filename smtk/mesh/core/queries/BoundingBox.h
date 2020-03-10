//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_mesh_BoundingBox_h
#define smtk_mesh_BoundingBox_h

#include "smtk/CoreExports.h"

#include "smtk/geometry/queries/BoundingBox.h"

#include "smtk/mesh/utility/Metrics.h"

namespace smtk
{
namespace mesh
{
/**\brief An API for computing the bounding box for a geometric resource
  * or component.
  */
struct SMTKCORE_EXPORT BoundingBox
  : public smtk::resource::query::DerivedFrom<BoundingBox, smtk::geometry::BoundingBox>
{
  std::array<double, 6> operator()(const smtk::resource::PersistentObjectPtr& object) const override
  {
    smtk::mesh::Resource::Ptr meshResource =
      std::dynamic_pointer_cast<smtk::mesh::Resource>(object);
    if (meshResource)
    {
      return utility::extent(meshResource->meshes());
    }

    smtk::mesh::Component::Ptr meshComponent =
      std::dynamic_pointer_cast<smtk::mesh::Component>(object);
    if (meshComponent)
    {
      return utility::extent(meshComponent->mesh());
    }

    return this->Parent::operator()(object);
  }
};
}
}

#endif
