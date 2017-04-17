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
#include "smtk/extension/delaunay/operators/TriangulateFace.h"

#include "smtk/extension/delaunay/io/ImportDelaunayMesh.h"
#include "smtk/extension/delaunay/io/ExportDelaunayMesh.h"

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

namespace smtk {
  namespace model {

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
  smtk::extension::delaunay::io::ExportDelaunayMesh exportToDelaunayMesh;
  std::vector<Delaunay::Shape::Point> points =
    exportToDelaunayMesh(exteriorLoop, collection);
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
      exportToDelaunayMesh(loop, collection);
    Delaunay::Shape::Polygon p_sub(points_sub);
    // if the orientation is not ccw, flip the orientation
    if (Delaunay::Shape::Orientation(p_sub) != 1)
    {
      p_sub = Delaunay::Shape::Polygon(points_sub.rbegin(), points_sub.rend());
    }
    excise(p_sub, mesh);
  }

  // remove the original collection and grab the collection with the same id as
  // the model
  this->session()->meshManager()->removeCollection(collection);
  collection = this->session()->meshManager()->collection(
    face.model().entity());
  if (!collection)
    {
      // If we can't find this collection, we can create it
      collection = this->session()->meshManager()->makeCollection(
        face.model().entity());
      collection->associateToModel(face.model().entity());
    }

  // populate the new collection
  smtk::extension::delaunay::io::ImportDelaunayMesh importFromDelaunayMesh;
  smtk::mesh::MeshSet meshSet = importFromDelaunayMesh(mesh, collection);
  if (!meshSet.is_empty())
  {
    collection->setAssociation(face, meshSet);
  }

  OperatorResult result = this->createResult(OPERATION_SUCCEEDED);
  this->addEntityToResult(result, face, MODIFIED);
  result->findModelEntity("mesh_created")->setValue(face);
  return result;
}

  } // namespace model
} // namespace smtk

#include "smtk/extension/delaunay/Exports.h"
#include "smtk/extension/delaunay/TriangulateFace_xml.h"

smtkImplementsModelOperator(
  SMTKDELAUNAYEXT_EXPORT,
  smtk::model::TriangulateFace,
  delaunay_triangulate_face,
  "triangulate face",
  TriangulateFace_xml,
  smtk::model::Session);
