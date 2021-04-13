//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_geometry_SelectionFootprint_h
#define smtk_geometry_SelectionFootprint_h

#include "smtk/CoreExports.h"

#include "smtk/geometry/Geometry.h"
#include "smtk/geometry/Resource.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/query/DerivedFrom.h"
#include "smtk/resource/query/Query.h"

#include <unordered_set>

namespace smtk
{
namespace geometry
{

/**\brief An API for determining which components to render to display a selection.
  *
  * Some components may not have any renderable geometry.
  * However, users might still wish to select them.
  * Examples include groups and volume cells in the smtk::model::Resource;
  * they typically do not have geometry but their members or boundary faces do.
  * When a group or volume is selected, we need a way to fetch the objects that
  * should be highlighted in the stead of the group or volume.
  *
  * This query allows SMTK's geometry representation to choose which components
  * to render with the "selection" appearance.
  * Normally, it will return the component itself if it has renderable geometry but
  * otherwise will look for related components that might be highlighted to
  * indicate the selection.
  */
struct SMTKCORE_EXPORT SelectionFootprint
  : public smtk::resource::query::DerivedFrom<SelectionFootprint, smtk::resource::query::Query>
{
  /// Add the "selection footprint" of \a selectedObject to the \a footprint set.
  ///
  /// The \a footprint set is a container of persistent objects that should be
  /// rendered as "selected" instead of the input \a selectedObject, usually because
  /// \a selectedObject has no visual representation itself.
  ///
  /// Returns true if at least one persistent object was added to (or was already in)
  /// the \a footprint set — either \a selectedObject itself or some other object.
  virtual bool operator()(
    smtk::resource::PersistentObject& selectedObject,
    std::unordered_set<smtk::resource::PersistentObject*>& footprint,
    const smtk::geometry::Backend& backend) const = 0;

  /// Returns true iff the given \a object has geometry for \a backend .
  bool hasGeometry(smtk::resource::PersistentObject& object, const smtk::geometry::Backend& backend)
    const
  {
    auto resource = dynamic_cast<smtk::geometry::Resource*>(&object);
    if (!resource)
    {
      auto component = dynamic_cast<smtk::resource::Component*>(&object);
      if (component)
      {
        resource = dynamic_cast<smtk::geometry::Resource*>(component->resource().get());
      }
    }
    if (!resource)
    {
      return false;
    }
    auto& geom = resource->geometry(backend);
    if (
      !geom ||
      geom->generationNumber(object.shared_from_this()) == smtk::geometry::Geometry::Invalid)
    {
      return false;
    }
    return true;
  }

  /// If \a object is a Resource, add all the components it owns that have geometry.
  ///
  /// Return false when \a object is not a Resource or when none of its components
  /// have any geometry for the given backend.
  bool addAllComponentsIfResource(
    smtk::resource::PersistentObject& object,
    std::unordered_set<smtk::resource::PersistentObject*>& footprint,
    const smtk::geometry::Backend& backend) const
  {
    auto resource = dynamic_cast<smtk::geometry::Resource*>(&object);
    if (!resource)
    {
      return false;
    }
    auto& geom = resource->geometry(backend);
    if (!geom)
    {
      return false;
    }
    if (geom->generationNumber(object.shared_from_this()) == smtk::geometry::Geometry::Invalid)
    {
      bool hasFootprint = false;
      auto visitor = [&hasFootprint, &footprint, &geom](
                       const resource::PersistentObject::Ptr& obj,
                       smtk::geometry::Geometry::GenerationNumber) -> bool {
        if (geom->generationNumber(obj) != smtk::geometry::Geometry::Invalid)
        {
          hasFootprint = true;
          footprint.insert(obj.get());
        }
        return true;
      };
      geom->visit(visitor);
      return hasFootprint;
    }
    return true; // Resource has geometry.
  }
};
} // namespace geometry
} // namespace smtk

#endif
