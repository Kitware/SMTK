//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_model_SelectionFootprint_h
#define smtk_model_SelectionFootprint_h

#include "smtk/geometry/queries/SelectionFootprint.h"

#include "smtk/geometry/Geometry.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"

#include <unordered_set>

namespace smtk
{
namespace model
{

/**\brief Identify model components highlighted to display a selection.
  *
  */
struct SMTKCORE_EXPORT SelectionFootprint
  : public smtk::resource::query::
      DerivedFrom<SelectionFootprint, smtk::geometry::SelectionFootprint>
{
  /// Add the "selection footprint" of \a selectedObject to the \a footprint set.
  ///
  /// If a model entity has geometry, it is its own footprint.
  /// Otherwise,
  /// 1. If it is a cell, we add its boundaries (if they have geometry)
  /// 2. If it is a group, we add its members' footprints.
  /// 3. If it is a model, we add its free cells and all their boundaries.
  ///
  /// Returns true if at least one persistent object was added to (or was already in)
  /// the \a footprint set — either \a selectedObject itself or some other object.
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
    auto ent = dynamic_cast<smtk::model::Entity*>(&selectedObject);
    if (!ent)
    {
      return hasFootprint;
    }
    auto resource = dynamic_cast<smtk::geometry::Resource*>(ent->resource().get());
    if (!resource)
    {
      return hasFootprint;
    }
    auto& geom = resource->geometry(backend);
    hasFootprint |= this->addComponentFootprint(ent, footprint, geom);
    return hasFootprint;
  }

  bool addComponentFootprint(
    smtk::model::Entity* ent,
    std::unordered_set<smtk::resource::PersistentObject*>& footprint,
    std::unique_ptr<smtk::geometry::Geometry>& geom,
    bool stopRecursingAtGeometry = true) const
  {
    bool hasFootprint = false;
    if (ent && geom)
    {
      if (geom->generationNumber(ent->shared_from_this()) != smtk::geometry::Geometry::Invalid)
      {
        footprint.insert(ent);
        hasFootprint = true;
        if (stopRecursingAtGeometry)
        {
          return hasFootprint;
        }
      }

      if (ent->isGroup())
      {
        auto members =
          smtk::model::Group(ent->shared_from_this()).members<smtk::model::EntityRefs>();
        for (auto member : members)
        {
          hasFootprint |= this->addComponentFootprint(member.entityRecord().get(), footprint, geom);
        }
      }
      else if (ent->isModel())
      {
        auto model = smtk::model::Model(ent->shared_from_this());
        auto cells = model.cellsAs<smtk::model::EntityRefs>();
        for (const auto& cell : cells)
        {
          hasFootprint |=
            this->addComponentFootprint(cell.entityRecord().get(), footprint, geom, false);
        }

        // Because groups may have their own geometry... process them, too.
        auto groups = model.groups();
        for (const auto& group : groups)
        {
          hasFootprint |= this->addComponentFootprint(group.entityRecord().get(), footprint, geom);
        }

        // TODO: Auxiliary geometry may also be handled by a separate representation.
        //       Need to ensure that representation also renders selection properly.
        auto auxGeoms = model.auxiliaryGeometry();
        // Convert auxGeoms to EntityRefs to match SelectComponentFootprint() API:
        smtk::model::EntityRefs auxEnts;
        for (const auto& auxGeom : auxGeoms)
        {
          hasFootprint |=
            this->addComponentFootprint(auxGeom.entityRecord().get(), footprint, geom);
        }
      }
      else if (ent->isCellEntity())
      {
        auto bdys = smtk::model::CellEntity(ent->shared_from_this())
                      .boundingCellsAs<smtk::model::EntityRefs>();
        for (const auto& bdy : bdys)
        {
          hasFootprint |= this->addComponentFootprint(bdy.entityRecord().get(), footprint, geom);
        }
      }
    }
    return hasFootprint;
  }
};
} // namespace model
} // namespace smtk

#endif
