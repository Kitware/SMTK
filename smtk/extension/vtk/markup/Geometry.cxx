//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/markup/Geometry.h"

#include "smtk/markup/Box.h"
#include "smtk/markup/Component.h"
#include "smtk/markup/ImageData.h"
#include "smtk/markup/Resource.h"
#include "smtk/markup/Sphere.h"
#include "smtk/markup/UnstructuredData.h"

#include "smtk/geometry/Generator.h"

#include "vtkCompositeDataSet.h"
#include "vtkCubeSource.h"
#include "vtkDataObjectTypes.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkSuperquadricSource.h"
#include "vtkUnstructuredGrid.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace markup
{

Geometry::Geometry(const std::shared_ptr<smtk::markup::Resource>& parent)
  : m_parent(parent)
{
}

smtk::geometry::Resource::Ptr Geometry::resource() const
{
  return std::dynamic_pointer_cast<smtk::geometry::Resource>(m_parent.lock());
}

void Geometry::queryGeometry(const smtk::resource::PersistentObject::Ptr& obj, CacheEntry& entry)
  const
{
  // Access the markup component
  auto component = std::dynamic_pointer_cast<smtk::markup::Component>(obj);
  if (!component)
  {
    entry.m_generation = Invalid;
    return;
  }

  entry.m_geometry = nullptr;

  // Check component-wide blanking
  if (auto* spatial = dynamic_cast<smtk::markup::SpatialData*>(component.get()))
  {
    if (spatial->isBlanked())
    {
      entry.m_generation = Invalid;
      return;
    }
  }

  if (auto* image = dynamic_cast<smtk::markup::ImageData*>(component.get()))
  {
    auto shape = image->shapeData();
    if (shape)
    {
      entry.m_geometry = vtkSmartPointer<vtkImageData>::New();
      entry.m_geometry->ShallowCopy(shape);
      // TODO: Add explicit global point/cell ID arrays?
    }
  }
  else if (auto* mesh = dynamic_cast<smtk::markup::UnstructuredData*>(component.get()))
  {
    auto shape = mesh->shapeData();
    if (shape)
    {
      entry.m_geometry = vtkDataObjectTypes::NewDataObject(shape->GetClassName());
      entry.m_geometry->ShallowCopy(shape);
      // TODO: Add explicit global point/cell ID arrays?
    }
  }
  else if (auto* analyticShape = dynamic_cast<smtk::markup::AnalyticShape*>(component.get()))
  {
    if (auto* box = dynamic_cast<smtk::markup::Box*>(analyticShape))
    {
      vtkNew<vtkCubeSource> boxSource;
      const auto& lo = box->range()[0];
      const auto& hi = box->range()[1];
      boxSource->SetBounds(lo[0], hi[0], lo[1], hi[1], lo[2], hi[2]);
      boxSource->Update();
      entry.m_geometry = vtkSmartPointer<vtkPolyData>::New();
      entry.m_geometry->ShallowCopy(boxSource->GetOutputDataObject(0));
    }
    else if (auto* sphere = dynamic_cast<smtk::markup::Sphere*>(analyticShape))
    {
      vtkNew<vtkSuperquadricSource> ellipsoidSource;
      const auto& center = sphere->center();
      const auto& radii = sphere->radius();
      ellipsoidSource->SetCenter(center.data());
      ellipsoidSource->SetScale(radii.data());
      ellipsoidSource->SetPhiRoundness(1.0);
      ellipsoidSource->SetThetaRoundness(1.0);
      ellipsoidSource->ToroidalOff();
      ellipsoidSource->Update();
      entry.m_geometry = vtkSmartPointer<vtkPolyData>::New();
      entry.m_geometry->ShallowCopy(ellipsoidSource->GetOutputDataObject(0));
    }
  }

  if (entry.m_geometry)
  {
    Geometry::addTransformArrayIfPresent(entry.m_geometry, obj);
    if (obj && !obj->name().empty())
    {
      entry.m_geometry->GetInformation()->Set(vtkCompositeDataSet::NAME(), obj->name().c_str());
    }

    ++entry.m_generation;
  }
  else
  {
    entry.m_generation = Invalid;
    return;
  }

  // If the object has color properties, apply them
  if (component->properties().contains<std::vector<double>>("color"))
  {
    Geometry::addColorArray(
      entry.m_geometry, component->properties().at<std::vector<double>>("color"));
  }
}

int Geometry::dimension(const smtk::resource::PersistentObject::Ptr& obj) const
{
  auto component = std::dynamic_pointer_cast<smtk::markup::Component>(obj);
  if (component)
  {
    // TODO: base this on actual geometry
    return 2;
  }
  return 0;
}

Geometry::Purpose Geometry::purpose(const smtk::resource::PersistentObject::Ptr&) const
{
  return Geometry::Surface;
}

void Geometry::update() const
{
  // Do nothing. Operations in smtk markup set content as needed.
}

void Geometry::geometricBounds(const DataType& geom, BoundingBox& bbox) const
{
  auto* pset = vtkPointSet::SafeDownCast(geom);
  if (pset)
  {
    pset->GetBounds(bbox.data());
    return;
  }
  auto* comp = vtkCompositeDataSet::SafeDownCast(geom);
  if (comp)
  {
    comp->GetBounds(bbox.data());
    return;
  }

  // Invalid bounding box:
  bbox[0] = bbox[2] = bbox[4] = 0.0;
  bbox[1] = bbox[3] = bbox[5] = -1.0;
}
} // namespace markup
} // namespace vtk
} // namespace extension
} // namespace smtk
