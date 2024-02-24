//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/geometry/Geometry.h"

#include "smtk/resource/CopyOptions.h"

namespace smtk
{
namespace geometry
{

bool Geometry::copyGeometry(const UniquePtr& sourceGeometry, smtk::resource::CopyOptions& options)
{
  (void)sourceGeometry;
  return !options.copyGeometry();
}

} // namespace geometry
} // namespace smtk
