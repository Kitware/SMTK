//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/oscillator/plugin/oscillatorAuxiliaryGeometryExtension.h"

#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "smtk/AutoInit.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"

#include "vtkBoundingBox.h"
#include "vtkClipClosedSurface.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataObject.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkGraph.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkPlaneCollection.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkStringArray.h"
#include "vtkSuperquadricSource.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkUnstructuredGrid.h"

#include <ctime>
#include <list>
#include <map>

#include <cstdlib> // for atexit()

using namespace smtk::model;

oscillatorAuxiliaryGeometryExtension::oscillatorAuxiliaryGeometryExtension()
{
  oscillatorAuxiliaryGeometryExtension::ensureCache();
}

oscillatorAuxiliaryGeometryExtension::~oscillatorAuxiliaryGeometryExtension() = default;

bool oscillatorAuxiliaryGeometryExtension::canHandleAuxiliaryGeometry(
  smtk::model::AuxiliaryGeometry& entity,
  std::vector<double>& bboxOut)
{
  if (!entity.isValid() || !entity.hasStringProperty("oscillator_type"))
  {
    return false;
  }

  std::time_t cachedTime;
  auto dataset = vtkAuxiliaryGeometryExtension::fetchCachedGeometry(entity, cachedTime);
  // FIXME: Test whether dataset is stale... do not return if so.
  if (dataset)
  {
    auto cachedFloatTime = static_cast<double>(cachedTime);
    const auto& entityMTime = entity.floatProperty("mtime");
    if (!entityMTime.empty() && entityMTime[0] < cachedFloatTime)
    {
      return oscillatorAuxiliaryGeometryExtension::updateBoundsFromDataSet(
        entity, bboxOut, dataset);
    }
  }

  constexpr bool trimCache = true;
  dataset = oscillatorAuxiliaryGeometryExtension::generateOscillatorRepresentation(entity);
  std::time_t mtime;
  std::time(&mtime);
  oscillatorAuxiliaryGeometryExtension::addCacheGeometry(dataset, entity, mtime, trimCache);
  return oscillatorAuxiliaryGeometryExtension::updateBoundsFromDataSet(entity, bboxOut, dataset);
}

vtkSmartPointer<vtkDataObject>
oscillatorAuxiliaryGeometryExtension::generateOscillatorRepresentation(
  const AuxiliaryGeometry& oscillatorEntity)
{
  oscillatorAuxiliaryGeometryExtension::ensureCache();
  if (oscillatorEntity.stringProperty("oscillator_type")[0] == "source")
  {
    return oscillatorAuxiliaryGeometryExtension::generateOscillatorSourceRepresentation(
      oscillatorEntity);
  }
  return vtkSmartPointer<vtkDataObject>();
}

vtkSmartPointer<vtkDataObject>
oscillatorAuxiliaryGeometryExtension::generateOscillatorSourceRepresentation(
  const AuxiliaryGeometry& src)
{
  oscillatorAuxiliaryGeometryExtension::ensureCache();

  // Extract geometric specification from properties
  smtk::model::FloatList center;
  smtk::model::FloatList radius;
  bool ok = true;
  if (src.hasFloatProperty("center"))
  {
    center = src.floatProperty("center");
  }
  else
  {
    ok = false;
  }
  if (src.hasFloatProperty("radius"))
  {
    radius = src.floatProperty("radius");
  }
  else
  {
    ok = false;
  }
  ok &= (center.size() == 3);
  ok &= (radius.size() == 1) || (radius.size() == 3);

  if (!ok)
  {
    return vtkSmartPointer<vtkDataObject>();
  }

  auto ellipsoidSource = vtkSmartPointer<vtkSuperquadricSource>::New();
  if (radius.size() == 1)
  {
    ellipsoidSource->SetSize(radius[0]);
  }
  else
  {
    ellipsoidSource->SetScale(radius.data());
  }
  ellipsoidSource->SetCenter(center.data());
  ellipsoidSource->Update();
  auto geom = vtkSmartPointer<vtkPolyData>::New();
  geom->ShallowCopy(ellipsoidSource->GetOutputDataObject(0));
  return geom;
}

smtkDeclareExtension(
  /*SMTKRGGSESSION_EXPORT*/,
  oscillator_auxiliary_geometry,
  oscillatorAuxiliaryGeometryExtension);
smtkComponentInitMacro(smtk_oscillator_auxiliary_geometry_extension);
