//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_attribute_SelectionFootprint_h
#define smtk_attribute_SelectionFootprint_h

#include "smtk/geometry/queries/SelectionFootprint.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItem.h"

#include "smtk/resource/query/BadTypeError.h"

namespace smtk
{
namespace attribute
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
  : public smtk::resource::query::DerivedFrom<SelectionFootprint,
      smtk::geometry::SelectionFootprint>
{
  /// Return the "selection footprint" of \a selectedObject.
  ///
  /// This is a set of persistent objects that should be rendered as "selected"
  /// instead of the input \a selectedObject, usually because \a selectedObject
  /// has no visual representation itself.
  virtual bool operator()(smtk::resource::PersistentObject& selectedObject,
    std::unordered_set<smtk::resource::PersistentObject*>& footprint,
    const smtk::geometry::Backend& backend) const override
  {
    bool hasFootprint = false;
    auto attr = dynamic_cast<smtk::attribute::Attribute*>(&selectedObject);
    if (attr)
    {
      // If the attribute has geometry, use it:
      if (this->hasGeometry(*attr, backend))
      {
        footprint.insert(attr);
        hasFootprint = true;
        return hasFootprint;
      }
      else
      {
        // Insert any associations that have geometry
        auto assocs = attr->associations();
        if (assocs)
        {
          for (std::size_t ii = 0; ii < assocs->numberOfValues(); ++ii)
          {
            if (!assocs->isSet(ii))
            {
              continue;
            }
            const auto& obj = assocs->value(ii);
            // Don't call ourselves recursively because obj may be
            // (or be owned by) a different type of resource that has
            // a different implementation. Instead, ask obj's resource
            // to create a query for us.
            try
            {
              smtk::geometry::SelectionFootprint& subquery =
                smtk::resource::queryForObject<smtk::geometry::SelectionFootprint>(*obj);
              hasFootprint |= subquery(*obj, footprint, backend);
            }
            catch (smtk::resource::query::BadTypeError&)
            {
              // Do nothing.
            }
          }
        }
      }
    }
    return hasFootprint;
  }
};
}
}

#endif
