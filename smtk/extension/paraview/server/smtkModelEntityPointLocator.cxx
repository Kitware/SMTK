//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/smtkModelEntityPointLocator.h"
#include "smtk/extension/vtk/source/vtkAuxiliaryGeometryExtension.h"

#include "smtk/AutoInit.h"
#include "smtk/extension/paraview/server/vtkPVModelSources.h"
#include "smtk/model/AuxiliaryGeometry.h"

#include "vtkNew.h"
#include "vtkPointSet.h"
#include "vtkSmartPointer.h"

smtkModelEntityPointLocator::smtkModelEntityPointLocator() = default;

smtkModelEntityPointLocator::~smtkModelEntityPointLocator() = default;

bool smtkModelEntityPointLocator::closestPointOn(const smtk::model::EntityRef& entity,
  std::vector<double>& closestPoints, const std::vector<double>& sourcePoints, bool snapToPoint)
{
  if (snapToPoint == false)
  {
    return false;
  }
  vtkSmartPointer<vtkDataObject> cachedAuxData; // Keep here so it stays in scope
  // TODO: Handle composite data, not just vtkPointSet data.
  vtkPointSet* pdata = vtkPointSet::SafeDownCast(vtkPVModelSources::findModelEntity(entity));
  if (!pdata && entity.isAuxiliaryGeometry())
  { // It may be that we don't have a tessellation yet; create one if we can
    smtk::model::AuxiliaryGeometry aux(entity);
    std::vector<double> bbox;
    auto agext = vtkAuxiliaryGeometryExtension::create();
    if (agext->canHandleAuxiliaryGeometry(aux, bbox))
    {
      cachedAuxData = agext->fetchCachedGeometry(aux);
      pdata = vtkPointSet::SafeDownCast(cachedAuxData);
    }
  }
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
  return false;
}

bool smtkModelEntityPointLocator::randomPoint(const smtk::model::EntityRef& entity,
  const std::size_t nPoints, std::vector<double>& points, const std::size_t seed)
{
  // TODO: fill me in!
  (void)entity;
  (void)nPoints;
  (void)points;
  (void)seed;
  return false;
}

smtkDeclareExtension(
  SMTKPVSERVEREXT_EXPORT, model_entity_point_locator, smtkModelEntityPointLocator);

smtkComponentInitMacro(smtk_model_entity_point_locator_extension);
