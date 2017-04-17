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

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/ExtractTessellation.h"
#include "smtk/mesh/MeshSet.h"

#include "Shape/Point.hh"

namespace smtk {
namespace extension {
namespace delaunay {
namespace io {

std::vector<Delaunay::Shape::Point> ExportDelaunayMesh::operator()
  (const smtk::model::Loop& loop, smtk::mesh::CollectionPtr& collection) const
{
  std::int64_t connectivityLength= -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(
    loop, collection, connectivityLength, numberOfCells, numberOfPoints);

  std::vector<std::int64_t> conn( connectivityLength );
  std::vector<float> fpoints(numberOfPoints * 3);

  smtk::mesh::PreAllocatedTessellation ftess(&conn[0], &fpoints[0]);

  ftess.disableVTKStyleConnectivity(true);
  ftess.disableVTKCellTypes(true);

  smtk::mesh::extractOrderedTessellation(loop, collection, ftess);

  std::vector<Delaunay::Shape::Point> points;

  for (std::size_t i=0;i<fpoints.size(); i+=3)
  {
    points.push_back(Delaunay::Shape::Point(fpoints[i], fpoints[i+1]));
  }
  // loops sometimes have a redundant point at the end. We need to remove it.
  if (points.front() == points.back())
  {
    points.pop_back();
  }

  return points;
}

}
}
}
}
