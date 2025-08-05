//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/vtk/Geometry.h"

#include "smtk/session/vtk/Resource.h"
#include "smtk/session/vtk/Session.h"

#include "smtk/extension/vtk/model/vtkAuxiliaryGeometryExtension.h"

#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"

#include "smtk/geometry/Generator.h"

#include "vtkCellArray.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataObject.h"
#include "vtkGeometryFilter.h"
#include "vtkImageData.h"
#include "vtkNew.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkVersionMacros.h"

namespace smtk
{
namespace session
{
namespace vtk
{

Geometry::Geometry(const std::shared_ptr<smtk::session::vtk::Resource>& parent)
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
  auto ent = std::dynamic_pointer_cast<smtk::model::Entity>(obj);
  if (!ent)
  {
    entry.m_generation = Invalid;
    return;
  }
  auto resource = m_parent.lock();
  if (!resource)
  {
    entry.m_generation = Invalid;
    return;
  }
  auto session = resource->session();
  auto& entMap = session->reverseIdMap();
  vtkSmartPointer<vtkDataObject> data;
  auto it = entMap.find(smtk::model::EntityRef(resource, obj->id()));
  if (it == entMap.end())
  {
    // Auxiliary geometry is handled outside of the map used for other
    // entity types. For instance, it may be procedural or parametric.
    if (ent->isAuxiliaryGeometry())
    {
      smtk::model::AuxiliaryGeometry aux(ent);
      std::vector<double> bbox;
      auto ext = vtkAuxiliaryGeometryExtension::create();
      if (ext->canHandleAuxiliaryGeometry(aux, bbox))
      {
        data = vtkAuxiliaryGeometryExtension::fetchCachedGeometry(aux);
      }
    }
    if (!data)
    {
      entry.m_generation = Invalid;
      return;
    }
  }
  else
  {
    data = it->second.m_object;
  }
  if (data)
  {
    switch (data->GetDataObjectType())
    {
      case VTK_COMPOSITE_DATA_SET:
      case VTK_MULTIBLOCK_DATA_SET:
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 5, 0)
      case VTK_MULTIGROUP_DATA_SET:
      case VTK_HIERARCHICAL_DATA_SET:
      case VTK_HIERARCHICAL_BOX_DATA_SET:
#endif
        // The VTK session doesn't support composite data yet:
        entry.m_geometry = nullptr;
        entry.m_generation = Invalid;
        break;

      case VTK_UNSTRUCTURED_GRID:
      {
        vtkNew<vtkGeometryFilter> bdy;
        bdy->MergingOff();
        bdy->SetInputDataObject(data);
        bdy->Update();
        entry.m_geometry = bdy->GetOutput();
        ++entry.m_generation;
      }
      break;

      case VTK_POLY_DATA:
      case VTK_IMAGE_DATA:
      default:
        entry.m_geometry = data;
        ++entry.m_generation;
        break;
    }

    if (entry.m_geometry && ent->properties().contains<std::vector<double>>("color"))
    {
      Geometry::addColorArray(entry.m_geometry, ent->properties().at<std::vector<double>>("color"));
    }
    Geometry::addTransformArrayIfPresent(entry.m_geometry, ent);
  }
  else
  {
    entry.m_geometry = nullptr;
    entry.m_generation = Invalid;
  }
}

int Geometry::dimension(const smtk::resource::PersistentObject::Ptr& obj) const
{
  auto ent = std::dynamic_pointer_cast<smtk::model::Entity>(obj);
  if (ent)
  {
    // return ent->dimension();
    auto it = m_cache.find(obj->id());
    bool found = it != m_cache.end();
    if (found && it->second.m_geometry)
    { // Cache is clean.
      return Geometry::dimensionOf(it->second.m_geometry, ent);
    }
    else if (found)
    { // Cache was marked dirty.
      this->queryGeometry(obj, it->second);
      if (!it->second.isValid())
      {
        m_cache.erase(it);
      }
      else
      {
        return Geometry::dimensionOf(it->second.m_geometry, ent);
      }
    }
    else
    { // No cache entry yet; try to add one.
      CacheEntry entry;
      this->queryGeometry(obj, entry);
      if (entry.isValid())
      {
        m_cache[obj->id()] = entry;
        return Geometry::dimensionOf(entry.m_geometry, ent);
      }
    }
  }
  return 0;
}

Geometry::Purpose Geometry::purpose(const smtk::resource::PersistentObject::Ptr& obj) const
{
  auto ent = std::dynamic_pointer_cast<smtk::model::Entity>(obj);
  if (ent)
  {
    return ent->isInstance() ? Geometry::Glyph : Geometry::Surface;
  }
  return Geometry::Surface;
}

void Geometry::update() const
{
  // Do nothing. Operations in the VTK session set content as needed.
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

int Geometry::dimensionOf(const DataType& geom, const smtk::model::EntityPtr& ent)
{
  (void)ent;
  if (!geom)
  {
    return 0;
  }
  vtkPolyData* poly;
  vtkImageData* image;
  vtkUnstructuredGrid* ugrid;
  if ((poly = vtkPolyData::SafeDownCast(geom)))
  {
    if (poly->GetNumberOfStrips() > 0 || poly->GetNumberOfPolys() > 0)
    {
      return 2;
    }
    else if (poly->GetNumberOfLines() > 0)
    {
      return 1;
    }
    return 0;
  }
  else if ((ugrid = vtkUnstructuredGrid::SafeDownCast(geom)))
  {
    return ent->dimension();
  }
  else if ((image = vtkImageData::SafeDownCast(geom)))
  {
    return image->GetDataDimension();
  }
  return 0;
}

} // namespace vtk
} // namespace session
} // namespace smtk
