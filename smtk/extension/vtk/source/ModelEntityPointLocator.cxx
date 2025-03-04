//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/source/ModelEntityPointLocator.h"
#include "smtk/extension/vtk/model/vtkAuxiliaryGeometryExtension.h"

#include "smtk/extension/vtk/geometry/Backend.h"
#include "smtk/extension/vtk/geometry/Geometry.h"

#include "smtk/AutoInit.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Resource.h"

#include "vtkCellLocator.h"
#include "vtkGenericCell.h"
#include "vtkNew.h"
#include "vtkPointSet.h"
#include "vtkSmartPointer.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace source
{

ModelEntityPointLocator::ModelEntityPointLocator() = default;

ModelEntityPointLocator::~ModelEntityPointLocator() = default;

bool ModelEntityPointLocator::closestPointOn(
  const smtk::model::EntityRef& entity,
  std::vector<double>& closestPoints,
  const std::vector<double>& sourcePoints,
  bool snapToPoint)
{
  smtk::extension::vtk::geometry::Backend vtk;
  const auto& geometry = entity.resource()->geometry(vtk);
  if (!geometry)
  {
    return false;
  }

  vtkDataSet* data = nullptr;
  try
  {
    const auto& vtkGeometry =
      dynamic_cast<const smtk::extension::vtk::geometry::Geometry&>(*geometry);

    data = vtkDataSet::SafeDownCast(vtkGeometry.data(entity.component()));
  }
  catch (std::bad_cast&)
  {
    return false;
  }

  if (!data)
  {
    return false;
  }

  vtkSmartPointer<vtkDataObject> cachedAuxData; // Keep here so it stays in scope

  // TODO: Handle composite data, not just vtkPointSet data.
  vtkPointSet* pdata = vtkPointSet::SafeDownCast(data);
  if (!pdata && entity.isAuxiliaryGeometry())
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

  if (snapToPoint)
  {
    if (pdata && pdata->GetNumberOfPoints() > 0)
    {
      int npts = static_cast<int>(sourcePoints.size()) / 3;
      closestPoints.resize(npts * 3);
      for (int ii = 0; ii < npts; ++ii)
      {
        vtkIdType closestId = pdata->FindPoint(const_cast<double*>(&sourcePoints[3 * ii]));
        pdata->GetPoint(closestId, &closestPoints[3 * ii]);
      }
      return true;
    }
  }
  else
  {
    // Create the standard locator
    vtkSmartPointer<vtkCellLocator> cellLocator = vtkSmartPointer<vtkCellLocator>::New();
    cellLocator->SetDataSet(pdata);
    cellLocator->BuildLocator();

    vtkSmartPointer<vtkGenericCell> cell = vtkSmartPointer<vtkGenericCell>::New();

    vtkIdType cellId;
    int subId;
    double dist2;

    int npts = static_cast<int>(sourcePoints.size()) / 3;
    closestPoints.resize(npts * 3);
    for (int ii = 0; ii < npts; ++ii)
    {
      cellLocator->FindClosestPoint(
        const_cast<double*>(&sourcePoints[3 * ii]),
        &closestPoints[3 * ii],
        cell,
        cellId,
        subId,
        dist2);
    }
    return true;
  }

  return false;
}

bool ModelEntityPointLocator::randomPoint(
  const smtk::model::EntityRef& entity,
  const std::size_t nPoints,
  std::vector<double>& points,
  const std::size_t seed)
{
  // TODO: fill me in!
  (void)entity;
  (void)nPoints;
  (void)points;
  (void)seed;
  return false;
}
} // namespace source
} // namespace vtk
} // namespace extension
} // namespace smtk

smtkDeclareExtension(
  VTKSMTKSOURCEEXT_EXPORT,
  vtk_model_entity_point_locator,
  smtk::extension::vtk::source::ModelEntityPointLocator);

smtkComponentInitMacro(smtk_vtk_model_entity_point_locator_extension);
