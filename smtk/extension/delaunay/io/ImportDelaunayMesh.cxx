//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#include "smtk/extension/delaunay/io/ImportDelaunayMesh.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/MeshSet.h"

#include "Mesh/Mesh.hh"
#include "Shape/Point.hh"
#include "Shape/Polygon.hh"
#include "Shape/PolygonUtilities.hh"

namespace smtk
{
namespace extension
{
namespace delaunay
{
namespace io
{

namespace
{

template <typename Point, typename PointContainer>
std::size_t IndexOf(Point& point, const PointContainer& points)
{
  return std::distance(points.begin(), std::find(points.begin(), points.end(), point));
}
}

smtk::mesh::MeshSet ImportDelaunayMesh::operator()(
  const Delaunay::Mesh::Mesh& mesh, smtk::mesh::CollectionPtr collection) const
{
  smtk::mesh::InterfacePtr iface = collection->interface();
  smtk::mesh::AllocatorPtr alloc = iface->allocator();
  smtk::mesh::Handle firstVertex = 0;
  std::vector<double*> coordinateMemory;

  if (!alloc->allocatePoints(mesh.GetVertices().size(), firstVertex, coordinateMemory))
  {
    return collection->createMesh(smtk::mesh::CellSet(collection, smtk::mesh::HandleRange()));
  }

  std::size_t idx = 0;
  for (auto& p : mesh.GetVertices())
  {
    idx = IndexOf(p, mesh.GetVertices());
    coordinateMemory[0][idx] = p.x;
    coordinateMemory[1][idx] = p.y;
    coordinateMemory[2][idx] = 0.;
  }

  smtk::mesh::HandleRange createdCellIds;
  smtk::mesh::Handle* connectivity;

  if (!alloc->allocateCells(smtk::mesh::Triangle, mesh.GetTriangles().size(),
        smtk::mesh::verticesPerCell(smtk::mesh::Triangle), createdCellIds, connectivity))
  {
    return collection->createMesh(smtk::mesh::CellSet(collection, smtk::mesh::HandleRange()));
  }

  idx = 0;
  for (auto& t : mesh.GetTriangles())
  {
    connectivity[idx++] = firstVertex + IndexOf(t.AB().A(), mesh.GetVertices());
    connectivity[idx++] = firstVertex + IndexOf(t.AB().B(), mesh.GetVertices());
    connectivity[idx++] = firstVertex + IndexOf(t.AC().B(), mesh.GetVertices());
  }

  alloc->connectivityModified(
    createdCellIds, smtk::mesh::verticesPerCell(smtk::mesh::Triangle), connectivity);

  return collection->createMesh(smtk::mesh::CellSet(collection, createdCellIds));
}

bool ImportDelaunayMesh::operator()(
  const Delaunay::Mesh::Mesh& mesh, smtk::model::EntityRef& eRef) const
{
  if (!eRef.isValid() || !eRef.isFace())
  {
    return false;
  }

  smtk::model::Tessellation* tess = eRef.resetTessellation();

  tess->coords().resize(mesh.GetVertices().size() * 3);
  std::size_t index = 0;
  double xyz[3];
  for (auto& p : mesh.GetVertices())
  {
    index = IndexOf(p, mesh.GetVertices());
    xyz[0] = p.x;
    xyz[1] = p.y;
    xyz[2] = 0.;
    tess->setPoint(static_cast<int>(index), xyz);
  }

  for (auto& t : mesh.GetTriangles())
  {
    tess->addTriangle(static_cast<int>(IndexOf(t.AB().A(), mesh.GetVertices())),
      static_cast<int>(IndexOf(t.AB().B(), mesh.GetVertices())),
      static_cast<int>(IndexOf(t.AC().B(), mesh.GetVertices())));
  }

  auto bounds = Delaunay::Shape::Bounds(mesh.GetPerimeter());
  const double bbox[6] = { bounds[0], bounds[1], bounds[2], bounds[3], 0., 0. };
  eRef.setBoundingBox(bbox);

  return true;
}
}
}
}
}
