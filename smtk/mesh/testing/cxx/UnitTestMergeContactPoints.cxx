//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/ModelToMesh.h"
#include "smtk/io/ImportJSON.h"
#include "smtk/io/WriteMesh.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Volume.h"
#include "smtk/model/EntityIterator.h"

#include "smtk/model/testing/cxx/helpers.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include <sstream>
#include <fstream>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

//----------------------------------------------------------------------------
void create_simple_mesh_model( smtk::model::ManagerPtr mgr )
{
  std::string file_path(data_root);
  file_path += "/smtk/test2D.json";

  std::ifstream file(file_path.c_str());

  std::string json(
    (std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));

  //we should load in the test2D.json file as an smtk to model
  smtk::io::ImportJSON::intoModelManager(json.c_str(), mgr);
  mgr->assignDefaultNames();

  file.close();
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet make_MeshPoint(smtk::mesh::CollectionPtr collection,
                                   double x, double y, double z)
{
  smtk::mesh::InterfacePtr interface = collection->interface();
  smtk::mesh::AllocatorPtr allocator = interface->allocator();

  smtk::mesh::Handle vertexHandle;
  std::vector<double*> coords;
  allocator->allocatePoints(1, vertexHandle, coords);

  coords[0][0] = x;
  coords[1][0] = y;
  coords[2][0] = z;

  smtk::mesh::HandleRange meshCells;
  meshCells.insert(vertexHandle);

  smtk::mesh::CellSet cellsForMesh(collection, meshCells);
  smtk::mesh::MeshSet result = collection->createMesh(cellsForMesh);

  return result;
}


//----------------------------------------------------------------------------
void verify_simple_merge()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_mesh_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  test( c->isValid(), "collection should be valid");

  //make sure merging points works properly
  smtk::mesh::PointSet points = c->points( );
  test( points.size() == 88, "Should be exactly 88 points in the original mesh");

  c->meshes().mergeCoincidentContactPoints();

  points = c->points( );
  test( points.size() == 32, "After merging of identical points we should have 32");

  //verify that after merging points we haven't deleted any of the cells
  //that represent a model vert
  smtk::mesh::CellSet vert_cells = c->cells( smtk::mesh::Dims0 );
  test( vert_cells.size() == 7 );
}

//----------------------------------------------------------------------------
void verify_complex_merge()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_mesh_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  test( c->isValid(), "collection should be valid");

  //add a new mesh which is a single point very close to 0,2,0
  smtk::mesh::MeshSet newMeshPoint = make_MeshPoint(c, 0.0, 2.0, 0.0 );

  //make sure merging points works properly
  smtk::mesh::PointSet points = c->points( );
  test( points.size() == 89, "should be 89 points before merge");

  //failing to merge this point into the other points
  c->meshes().mergeCoincidentContactPoints();
  test( c->points().size() == 32, "After merging of identical points we should have 32");

  //verify the point is merged properly
  std::vector<double> p(3);
  newMeshPoint.points().get(&p[0]);
  test(p[0] == 0); test(p[1] == 2.0); test(p[2] == 0);
}
}

//----------------------------------------------------------------------------
int UnitTestMergeContactPoints(int, char** const)
{
  // verify_simple_merge();
  verify_complex_merge();
  return 0;
}