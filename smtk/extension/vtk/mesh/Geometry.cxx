//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/mesh/Geometry.h"

#include "smtk/extension/vtk/io/mesh/ExportVTKData.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/utility/Metrics.h"

#include "smtk/geometry/Generator.h"

#include <vtkCompositeDataSet.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace mesh
{

Geometry::Geometry(const std::shared_ptr<smtk::mesh::Resource>& parent)
  : m_parent(parent)
{
}

smtk::geometry::Resource::Ptr Geometry::resource() const
{
  return std::dynamic_pointer_cast<smtk::geometry::Resource>(m_parent.lock());
}

void Geometry::queryGeometry(
  const smtk::resource::PersistentObject::Ptr& obj, CacheEntry& entry) const
{
  // Access the mesh component
  auto component = std::dynamic_pointer_cast<smtk::mesh::Component>(obj);
  if (!component || !component->mesh().isValid())
  {
    entry.m_generation = Invalid;
    return;
  }

  // Convert the meshset into a vtkPolyData
  smtk::extension::vtk::io::mesh::ExportVTKData exportVTKData;
  entry.m_geometry = vtkSmartPointer<vtkPolyData>::New();

  exportVTKData(component->mesh(), vtkPolyData::SafeDownCast(entry.m_geometry));

  ++entry.m_generation;

  // If the object has color properties, apply them
  if (component->properties().contains<std::vector<double> >("color"))
  {
    Geometry::addColorArray(
      entry.m_geometry, component->properties().at<std::vector<double> >("color"));
  }
}

int Geometry::dimension(const smtk::resource::PersistentObject::Ptr& obj) const
{
  auto component = std::dynamic_pointer_cast<smtk::mesh::Component>(obj);
  if (component && component->mesh().isValid())
  {
    int highestDim = static_cast<int>(smtk::mesh::utility::highestDimension(component->mesh()));

    // While the geometry may represent a 3-dimensional mesh, the geometric
    // representation is always a vtkPolyData, which has highest dimension 2.
    return (highestDim <= 2 ? highestDim : 2);
  }
  return 0;
}

Geometry::Purpose Geometry::purpose(const smtk::resource::PersistentObject::Ptr&) const
{
  return Geometry::Surface;
}

void Geometry::update() const
{
  // Do nothing. Operations in smtk mesh set content as needed.
}

void Geometry::geometricBounds(const DataType& geom, BoundingBox& bbox) const
{
  auto pset = vtkPointSet::SafeDownCast(geom);
  if (pset)
  {
    pset->GetBounds(bbox.data());
    return;
  }
  auto comp = vtkCompositeDataSet::SafeDownCast(geom);
  if (comp)
  {
    comp->GetBounds(bbox.data());
    return;
  }

  // Invalid bounding box:
  bbox[0] = bbox[2] = bbox[4] = 0.0;
  bbox[1] = bbox[3] = bbox[5] = -1.0;
}
}
}
}
}
