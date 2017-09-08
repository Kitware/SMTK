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
#include "smtk/extension/delaunay/operators/TriangulateFaces.h"

#include "smtk/attribute/VoidItem.h"

#include "smtk/extension/delaunay/io/ExportDelaunayMesh.h"
#include "smtk/extension/delaunay/io/ImportDelaunayMesh.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"

#include "Discretization/ConstrainedDelaunayMesh.hh"
#include "Discretization/ExcisePolygon.hh"
#include "Mesh/Mesh.hh"
#include "Shape/Point.hh"
#include "Shape/Polygon.hh"
#include "Shape/PolygonUtilities.hh"
#include "Validation/IsValidPolygon.hh"

#include <algorithm>

namespace smtk
{
namespace mesh
{

TriangulateFaces::TriangulateFaces()
{
}

bool TriangulateFaces::ableToOperate()
{
  smtk::model::EntityRefArray entities = this->associatedEntitiesAs<smtk::model::EntityRefArray>();

  for (auto& eRef : entities)
  {
    if (!eRef.isValid() || !eRef.isFace() || !eRef.owningModel().isValid())
    {
      return false;
    }
  }

  return this->Superclass::ableToOperate();
}

smtk::model::OperatorResult TriangulateFaces::operateInternal()
{
  smtk::model::Faces faces = this->associatedEntitiesAs<smtk::model::Faces>();
  bool validatePolygons = this->findVoid("validate polygons")->isEnabled();

  // construct a collection and associate it with the face's model
  smtk::mesh::CollectionPtr collection = this->session()->meshManager()->makeCollection();
  collection->assignUniqueNameIfNotAlready();
  collection->setModelManager(faces[0].manager());
  collection->associateToModel(faces[0].model().entity());

  smtk::model::OperatorResult result = this->createResult(smtk::model::OPERATION_SUCCEEDED);

  for (auto& face : faces)
  {
    // get the face use for the face
    smtk::model::FaceUse fu = face.positiveUse();

    // check if we have an exterior loop
    smtk::model::Loops exteriorLoops = fu.loops();
    if (exteriorLoops.size() == 0)
    {
      // if we don't have loops, there is nothing to mesh
      smtkErrorMacro(this->log(), "No loops associated with this face.");
      return this->createResult(smtk::model::OPERATION_FAILED);
    }

    // the first loop is the exterior loop
    smtk::model::Loop exteriorLoop = exteriorLoops[0];

    // make a polygon from the points in the loop
    smtk::extension::delaunay::io::ExportDelaunayMesh exportToDelaunayMesh;
    std::vector<Delaunay::Shape::Point> points = exportToDelaunayMesh(exteriorLoop);

    // make a polygon validator
    Delaunay::Validation::IsValidPolygon isValidPolygon;

    Delaunay::Shape::Polygon p(points);
    // if the orientation is not ccw, flip the orientation
    if (Delaunay::Shape::Orientation(p) != 1)
    {
      p = Delaunay::Shape::Polygon(points.rbegin(), points.rend());
    }

    if (validatePolygons && !isValidPolygon(p))
    {
      // the polygon is invalid, so we exit with failure
      smtkErrorMacro(this->log(), "Outer boundary polygon is invalid.");
      return this->createResult(smtk::model::OPERATION_FAILED);
    }

    // discretize the polygon
    Delaunay::Discretization::ConstrainedDelaunayMesh discretize;
    Delaunay::Mesh::Mesh mesh;
    discretize(p, mesh);

    // then we excise each inner loop within the exterior loop
    Delaunay::Discretization::ExcisePolygon excise;
    for (auto& loop : exteriorLoop.containedLoops())
    {
      std::vector<Delaunay::Shape::Point> points_sub = exportToDelaunayMesh(loop);
      Delaunay::Shape::Polygon p_sub(points_sub);
      // if the orientation is not ccw, flip the orientation
      if (Delaunay::Shape::Orientation(p_sub) != 1)
      {
        p_sub = Delaunay::Shape::Polygon(points_sub.rbegin(), points_sub.rend());
      }

      if (validatePolygons && !isValidPolygon(p_sub))
      {
        // the polygon is invalid, so we exit with failure
        smtkErrorMacro(this->log(), "Inner boundary polygon is invalid.");
        return this->createResult(smtk::model::OPERATION_FAILED);
      }

      excise(p_sub, mesh);
    }

    // populate the collection
    smtk::extension::delaunay::io::ImportDelaunayMesh importFromDelaunayMesh;
    smtk::mesh::MeshSet meshSet = importFromDelaunayMesh(mesh, collection);
    if (!meshSet.is_empty())
    {
      collection->setAssociation(face, meshSet);
    }
    meshSet.mergeCoincidentContactPoints();

    // we flag the model that owns this face as modified so that a mesh
    // collection for the entire model is placed in ModelBuilder's model
    // tree. In the future, ModelBuilder should be able to handle meshes
    // on model entities (rather than entire models).
    this->addEntityToResult(result, face.owningModel(), MODIFIED);
    result->findModelEntity("mesh_created")->appendValue(face.owningModel());
  }

  return result;
}

} // namespace model
} // namespace smtk

#include "smtk/extension/delaunay/Exports.h"
#include "smtk/extension/delaunay/TriangulateFaces_xml.h"

smtkImplementsModelOperator(SMTKDELAUNAYEXT_EXPORT, smtk::mesh::TriangulateFaces,
  delaunay_triangulate_faces, "triangulate faces", TriangulateFaces_xml, smtk::model::Session);
