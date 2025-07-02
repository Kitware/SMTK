//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_geometry_BoundingBox_h
#define smtk_geometry_BoundingBox_h

#include "smtk/CoreExports.h"

#include "smtk/resource/PersistentObject.h"
#include "smtk/resource/query/DerivedFrom.h"
#include "smtk/resource/query/Query.h"

#include <array>

namespace smtk
{
namespace geometry
{

/**\brief An API for computing the bounding box for a geometric resource
  * or component.
  */
struct SMTKCORE_EXPORT BoundingBox
  : public smtk::resource::query::DerivedFrom<BoundingBox, smtk::resource::query::Query>
{
  virtual std::array<double, 6> operator()(const smtk::resource::PersistentObject::Ptr&) const = 0;
};

} // namespace geometry
} // namespace smtk

#endif
