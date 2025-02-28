//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/mesh/vtk/Geometry.h"

#include "smtk/session/mesh/Resource.h"

#include "smtk/extension/vtk/io/mesh/ExportVTKData.h"
#include "smtk/extension/vtk/model/vtkAuxiliaryGeometryExtension.h"

#include "smtk/geometry/Generator.h"

#include <vtkCompositeDataSet.h>
#include <vtkPolyData.h>

namespace smtk
{
namespace session
{
namespace mesh
{
namespace vtk
{

Geometry::Geometry(const std::shared_ptr<smtk::session::mesh::Resource>& parent)
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
  // Do not draw the model (its tessellation contains all meshes in the resource)
  auto ent = std::dynamic_pointer_cast<smtk::model::Entity>(obj);
  if (!ent || ent->isModel())
  {
    entry.m_generation = Invalid;
    return;
  }

  // Access the component's resource
  auto resource = m_parent.lock();
  if (resource == nullptr)
  {
    entry.m_generation = Invalid;
    return;
  }

  // Access the model resource's associated mesh resource
  smtk::mesh::Resource::Ptr meshResource = resource->resource();

  // Access the model resource's associated topology
  smtk::session::mesh::Topology* topology = resource->session()->topology(resource);
  if (topology == nullptr)
  {
    entry.m_generation = Invalid;
    return;
  }

  // Access the component's associated topology element.
  auto elementIt = topology->m_elements.find(obj->id());
  if (elementIt == topology->m_elements.end())
  {
    // Auxiliary geometry is handled outside of the map used for other
    // entity types. For instance, it may be procedural or parametric.
    if (ent->isAuxiliaryGeometry())
    {
      vtkSmartPointer<vtkDataObject> cgeom;
      smtk::model::AuxiliaryGeometry aux(ent);
      smtk::common::Extension::visit<vtkAuxiliaryGeometryExtension::Ptr>(
        [&cgeom, &aux](const std::string& /*unused*/, vtkAuxiliaryGeometryExtension::Ptr ext)
          -> std::pair<bool, bool> {
          std::vector<double> bbox;
          if (ext->canHandleAuxiliaryGeometry(aux, bbox))
          {
            cgeom = vtkAuxiliaryGeometryExtension::fetchCachedGeometry(aux);
            return std::make_pair(true, true);
          }
          return std::make_pair(false, false);
        });
      if (cgeom)
      {
        entry.m_geometry = cgeom;
        ++entry.m_generation;
        // If the object has color properties, apply them
        if (obj->properties().contains<std::vector<double>>("color"))
        {
          Geometry::addColorArray(
            entry.m_geometry, obj->properties().at<std::vector<double>>("color"));
        }
        return;
      }
    }
    entry.m_generation = Invalid;
    return;
  }
  Topology::Element& element = elementIt->second;

  // Access the element's associated meshset
  smtk::mesh::MeshSet meshset = element.m_mesh;

  // Convert the meshest into a vtkPolyData
  smtk::extension::vtk::io::mesh::ExportVTKData exportVTKData;
  entry.m_geometry = vtkSmartPointer<vtkPolyData>::New();
  exportVTKData(meshset, vtkPolyData::SafeDownCast(entry.m_geometry));
  ++entry.m_generation;

  // If the object has color properties, apply them
  if (obj->properties().contains<std::vector<double>>("color"))
  {
    Geometry::addColorArray(entry.m_geometry, obj->properties().at<std::vector<double>>("color"));
  }
}

int Geometry::dimension(const smtk::resource::PersistentObject::Ptr& obj) const
{
  auto ent = std::dynamic_pointer_cast<smtk::model::Entity>(obj);
  if (ent)
  {
    // While the geometry may represent a 3-dimensional volume, the geometric
    // representation is always a vtkPolyData, which has highest dimension 2.
    // Unknown (-1) (like auxiliary geometry) also gets dimension 2.
    return (ent->dimension() >= 0 && ent->dimension() <= 2 ? ent->dimension() : 2);
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
  // Do nothing. Mesh session operations set content as needed.
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
} // namespace vtk
} // namespace mesh
} // namespace session
} // namespace smtk
