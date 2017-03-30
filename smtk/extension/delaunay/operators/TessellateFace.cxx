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
#include "smtk/extension/delaunay/operators/TessellateFace.h"

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
namespace model
{

TessellateFace::TessellateFace()
{
}

bool TessellateFace::ableToOperate()
{
  smtk::model::EntityRef eRef = this->specification()->associations()->value();

  return this->Superclass::ableToOperate() && eRef.isValid() && eRef.isFace() &&
    eRef.owningModel().isValid();
}

OperatorResult TessellateFace::operateInternal()
{
  smtk::model::Face face = this->specification()->associations()->value().as<smtk::model::Face>();

  bool validatePolygons = this->findVoid("validate polygons")->isEnabled();

  // get the face use for the face
  smtk::model::FaceUse fu = face.positiveUse();

  // check if we have an exterior loop
  smtk::model::Loops exteriorLoops = fu.loops();
  if (exteriorLoops.size() == 0)
  {
    // if we don't have loops, there is nothing to mesh
    smtkErrorMacro(this->log(), "No loops associated with this face.");
    return this->createResult(OPERATION_FAILED);
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
    return this->createResult(OPERATION_FAILED);
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
      return this->createResult(OPERATION_FAILED);
    }

    excise(p_sub, mesh);
  }

  // Use the delaunay mesh to retessellate the face
  smtk::extension::delaunay::io::ImportDelaunayMesh importFromDelaunayMesh;
  importFromDelaunayMesh(mesh, face);

  OperatorResult result = this->createResult(OPERATION_SUCCEEDED);
  this->addEntityToResult(result, face, MODIFIED);
  result->findModelEntity("tess_changed")->setValue(face);
  return result;
}

} // namespace model
} // namespace smtk

#include "smtk/extension/delaunay/Exports.h"
#include "smtk/extension/delaunay/TessellateFace_xml.h"

smtkImplementsModelOperator(SMTKDELAUNAYEXT_EXPORT, smtk::model::TessellateFace,
  delaunay_tessellate_face, "tessellate face", TessellateFace_xml, smtk::model::Session);
