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

#include "smtk/extension/delaunay/io/ExportDelaunayMesh.h"

#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/utility/ExtractTessellation.h"

#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Loop.h"

#include "Shape/Point.hh"

namespace smtk
{
namespace extension
{
namespace delaunay
{
namespace io
{

std::vector<Delaunay::Shape::Point> ExportDelaunayMesh::operator()(
  const smtk::model::Loop& loop) const
{
  std::vector<Delaunay::Shape::Point> points;
  // We collect the points for each edge and insert them at once into our
  // points vector. This allows us to properly account for the edge use
  // orientation while using smtk::model::Tessellation's forward iterator to
  // loop through tessellation cells.
  std::vector<Delaunay::Shape::Point> pointsForEdge;

  for (auto& eu : loop.edgeUses())
  {
    const smtk::model::Tessellation* tess = eu.edge().hasTessellation();
    std::vector<int> cell_conn;
    smtk::model::Tessellation::size_type start_off;
    std::vector<std::size_t> numCellsOfType(smtk::mesh::CellType_MAX, 0);
    for (start_off = tess->begin(); start_off != tess->end();
         start_off = tess->nextCellOffset(start_off))
    {
      tess->vertexIdsOfCell(start_off, cell_conn);
      for (std::size_t j = 0; j < cell_conn.size(); j++)
      {
        pointsForEdge.emplace_back(
          tess->coords()[cell_conn[j] * 3 + 0], tess->coords()[cell_conn[j] * 3 + 1]);
      }
      cell_conn.clear();
    }

    // We transplant the edge's points into the global point vector according to
    // the edge use's orientation. We skip the first point, as it is always a
    // duplicate of the last point in the global vector.
    if (eu.orientation() == 1)
    {
      for (auto p = pointsForEdge.begin() + 1; p != pointsForEdge.end(); ++p)
      {
        points.emplace_back(p->x, p->y);
      }
    }
    else
    {
      for (auto p = pointsForEdge.rbegin() + 1; p != pointsForEdge.rend(); ++p)
      {
        points.emplace_back(p->x, p->y);
      }
    }
    pointsForEdge.clear();
  }

  return points;
}

std::vector<Delaunay::Shape::Point> ExportDelaunayMesh::operator()(
  const smtk::model::Loop& loop,
  smtk::mesh::ResourcePtr& resource) const
{
  std::int64_t connectivityLength = -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
    loop, resource, connectivityLength, numberOfCells, numberOfPoints);

  std::vector<std::int64_t> conn(connectivityLength);
  std::vector<float> fpoints(numberOfPoints * 3);

  smtk::mesh::utility::PreAllocatedTessellation ftess(conn.data(), fpoints.data());

  ftess.disableVTKStyleConnectivity(true);
  ftess.disableVTKCellTypes(true);

  smtk::mesh::utility::extractOrderedTessellation(loop, resource, ftess);

  std::vector<Delaunay::Shape::Point> points;

  for (std::size_t i = 0; i < fpoints.size(); i += 3)
  {
    points.emplace_back(fpoints[i], fpoints[i + 1]);
  }

  // Delaunay polygons do not use a repeated point to denote a loop.
  if (points.front() == points.back())
  {
    points.pop_back();
  }

  return points;
}
} // namespace io
} // namespace delaunay
} // namespace extension
} // namespace smtk
