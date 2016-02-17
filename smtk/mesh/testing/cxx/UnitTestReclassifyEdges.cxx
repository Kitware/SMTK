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
void create_simple_2d_model( smtk::model::ManagerPtr mgr )
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
void verify_empty_mesh()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_2d_model(modelManager);

  smtk::mesh::CollectionPtr c = meshManager->makeCollection();

  //lets grab all

  // bool splitOccured = smtk::mesh::split(c,);
  // test(!splitOccured, "split can't occur with an invalid collection");

  // bool mergeOccured = smtk::mesh::merge(c,);
  // test(!mergeOccured, "merge can't occur with an invalid collection");
}

//----------------------------------------------------------------------------
void verify_invalid_model_elements()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_2d_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);


}


//----------------------------------------------------------------------------
void verify_split()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_2d_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  test( c->isValid(), "collection should be valid");
  test( c->numberOfMeshes() == 21, "collection should have 21 mesh elements");

  std::cout << "Entity lookup via reverse classification\n";
  smtk::model::EntityRefArray ents = c->meshes().modelEntities();
  for (smtk::model::EntityRefArray::iterator eit = ents.begin(); eit != ents.end(); ++eit)
    {
    std::cout << "  " << eit->entity().toString() << " (" << eit->flagSummary(0) << ")\n";
    }
}

//----------------------------------------------------------------------------
void verify_merge()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_2d_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  test( c->isValid(), "collection should be valid");
  test( c->numberOfMeshes() == 21, "collection should have 21 mesh elements");


}

}

//----------------------------------------------------------------------------
int UnitTestReclassifyEdges(int, char** const)
{
  verify_empty_mesh();
  verify_invalid_model_elements();
  verify_split();
  verify_merge();

  return 0;
}
