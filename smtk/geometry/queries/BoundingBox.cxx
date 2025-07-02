//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/geometry/queries/BoundingBox.h"

namespace smtk
{
namespace geometry
{

std::array<double, 6> BoundingBox::operator()(const smtk::resource::PersistentObject::Ptr&) const
{
  return { { 1., 0., 1., 0., 1., 0. } };
}

} // namespace geometry
} // namespace smtk
