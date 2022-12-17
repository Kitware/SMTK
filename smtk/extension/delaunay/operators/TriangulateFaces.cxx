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

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/extension/delaunay/io/ExportDelaunayMesh.h"
#include "smtk/extension/delaunay/io/ImportDelaunayMesh.h"

#include "smtk/mesh/core/Resource.h"

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

#include "smtk/extension/delaunay/operators/TriangulateFaces_xml.h"

#include <algorithm>

namespace smtk
{
namespace extension
{
namespace delaunay
{

bool TriangulateFaces::ableToOperate()
{
  auto associations = this->parameters()->associations();
  auto entities =
    associations->as<smtk::model::EntityRefArray>([](smtk::resource::PersistentObjectPtr obj) {
      return smtk::model::EntityRef(std::dynamic_pointer_cast<smtk::model::Entity>(obj));
    });

  for (auto& eRef : entities)
  {
    if (!eRef.isValid() || !eRef.isFace() || !eRef.owningModel().isValid())
    {
      return false;
    }
  }

  return smtk::operation::Operation::ableToOperate();
}

TriangulateFaces::Result TriangulateFaces::operateInternal()
{
  auto associations = this->parameters()->associations();
  auto faces = associations->as<smtk::model::Faces>([](smtk::resource::PersistentObjectPtr obj) {
    return smtk::model::Face(std::dynamic_pointer_cast<smtk::model::Entity>(obj));
  });

  bool validatePolygons = this->parameters()->findVoid("validate polygons")->isEnabled();

  // construct a meshresource and associate it with the face's model
  smtk::model::Resource::Ptr resource =
    std::dynamic_pointer_cast<smtk::model::Resource>(faces[0].component()->resource());
  smtk::mesh::ResourcePtr meshresource = smtk::mesh::Resource::create();
  meshresource->setModelResource(faces[0].resource());
  meshresource->associateToModel(faces[0].model().entity());

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  for (auto& face : faces)
  {
    // get the face use for the face
    smtk::model::FaceUse fu = face.positiveUse();

    // check if we have an exterior loop
    smtk::model::Loops exteriorLoops = fu.loops();
    if (exteriorLoops.empty())
    {
      // if we don't have loops, there is nothing to mesh
      smtkErrorMacro(this->log(), "No loops associated with this face.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
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
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
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
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }

      excise(p_sub, mesh);
    }

    // populate the meshresource
    smtk::extension::delaunay::io::ImportDelaunayMesh importFromDelaunayMesh;
    smtk::mesh::MeshSet meshSet = importFromDelaunayMesh(mesh, meshresource);
    if (!meshSet.is_empty())
    {
      meshresource->setAssociation(face, meshSet);
      meshSet.setName(face.name());
    }
    meshSet.mergeCoincidentContactPoints();

    smtk::attribute::ResourceItem::Ptr meshresourceItem = result->findResource("meshresource");
    meshresourceItem->setValue(std::static_pointer_cast<smtk::resource::Resource>(meshresource));

    // we flag the model that owns this face as modified so that a mesh
    // meshresource for the entire model is placed in ModelBuilder's model
    // tree. In the future, ModelBuilder should be able to handle meshes
    // on model entities (rather than entire models).
    smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");
    modified->setValue(face.component());
    result->findComponent("mesh_created")->appendValue(face.owningModel().component());
  }

  return result;
}

const char* TriangulateFaces::xmlDescription() const
{
  return TriangulateFaces_xml;
}
} // namespace delaunay
} // namespace extension
} // namespace smtk
