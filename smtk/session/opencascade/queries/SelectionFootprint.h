//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_SelectionFootprint_h
#define smtk_session_opencascade_SelectionFootprint_h

#include "smtk/geometry/queries/SelectionFootprint.h"

#include "smtk/geometry/Geometry.h"

#include "smtk/session/opencascade/Shape.h"

#include <unordered_set>

namespace smtk
{
namespace session
{
namespace opencascade
{

/**\brief Identify b-rep components highlighted to display a selection.
  *
  */
struct SMTKCORE_EXPORT SelectionFootprint
  : public smtk::resource::query::DerivedFrom<SelectionFootprint,
      smtk::geometry::SelectionFootprint>
{
  /// Add the "selection footprint" of \a selectedObject to the \a footprint set.
  ///
  /// If a shape entity has geometry, it is its own footprint.
  /// Otherwise, add its subshapes recursively until shapes with geometry are identified.
  ///
  /// Returns true if at least one persistent object was added to (or was already in)
  /// the \a footprint set — either \a selectedObject itself or some other object.
  virtual bool operator()(smtk::resource::PersistentObject& selectedObject,
    std::unordered_set<smtk::resource::PersistentObject*>& footprint,
    const smtk::geometry::Backend& backend) const override
  {
    bool hasFootprint;
    if ((hasFootprint = this->addAllComponentsIfResource(selectedObject, footprint, backend)))
    {
      return hasFootprint;
    }
    auto shape = dynamic_cast<Shape*>(&selectedObject);
    if (!shape)
    {
      return hasFootprint;
    }
    auto resource = dynamic_cast<smtk::geometry::Resource*>(shape->resource().get());
    if (!resource)
    {
      return hasFootprint;
    }
    auto& geom = resource->geometry(backend);
    hasFootprint |= this->addComponentFootprint(shape, footprint, geom);
    return hasFootprint;
  }

  bool addComponentFootprint(Shape* shape,
    std::unordered_set<smtk::resource::PersistentObject*>& footprint,
    std::unique_ptr<smtk::geometry::Geometry>& geom, bool stopRecursingAtGeometry = true) const
  {
    bool hasFootprint = false;
    if (shape && geom)
    {
      if (geom->generationNumber(shape->shared_from_this()) != smtk::geometry::Geometry::Invalid)
      {
        footprint.insert(shape);
        hasFootprint = true;
        if (stopRecursingAtGeometry)
        {
          return hasFootprint;
        }
      }
      else
      {
        shape->visit<Children>(
          [this, &footprint, &geom, &stopRecursingAtGeometry, &hasFootprint](const Shape& child) {
            hasFootprint |= this->addComponentFootprint(
              const_cast<Shape*>(&child), footprint, geom, stopRecursingAtGeometry);
            return false; // Keep iterating children.
          });
      }
    }
    return hasFootprint;
  }
};
}
}
}

#endif
