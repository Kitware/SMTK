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
#include "smtk/extension/delaunay/TriangulateFace.h"

#include "smtk/io/ModelToMesh.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/ExtractTessellation.h"
#include "smtk/mesh/Manager.h"

#include "smtk/model/Edge.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"

#include "Discretization/ConstrainedDelaunayMesh.hh"
#include "Discretization/ExcisePolygon.hh"
#include "Mesh/Mesh.hh"
#include "Shape/Point.hh"
#include "Shape/Polygon.hh"
#include "Shape/PolygonUtilities.hh"

#include <algorithm>

namespace {

std::vector<Delaunay::Shape::Point> pointsInLoop(
  const smtk::model::Loop& loop, smtk::mesh::CollectionPtr& collection)
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
  // loops have a redundant point at the end. We need to remove it.
  points.pop_back();

  return points;
}

template <typename Point, typename PointContainer>
std::size_t IndexOf(Point& point, const PointContainer& points)
{
  return std::distance(points.begin(),
               std::find(points.begin(), points.end(), point));
}

smtk::mesh::HandleRange ImportDelaunayMesh(const Delaunay::Mesh::Mesh& mesh,
                                           smtk::mesh::CollectionPtr collection)
{
  smtk::mesh::InterfacePtr iface = collection->interface();
  smtk::mesh::AllocatorPtr alloc = iface->allocator();
  smtk::mesh::Handle firstVertex = 0;
  std::vector<double*> coordinateMemory;

  if (!alloc->allocatePoints(mesh.GetVertices().size(),
                             firstVertex,
                             coordinateMemory))
  {
    return smtk::mesh::HandleRange();
  }

  std::size_t index = 0;
  for (auto& p : mesh.GetVertices())
  {
    index = IndexOf(p, mesh.GetVertices());
    coordinateMemory[0][index] = p.x;
    coordinateMemory[1][index] = p.y;
    coordinateMemory[2][index] = 0.;
  }

  smtk::mesh::HandleRange createdCellIds;
  smtk::mesh::Handle* connectivity;

  if (!alloc->allocateCells(smtk::mesh::Triangle,
                            mesh.GetTriangles().size(),
                            smtk::mesh::verticesPerCell(smtk::mesh::Triangle),
                            createdCellIds,
                            connectivity))
  {
    return smtk::mesh::HandleRange();
  }

  index = 0;
  for (auto& t : mesh.GetTriangles())
  {
    connectivity[index++] = firstVertex + IndexOf(t.AB().A(), mesh.GetVertices());
    connectivity[index++] = firstVertex + IndexOf(t.AB().B(), mesh.GetVertices());
    connectivity[index++] = firstVertex + IndexOf(t.AC().B(), mesh.GetVertices());
  }

  alloc->connectivityModified(createdCellIds,
                              smtk::mesh::verticesPerCell(smtk::mesh::Triangle),
                              connectivity);

  return createdCellIds;
}

}

//-----------------------------------------------------------------------------
namespace smtk {
  namespace model {

//-----------------------------------------------------------------------------
TriangulateFace::TriangulateFace()
{
}

bool TriangulateFace::ableToOperate()
{
  smtk::model::EntityRef eRef =
    this->specification()->findModelEntity("face")->value();

  return
    this->Superclass::ableToOperate() &&
    eRef.isValid() &&
    eRef.isFace() &&
    eRef.owningModel().isValid();
}

//-----------------------------------------------------------------------------
OperatorResult TriangulateFace::operateInternal()
{
  smtk::model::Face face =
    this->specification()->findModelEntity("face")->
    value().as<smtk::model::Face>();

  smtk::io::ModelToMesh convert;
  convert.setIsMerging(false);
  smtk::mesh::CollectionPtr collection = convert(this->session()->meshManager(),
                                                 this->session()->manager());

  // get the face use for the face
  smtk::model::FaceUse fu = face.positiveUse();

  // check if we have an exterior loop
  smtk::model::Loops exteriorLoops = fu.loops();
  if (exteriorLoops.size() == 0)
  {
    // if we don't have loops we are bailing out!
    smtkErrorMacro(this->log(), "No loops associated with this face.");
    return this->createResult(OPERATION_FAILED);
  }

  // the first loop is the exterior loop
  smtk::model::Loop exteriorLoop = exteriorLoops[0];

  // make a polygon from the points in the loop
  std::vector<Delaunay::Shape::Point> points =
    pointsInLoop(exteriorLoop, collection);
  Delaunay::Shape::Polygon p(points);
  // if the orientation is not ccw, flip the orientation
  if (Delaunay::Shape::Orientation(p) != 1)
  {
    p = Delaunay::Shape::Polygon(points.rbegin(), points.rend());
  }

  // discretize the polygon
  Delaunay::Discretization::ConstrainedDelaunayMesh discretize;
  Delaunay::Mesh::Mesh mesh;
  discretize(p, mesh);

  // then we excise each inner loop within the exterior loop
  Delaunay::Discretization::ExcisePolygon excise;
  for (auto& loop : exteriorLoop.containedLoops())
  {
    std::vector<Delaunay::Shape::Point> points_sub =
      pointsInLoop(loop, collection);
    Delaunay::Shape::Polygon p_sub(points_sub);
    // if the orientation is not ccw, flip the orientation
    if (Delaunay::Shape::Orientation(p_sub) != 1)
    {
      p_sub = Delaunay::Shape::Polygon(points_sub.rbegin(), points_sub.rend());
    }
    excise(p_sub, mesh);
  }

  // remove the original collection and replace it with a new one
  this->session()->meshManager()->removeCollection(collection);
  collection = this->session()->meshManager()->makeCollection();

  // populate the new collection
  smtk::mesh::HandleRange cells = ImportDelaunayMesh(mesh, collection);
  smtk::mesh::Handle meshHandle;
  const bool created = collection->interface()->createMesh(cells, meshHandle);
  if (created)
  {
    smtk::mesh::HandleRange meshHandles;
    meshHandles.insert(meshHandle);
    collection->interface()->setAssociation(face.entity(), meshHandles);
  }

  OperatorResult result = this->createResult(OPERATION_SUCCEEDED);
  this->addEntityToResult(result, face, MODIFIED);
  result->findModelEntity("mesh_created")->setValue(face);
  return result;
}

  } // namespace model
} // namespace smtk

#include "smtk/extension/delaunay/TriangulateFace_xml.h"
#include "smtk/extension/delaunay/Exports.h"

smtkImplementsModelOperator(
  SMTKDELAUNAYEXT_EXPORT,
  smtk::model::TriangulateFace,
  delaunay_triangulate_face,
  "triangulate face",
  TriangulateFace_xml,
  smtk::model::Session);
