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
#include "DelaunayMeshWorker.h"

#include "remus/common/LocateFile.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/extension/delaunay/io/ExportDelaunayMesh.h"
#include "smtk/extension/delaunay/io/ImportDelaunayMesh.h"

#include "smtk/extension/delaunay/worker/detail/fromRemus.h"

#include "smtk/model/EntityIterator.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Manager.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/io/SaveJSON.h"

#include "Discretization/ConstrainedDelaunayMesh.hh"
#include "Discretization/ExcisePolygon.hh"
#include "Mesh/Mesh.hh"
#include "Shape/Point.hh"
#include "Shape/Polygon.hh"
#include "Shape/PolygonUtilities.hh"
#include "Validation/IsValidPolygon.hh"

//----------------------------------------------------------------------------
DelaunayMeshWorker::DelaunayMeshWorker(
  remus::worker::ServerConnection const& connection, remus::common::FileHandle const& fhandle)
  : remus::worker::Worker(
      remus::proto::make_JobRequirements(
        remus::common::make_MeshIOType(remus::meshtypes::Model(), remus::meshtypes::Model()),
        std::string("DelaunayMeshWorker"), fhandle, remus::common::ContentFormat::XML),
      connection)
{
  //make this a highly responsive worker, so that if it crashes the UI knows asap
  this->pollingRates(remus::worker::PollingRates(33, 250));
}

//----------------------------------------------------------------------------
void DelaunayMeshWorker::meshJob()
{
  remus::worker::Job j = this->getJob();
  if (!j.valid())
  {
    this->sendJobFailure(j, "Invalid Job Given to Worker");
    return;
  }

  //todo need to handle an xml file from the client with setting information
  //for now just focus on model data
  remus::proto::JobContent modelData;
  remus::proto::JobContent modelUUIDData;
  remus::proto::JobContent meshAttributes;

  //needed keys in the submission
  const bool has_model = j.details("model", modelData);
  const bool has_meshing_controls = j.details("meshing_attributes", meshAttributes);
  const bool has_modelUUIDS = j.details("modelUUIDS", modelUUIDData);

  //both meshing controls and modelUUIDS are optional
  (void)has_meshing_controls;
  (void)has_modelUUIDS;
  if (!has_model)
  {
    this->sendJobFailure(j, "Invalid Job Submission Content");
    return;
  }

  this->sendProgress(j, 0, "Reconstructing smtk::model");

  //convert the json data stream into an smtk model
  detail::Resources resources =
    detail::deserialize_smtk_model(modelData, meshAttributes, modelUUIDData);

  if (!resources.valid())
  {
    this->sendJobFailure(j, "Invalid smtk::model data passed to the Delaunay Worker");
    return;
  }

  bool validatePolygons = false;
  {
    const auto attribute = resources.m_attributes->findAttribute("Globals");
    if (attribute)
    {
      const auto voidItem = attribute->findVoid("validate polygons");
      if (voidItem)
      {
        validatePolygons = voidItem->isEnabled();
      }
    }
  }

  //determine the progress increment per model call
  double steps = 1;
  for (std::size_t i = 0; i < resources.m_modelsToMesh.size(); ++i)
  {
    steps += static_cast<double>(resources.m_modelsToMesh[i].m_faces.size());
  }
  double inc = 100.0 / steps;

  //now we can actually iterate each model and mesh the faces. We need to
  //make sure that each collection only has the faces of a single model, as
  //the association between collection and model is only 1 to 1.
  int index = 0;
  for (std::size_t i = 0; i < resources.m_modelsToMesh.size(); ++i)
  {
    smtk::mesh::CollectionPtr collection = resources.m_mesh->makeCollection();
    const detail::FacesOfModel& fom = resources.m_modelsToMesh[i];

    for (auto it = fom.m_faces.cbegin(); it != fom.m_faces.cend(); ++it, ++index)
    {
      std::stringstream buffer;
      buffer << "Submitting " << *it << " for meshing"
             << "\n";
      this->sendProgress(j, static_cast<int>(index * inc), buffer.str());
      std::cout << "[" << index * inc << "%] " << buffer.str();

      this->meshFace(*it, validatePolygons, collection);
    }

    //determine a location that we can write a temporary file for the heavy
    //mesh data
    const auto tempFile = remus::common::makeTempFileHandle(collection->entity().toString(), "h5m");
    collection->writeLocation(tempFile.path());

    // We need to explicitly associate the collection to a model so that
    // cmb/model builder can properly add this mesh to the UI
    collection->associateToModel(fom.m_model.entity());
  }

  //serialize all the collections into a single string
  std::string meshesToJson =
    smtk::io::SaveJSON::fromModelManager(resources.m_model, smtk::io::JSON_MESHES);

  this->sendProgress(j, 100, "Finished serializing process");
  std::cout << "Finished serializing process" << std::endl;

  remus::proto::JobResult jr(j.id(), //use zero copy constructor
    remus::common::ContentFormat::JSON, meshesToJson.c_str(), meshesToJson.size());
  this->returnResult(jr);
}

//----------------------------------------------------------------------------
bool DelaunayMeshWorker::meshFace(
  const smtk::model::Face& face, bool validatePolygons, const smtk::mesh::CollectionPtr& collection)
{
  //step 1 get the face use for the face
  smtk::model::FaceUse fu_p = face.positiveUse();

  //check if we have loops
  smtk::model::Loops loops = fu_p.loops();
  if (loops.size() == 0)
  {
    // if we don't have loops, there is nothing to mesh
    return false;
  }

  // the first loop is the exterior loop
  smtk::model::Loop exteriorLoop = loops[0];

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
    return false;
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
      return false;
    }

    excise(p_sub, mesh);
  }

  // associate the collection with the face's model
  collection->setModelManager(face.manager());
  collection->associateToModel(face.model().entity());

  // populate the new collection
  smtk::extension::delaunay::io::ImportDelaunayMesh importFromDelaunayMesh;
  smtk::mesh::MeshSet meshSet = importFromDelaunayMesh(mesh, collection);
  if (!meshSet.is_empty())
  {
    collection->setAssociation(face, meshSet);
  }

  return true;
}
