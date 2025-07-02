//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/geometry/BoundingBox.h"

#include "smtk/extension/vtk/geometry/Backend.h"
#include "smtk/extension/vtk/geometry/Geometry.h"

#include "smtk/geometry/Geometry.h"
#include "smtk/geometry/Resource.h"

#include <vtkDataSet.h>

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace geometry
{
std::array<double, 6> BoundingBox::operator()(
  const smtk::resource::PersistentObjectPtr& object) const
{
  std::array<double, 6> returnValue{ { 1., 0., 1., 0., 1., 0. } };

  smtk::geometry::Resource::Ptr resource =
    std::dynamic_pointer_cast<smtk::geometry::Resource>(object);
  if (!resource)
  {
    if (
      smtk::resource::Component::Ptr component =
        std::dynamic_pointer_cast<smtk::resource::Component>(object))
    {
      resource = std::dynamic_pointer_cast<smtk::geometry::Resource>(component->resource());
    }
  }

  if (!resource)
  {
    return returnValue;
  }

  smtk::extension::vtk::geometry::Backend vtk;
  const auto& geometry = resource->geometry(vtk);
  if (geometry)
  {
    vtkDataSet* data = nullptr;
    try
    {
      const auto& vtkGeometry =
        dynamic_cast<const smtk::extension::vtk::geometry::Geometry&>(*geometry);

      data = vtkDataSet::SafeDownCast(vtkGeometry.data(object));
    }
    catch (std::bad_cast&)
    {
      return returnValue;
    }

    if (data)
    {
      data->GetBounds(returnValue.data());
    }
  }

  return returnValue;
}
} // namespace geometry
} // namespace vtk
} // namespace extension
} // namespace smtk
