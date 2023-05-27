//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/UUID.h"

#include "smtk/io/ModelToMesh.h"
#include "smtk/io/WriteMesh.h"

#include "smtk/io/ImportMesh.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/model/EntityIterator.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Volume.h"
#include "smtk/model/json/jsonResource.h"
#include "smtk/model/json/jsonTessellation.h"

#include "smtk/mesh/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include <fstream>
#include <sstream>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;
std::string write_root = SMTK_SCRATCH_DIR;

void cleanup(const std::string& file_path)
{
  //first verify the file exists
  ::boost::filesystem::path path(file_path);
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }
}

void create_simple_mesh_model(smtk::model::ResourcePtr resource)
{
  std::string file_path(data_root);
  file_path += "/model/2d/smtk/test2D.json";

  std::ifstream file(file_path.c_str());

  std::string json_str((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
  nlohmann::json json = nlohmann::json::parse(json_str);

  smtk::model::from_json(json, resource);
  for (auto& tessPair : json["tessellations"])
  {
    smtk::common::UUID id = tessPair[0];
    smtk::model::Tessellation tess = tessPair[1];
    resource->setTessellation(id, tess);
  }

  resource->assignDefaultNames();

  file.close();
}

smtk::mesh::MeshSet
make_MeshPoint(smtk::mesh::ResourcePtr meshResource, double x, double y, double z)
{
  smtk::mesh::InterfacePtr interface = meshResource->interface();
  smtk::mesh::AllocatorPtr allocator = interface->allocator();

  smtk::mesh::Handle vertexHandle;
  std::vector<double*> coords;
  allocator->allocatePoints(1, vertexHandle, coords);

  coords[0][0] = x;
  coords[1][0] = y;
  coords[2][0] = z;

  smtk::mesh::HandleRange meshCells;
  meshCells.insert(vertexHandle);

  smtk::mesh::CellSet cellsForMesh(meshResource, meshCells);
  smtk::mesh::MeshSet result = meshResource->createMesh(cellsForMesh);

  return result;
}

void verify_simple_merge()
{
  smtk::model::ResourcePtr modelResource = smtk::model::Resource::create();

  create_simple_mesh_model(modelResource);

  smtk::io::ModelToMesh convert;
  convert.setIsMerging(false);
  smtk::mesh::ResourcePtr mr = convert(modelResource);
  test(mr->isValid(), "resouce should be valid");

  //make sure merging points works properly
  smtk::mesh::PointSet points = mr->points();

  test(points.size() == 88, "Should be exactly 88 points in the original mesh");

  mr->meshes().mergeCoincidentContactPoints();

  points = mr->points();
  test(points.size() == 32, "After merging of identical points we should have 32");

  //verify that after merging points we haven't deleted any of the cells
  //that represent a model vert
  smtk::mesh::CellSet vert_cells = mr->cells(smtk::mesh::Dims0);
  test(vert_cells.size() == 7);
}

void verify_complex_merge()
{
  smtk::model::ResourcePtr modelResource = smtk::model::Resource::create();

  create_simple_mesh_model(modelResource);

  smtk::io::ModelToMesh convert;
  convert.setIsMerging(false);
  smtk::mesh::ResourcePtr mr = convert(modelResource);
  test(mr->isValid(), "meshResource should be valid");

  //add multiple new mesh points
  smtk::mesh::MeshSet newMeshPoint1 = make_MeshPoint(mr, 0.0, 2.0, 0.0);
  smtk::mesh::MeshSet newMeshPoint2 = make_MeshPoint(mr, 1.0, 0.0, 0.0);
  smtk::mesh::MeshSet newMeshPoint3 = make_MeshPoint(mr, 3.0, 0.0, 0.0);
  smtk::mesh::MeshSet newMeshPoint4 = make_MeshPoint(mr, 0.0, 2.0, 0.0);

  //make sure merging points works properly
  smtk::mesh::PointSet points = mr->points();
  test(points.size() == 92, "should be 92 points before merge");

  //failing to merge this point into the other points
  mr->meshes().mergeCoincidentContactPoints();

  points = mr->points();
  test(mr->points().size() == 32, "After merging of identical points we should have 32");

  //verify the all the points merged properly
  std::vector<double> p(3);

  newMeshPoint1.points().get(p.data());
  test(p[0] == 0.0);
  test(p[1] == 2.0);
  test(p[2] == 0.0);

  newMeshPoint2.points().get(p.data());
  test(p[0] == 1.0);
  test(p[1] == 0.0);
  test(p[2] == 0.0);

  newMeshPoint3.points().get(p.data());
  test(p[0] == 3.0);
  test(p[1] == 0.0);
  test(p[2] == 0.0);

  newMeshPoint4.points().get(p.data());
  test(p[0] == 0.0);
  test(p[1] == 2.0);
  test(p[2] == 0.0);
}

void verify_write_valid_meshResource_hdf5_after_merge()
{
  smtk::model::ResourcePtr modelResource = smtk::model::Resource::create();

  create_simple_mesh_model(modelResource);

  smtk::io::ModelToMesh convert;
  convert.setIsMerging(false);
  smtk::mesh::ResourcePtr mr = convert(modelResource);
  test(mr->isValid(), "resource should be valid");

  //make sure merging points works properly
  smtk::mesh::PointSet points = mr->points();

  test(points.size() == 88, "Should be exactly 88 points in the original mesh");

  //add multiple new mesh points
  smtk::mesh::MeshSet newMeshPoint1 = make_MeshPoint(mr, 0.0, 2.0, 0.0);
  smtk::mesh::MeshSet newMeshPoint2 = make_MeshPoint(mr, 1.0, 0.0, 0.0);
  smtk::mesh::MeshSet newMeshPoint3 = make_MeshPoint(mr, 3.0, 0.0, 0.0);
  smtk::mesh::MeshSet newMeshPoint4 = make_MeshPoint(mr, 0.0, 2.0, 0.0);

  points = mr->points();
  test(mr->points().size() == 92, "Should be exactly 92 points before merge");
  test(mr->meshes(smtk::mesh::Dims0).size() == 11, "Should have 11 vertices before merge");

  smtk::mesh::CellSet vert_cells = mr->cells(smtk::mesh::Dims0);
  test(vert_cells.size() == 11, "Should have 11 vertex cells before merge");

  mr->meshes().mergeCoincidentContactPoints();

  points = mr->points();
  test(points.size() == 32, "After merging of identical points we should have 32");

  //verify that after merging points we haven't deleted any of the cells
  //that represent a model vert
  test(mr->meshes(smtk::mesh::Dims0).size() == 11, "Should have 11 vertices after merge");

  vert_cells = mr->cells(smtk::mesh::Dims0);
  test(vert_cells.size() == 9, "Should have 9 vertex cells after merge");

  // write out the resource after mergeCoincidentContactPoints()
  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".h5m";

  //write out the mesh.
  bool result = smtk::io::writeMesh(write_path, mr);
  if (!result)
  {
    cleanup(write_path);
    test(result, "failed to properly write out a valid hdf5 resource");
  }

  //reload the written file and verify the number of meshes are the same as the
  //input mesh
  smtk::mesh::ResourcePtr mr1 = smtk::mesh::Resource::create();
  smtk::io::importMesh(write_path, mr1);

  //remove the file from disk
  cleanup(write_path);

  //verify the meshes
  test(mr1->isValid(), "resource should be valid");
  test(mr1->name() == mr->name());
  test(mr1->types() == mr->types());

  test(mr->meshes(smtk::mesh::Dims2).size() == 4, "Should have 4 faces in saved resource");
  test(mr->meshes(smtk::mesh::Dims1).size() == 10, "Should have 10 edges in saved resource");
  test(mr->meshes(smtk::mesh::Dims0).size() == 11, "Should have 11 vertices in saved resource");

  vert_cells = mr->cells(smtk::mesh::Dims0);
  test(vert_cells.size() == 9, "Should have 9 vertex cells in saved resource");

  points = mr1->points();
  test(points.size() == 32, "Should have 32 points in saved resource");
}
} // namespace

int UnitTestMergeContactPoints(int /*unused*/, char** const /*unused*/)
{
  verify_simple_merge();
  verify_complex_merge();
  verify_write_valid_meshResource_hdf5_after_merge();
  return 0;
}
