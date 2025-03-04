//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/geometry/ClosestPoint.h"

#include "smtk/extension/vtk/geometry/Backend.h"
#include "smtk/extension/vtk/geometry/Geometry.h"
#include "smtk/extension/vtk/model/vtkAuxiliaryGeometryExtension.h"

#include "smtk/geometry/Geometry.h"
#include "smtk/geometry/Resource.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Entity.h"

#include <vtkDataSet.h>
#include <vtkPointSet.h>

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace geometry
{
std::array<double, 3> ClosestPoint::operator()(
  const smtk::resource::ComponentPtr& component,
  const std::array<double, 3>& input) const
{
  static constexpr const double nan = std::numeric_limits<double>::quiet_NaN();
  std::array<double, 3> returnValue{ { nan, nan, nan } };

  smtk::geometry::Resource::Ptr resource =
    std::dynamic_pointer_cast<smtk::geometry::Resource>(component->resource());

  if (!resource)
  {
    return returnValue;
  }

  vtkDataSet* data = nullptr;

  smtk::extension::vtk::geometry::Backend vtk;
  const auto& geometry = resource->geometry(vtk);
  if (geometry)
  {
    try
    {
      const auto& vtkGeometry =
        dynamic_cast<const smtk::extension::vtk::geometry::Geometry&>(*geometry);

      data = vtkDataSet::SafeDownCast(vtkGeometry.data(component));
    }
    catch (std::bad_cast&)
    {
      return returnValue;
    }
  }

  if (data == nullptr)
  {
    return returnValue;
  }

  vtkSmartPointer<vtkDataObject> cachedAuxData; // Keep here so it stays in scope

  // TODO: Handle composite data, not just vtkPointSet data.
  vtkPointSet* pdata = vtkPointSet::SafeDownCast(data);
  if (!pdata)
  {
    smtk::model::Entity::Ptr entity = std::dynamic_pointer_cast<smtk::model::Entity>(component);
    if (entity && entity->isAuxiliaryGeometry())
    { // It may be that we don't have a tessellation yet; create one if we can
      smtk::model::AuxiliaryGeometry aux(entity);
      std::vector<double> bbox;
      auto agext = vtkAuxiliaryGeometryExtension::create();
      if (agext->canHandleAuxiliaryGeometry(aux, bbox))
      {
        cachedAuxData = vtkAuxiliaryGeometryExtension::fetchCachedGeometry(aux);
        pdata = vtkPointSet::SafeDownCast(cachedAuxData);
      }
    }

    if (pdata && pdata->GetNumberOfPoints() > 0)
    {
      vtkIdType closestId = pdata->FindPoint(const_cast<double*>(input.data()));
      pdata->GetPoint(closestId, returnValue.data());
    }
  }

  return returnValue;
};
} // namespace geometry
} // namespace vtk
} // namespace extension
} // namespace smtk
