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
std::string write_root = data_root + "/mesh/tmp";

//----------------------------------------------------------------------------
void create_simple_model( smtk::model::ManagerPtr mgr )
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
void verify_null_managers()
{
  smtk::mesh::ManagerPtr null_meshManager;
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();

  smtk::model::ManagerPtr null_modelManager;
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  smtk::io::ModelToMesh convert;

  {
  smtk::mesh::CollectionPtr c = convert(null_meshManager, null_modelManager);
  test( !c, "collection should be invalid for a NULL managers");
  }

  {
  smtk::mesh::CollectionPtr c = convert(null_meshManager, modelManager);
  test( !c, "collection should be invalid for a NULL mesh manager");
  }

  {
  smtk::mesh::CollectionPtr c = convert(meshManager, null_modelManager);
  test( !c, "collection should be invalid for a NULL model manager");
  }
}

//----------------------------------------------------------------------------
void verify_empty_model()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  test( !c, "collection should be invalid for an empty model");
}

//----------------------------------------------------------------------------
void verify_model_association()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);

  //we need to verify that the collection is now has an associated model
  test( c->hasAssociations(), "collection should have associations");
  test( (c->associatedModel() != smtk::common::UUID()), "collection should be associated to a real model");
  test( (c->isAssociatedToModel()), "collection should be associated to a real model");

  //verify the MODEL_ENTITY is correct
  smtk::model::EntityRefs currentModels = modelManager->entitiesMatchingFlagsAs<
                                            smtk::model::EntityRefs>( smtk::model::MODEL_ENTITY);
  if(currentModels.size() > 0)
    { //presuming only a single model in the model manager
    test( (c->associatedModel() == currentModels.begin()->entity()), "collection associated model should match model manager");
    }
}

//----------------------------------------------------------------------------
void verify_cell_conversion()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  test( c->isValid(), "collection should be valid");
  test( c->numberOfMeshes() == 21, "collection should have a mesh per tet");

  //confirm that we have the proper number of volume cells
  smtk::mesh::CellSet tri_cells = c->cells( smtk::mesh::Dims2 );
  test( tri_cells.size() == 45 );

  smtk::mesh::CellSet edge_cells = c->cells( smtk::mesh::Dims1 );
  test( edge_cells.size() == 32 );

  smtk::mesh::CellSet vert_cells = c->cells( smtk::mesh::Dims0 );
  test( vert_cells.size() == 8 );

}

//----------------------------------------------------------------------------
void verify_vertex_conversion()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  test( c->isValid(), "collection should be valid");
  test( c->numberOfMeshes() == 21, "collection should have a mesh per tet");

  //make sure merging points works properly
  smtk::mesh::PointSet points = c->points( );
  test( points.size() == 88, "Should be exactly 88 points in the original mesh");

  c->meshes().mergeCoincidentContactPoints();

  points = c->points( );
  test( points.size() == 32, "After merging of identical points we should have 32");
}

}

//----------------------------------------------------------------------------
int UnitTestModelToMesh2D(int, char** const)
{
  verify_null_managers();
  verify_empty_model();
  verify_model_association();
  verify_cell_conversion();
  verify_vertex_conversion();
  return 0;
}
