//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_markup_SelectionFootprint_h
#define smtk_markup_SelectionFootprint_h

#include "smtk/geometry/queries/SelectionFootprint.h"

#include "smtk/geometry/Geometry.h"

#include "smtk/markup/Component.h"

#include <unordered_set>

namespace smtk
{
namespace markup
{

/**\brief Identify b-rep components highlighted to display a selection.
  *
  */
struct SMTKMARKUP_EXPORT SelectionFootprint
  : public smtk::resource::query::
      DerivedFrom<SelectionFootprint, smtk::geometry::SelectionFootprint>
{
  /**\brief Add the "selection footprint" of \a selectedObject to the \a footprint set.
    *
    * If a node has geometry, it is its own footprint.
    * Otherwise, we may add nodes connected via arcs until nodes with geometry are identified.
    *
    * Returns true if at least one persistent object was added to (or was already in)
    * the \a footprint set — either \a selectedObject itself or some other object.
    */
  bool operator()(
    smtk::resource::PersistentObject& selectedObject,
    std::unordered_set<smtk::resource::PersistentObject*>& footprint,
    const smtk::geometry::Backend& backend) const override
  {
    bool hasFootprint;
    if ((hasFootprint = this->addAllComponentsIfResource(selectedObject, footprint, backend)))
    {
      return hasFootprint;
    }
    auto* node = dynamic_cast<smtk::markup::Component*>(&selectedObject);
    if (!node)
    {
      return hasFootprint;
    }
    auto* resource = dynamic_cast<smtk::geometry::Resource*>(node->resource().get());
    if (!resource)
    {
      return hasFootprint;
    }
    auto& geom = resource->geometry(backend);
    hasFootprint |= this->addComponentFootprint(node, footprint, geom);
    return hasFootprint;
  }

  bool addComponentFootprint(
    smtk::markup::Component* node,
    std::unordered_set<smtk::resource::PersistentObject*>& footprint,
    std::unique_ptr<smtk::geometry::Geometry>& geom,
    bool stopRecursingAtGeometry = true) const
  {
    bool hasFootprint = false;
    if (node && geom)
    {
      if (geom->generationNumber(node->shared_from_this()) != smtk::geometry::Geometry::Invalid)
      {
        footprint.insert(node);
        hasFootprint = true;
        if (stopRecursingAtGeometry)
        {
          return hasFootprint;
        }
      }
      else
      {
        if (auto* group = dynamic_cast<Group*>(node))
        {
          group->outgoing<arcs::GroupsToMembers>().visit(
            [this, &hasFootprint, &geom, &footprint, &stopRecursingAtGeometry](
              const smtk::markup::Component* member) {
              hasFootprint |= this->addComponentFootprint(
                const_cast<smtk::markup::Component*>(member),
                footprint,
                geom,
                stopRecursingAtGeometry);
            });
        }
        else if (auto* field = dynamic_cast<Field*>(node))
        {
          field->outgoing<arcs::FieldsToShapes>().visit(
            [this, &hasFootprint, &geom, &footprint, &stopRecursingAtGeometry](
              const smtk::markup::SpatialData* shape) {
              hasFootprint |= this->addComponentFootprint(
                const_cast<smtk::markup::SpatialData*>(shape),
                footprint,
                geom,
                stopRecursingAtGeometry);
            });
        }
      }
    }
    return hasFootprint;
  }
};
} // namespace markup
} // namespace smtk

#endif
