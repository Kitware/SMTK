//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_mesh_BoundingBox_h
#define smtk_session_mesh_BoundingBox_h

#include "smtk/CoreExports.h"

#include "smtk/geometry/queries/BoundingBox.h"

#include "smtk/mesh/core/Component.h"

#include "smtk/session/mesh/Resource.h"

namespace smtk
{
namespace session
{
namespace mesh
{

/**\brief An API for computing the bounding box for a geometric resource
  * or component.
  */
struct SMTKCORE_EXPORT BoundingBox
  : public smtk::resource::query::DerivedFrom<BoundingBox, smtk::geometry::BoundingBox>
{
  virtual std::array<double, 6> operator()(const smtk::resource::PersistentObjectPtr& object) const
  {
    smtk::session::mesh::Resource::Ptr resource =
      std::dynamic_pointer_cast<smtk::session::mesh::Resource>(object);

    if (resource)
    {
      smtk::mesh::Resource::Ptr meshResource = resource->resource();
      return meshResource->queries().get<smtk::geometry::BoundingBox>().operator()(meshResource);
    }
    else if (auto entity = std::dynamic_pointer_cast<smtk::model::Entity>(object))
    {
      resource = std::dynamic_pointer_cast<smtk::session::mesh::Resource>(entity->resource());
      if (resource)
      {
        smtk::mesh::Resource::Ptr meshResource = resource->resource();
        return meshResource->queries().get<smtk::geometry::BoundingBox>().operator()(
          smtk::mesh::Component::create(meshResource->findAssociatedMeshes(object->id())));
      }
    }
    return smtk::geometry::BoundingBox::operator()(object);
  }
};
}
}
}

#endif
