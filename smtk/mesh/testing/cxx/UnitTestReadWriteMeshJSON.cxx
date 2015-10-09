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
#include "smtk/io/WriteMesh.h"
#include "smtk/io/ImportMesh.h"
#include "smtk/io/ExportJSON.h"
#include "smtk/io/ImportJSON.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Volume.h"
#include "smtk/model/EntityIterator.h"

#include "smtk/model/testing/cxx/helpers.h"
#include "smtk/mesh/testing/cxx/helpers.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{
//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;
std::string write_root = data_root + "/mesh/tmp";

void cleanup( const std::string& file_path )
{
  //first verify the file exists
  ::boost::filesystem::path path( file_path );
  if( ::boost::filesystem::is_regular_file( path ) )
    {
    //remove the file_path if it exists.
    ::boost::filesystem::remove( path );
    }
}

//----------------------------------------------------------------------------
void create_simple_model( smtk::model::ManagerPtr mgr )
{
  std::size_t numTetsInModel = 4;
  using namespace smtk::model::testing;

  smtk::model::SessionRef sess = mgr->createSession("native");
  smtk::model::Model model = mgr->addModel();

  for(std::size_t i=0; i < numTetsInModel; ++i)
    {
    smtk::common::UUIDArray uids = createTet(mgr);
    model.addCell( smtk::model::Volume(mgr, uids[21]));
    }
  model.setSession(sess);
  mgr->assignDefaultNames();

}

//----------------------------------------------------------------------------
void verify_writing_and_loading_collection()
{
  std::string write_path(write_root);
  write_path += "/output.h5m";

  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();
  smtk::mesh::ManagerPtr meshManager = modelManager->meshes();

  create_simple_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  smtk::common::UUID cUUID = c->entity();
  smtk::common::UUIDArray associations = c->meshes().modelEntityIds();


  cJSON* top = cJSON_CreateObject();
  c->writeLocation(write_path);
  const bool exportGood = smtk::io::ExportJSON::fromModelManager(top, modelManager);

  test(exportGood == 1, "Failed to export the mesh collections related to the model");

  //now remove the collection
  meshManager->removeCollection(c);
  test(meshManager->collection(cUUID) == NULL);
  c.reset(); //actually remove the collection from memory


  //now import collection from json stream
  const bool importGood = smtk::io::ImportJSON::ofMeshesOfModel(top,modelManager);
  test(importGood == 0, "Failed to import the mesh collections related to the model");

  //before we verify if the write was good, first remove the output file(s)
  cleanup( write_path );

  //verify collection uuid is the same.
  smtk::mesh::CollectionPtr c2 = meshManager->collection(cUUID);
  test(c2 != NULL,
       "Collection UUID can't change when being loaded from JSON");

  test( c2->modelManager()  == modelManager,
        "Collection loaded from JSON should be related to the model Manager");

  smtk::common::UUIDArray associations2 = c2->meshes().modelEntityIds();
  //most likely failing as we are not saving out custom tags?
  test( associations == associations2,
        "associations after loading from JSON should be the same" );

  test( meshManager->collectionsWithAssociations().size() == 1);
}

//----------------------------------------------------------------------------
void verify_writing_and_loading_multiple_collections()
{
  std::string write_path(write_root);
  std::string write_path2(write_root);

  write_path += "/output1.h5m";
  write_path2 += "/output2.h5m";

  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();
  smtk::mesh::ManagerPtr meshManager = modelManager->meshes();

  create_simple_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  smtk::mesh::CollectionPtr c2 = convert(meshManager,modelManager);

  cJSON* top = cJSON_CreateObject();
  c->writeLocation(write_path);
  c2->writeLocation(write_path2);
  const bool exportGood = smtk::io::ExportJSON::fromModelManager(top, modelManager);

  test(exportGood == 1, "Failed to export the mesh collections related to the model");

  //now remove the collection
  meshManager->removeCollection(c);
  meshManager->removeCollection(c2);
  c.reset();
  c2.reset();

  //now import collection from json stream
  const bool importGood = smtk::io::ImportJSON::ofMeshesOfModel(top,modelManager);

  //before we verify if the write was good, first remove the output file(s)
  cleanup( write_path );
  cleanup( write_path2 );

  test(importGood == 0, "Failed to import the mesh collections related to the model");

  test(meshManager->numberOfCollections() == 2, "number of collections incorrect");

  std::vector<smtk::mesh::CollectionPtr> collections =
                            meshManager->collectionsWithAssociations();
  test(collections.size() == 2, "number of collections with associations incorrect");
}

//----------------------------------------------------------------------------
void verify_writing_and_loading_collections_without_file_path()
{
  std::string write_path(write_root);
  write_path += "/output1.h5m";

  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();
  smtk::mesh::ManagerPtr meshManager = modelManager->meshes();

  create_simple_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  smtk::mesh::CollectionPtr c2 = convert(meshManager,modelManager);

  c->name("fileBased");
  c->writeLocation(write_path);

  c2->name("jsonBased");

  cJSON* top = cJSON_CreateObject();

  const bool exportGood = smtk::io::ExportJSON::fromModelManager(top, modelManager);
  test(exportGood == 1, "Failed to export the mesh collections related to the model");

  //now remove the collection
  meshManager->removeCollection(c);
  meshManager->removeCollection(c2);
  c.reset();
  c2.reset();
  test(meshManager->numberOfCollections() == 0, "number of collections incorrect");

  //now import collection from json stream
  const bool importGood = smtk::io::ImportJSON::ofMeshesOfModel(top,modelManager);

  //before we verify if the write was good, first remove the output file(s)
  cleanup( write_path );

  test(importGood == 0, "Failed to import the mesh collections related to the model");

  test(meshManager->numberOfCollections() == 2, "number of collections incorrect");

  std::vector<smtk::mesh::CollectionPtr> collections =
                            meshManager->collectionsWithAssociations();
  test(collections.size() == 2, "number of collections with associations incorrect");

  //next verify that both collections have valid names
  std::vector< std::string > names;
  names.push_back( collections[0]->name() );
  names.push_back( collections[1]->name() );
  std::sort(names.begin(), names.end());

  test( (names[0]==std::string("fileBased")), "Name doesn't match name during export");
  test( (names[1]==std::string("jsonBased")), "Name doesn't match name during export");

}


//----------------------------------------------------------------------------
void verify_writing_of_single_collection_to_disk()
{
  std::string file_path(data_root);
  file_path += "/mesh/twoassm_out.h5m";

  std::string write_path(write_root);
  write_path += "/output.h5m";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = smtk::io::ImportMesh::entireFile(file_path, manager);
  test( c->isValid(), "collection should be valid");

  std::size_t numMeshes = c->numberOfMeshes();
  test( numMeshes!=0, "dataset once loaded should have more than zero meshes");
  test( numMeshes == 53, "dataset once loaded should have 53 meshes");

  //add some fake boundary conditions
  c->meshes( smtk::mesh::Domain(444) ).setDirichlet( smtk::mesh::Dirichlet(2) );
  c->meshes( smtk::mesh::Domain(444) ).setNeumann( smtk::mesh::Neumann(3) );
  c->meshes( smtk::mesh::Domain(446) ).setNeumann( smtk::mesh::Neumann(2) );

  //verify that we can write this out even when we have no model associations
  //by using forSingleCollection
  c->writeLocation( write_path );
  cJSON* top = cJSON_CreateObject();
  const bool exportGood = smtk::io::ExportJSON::forSingleCollection(top, c);

  test(exportGood == 1, "Expected the Export of forSingleCollection to pass");
}

//----------------------------------------------------------------------------
void verify_writing_of_single_collection_to_json()
{
  std::string file_path(data_root);
  file_path += "/mesh/twoassm_out.h5m";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = smtk::io::ImportMesh::entireFile(file_path, manager);
  test( c->isValid(), "collection should be valid");

  std::size_t numMeshes = c->numberOfMeshes();
  test( numMeshes!=0, "dataset once loaded should have more than zero meshes");
  test( numMeshes == 53, "dataset once loaded should have 53 meshes");

  //add some fake boundary conditions
  c->meshes( smtk::mesh::Domain(444) ).setDirichlet( smtk::mesh::Dirichlet(2) );
  c->meshes( smtk::mesh::Domain(444) ).setNeumann( smtk::mesh::Neumann(3) );
  c->meshes( smtk::mesh::Domain(446) ).setNeumann( smtk::mesh::Neumann(2) );

  cJSON* top = cJSON_CreateObject();
  const bool exportGood = smtk::io::ExportJSON::forSingleCollection(top, c);

  test(exportGood == 1, "Expected the Export of forSingleCollection to pass");
}

//----------------------------------------------------------------------------
void verify_reading_of_single_collection_from_json()
{
  std::string file_path(data_root);
  file_path += "/mesh/twoassm_out.h5m";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  cJSON* top = cJSON_CreateObject();

  {
  smtk::mesh::CollectionPtr c = smtk::io::ImportMesh::entireFile(file_path, manager);
  test( c->isValid(), "collection should be valid");

  std::size_t numMeshes = c->numberOfMeshes();
  test( numMeshes != 0, "dataset once loaded should have more than zero meshes");
  test( numMeshes == 53, "dataset once loaded should have 53 meshes");

  //add some fake boundary conditions
  c->meshes( smtk::mesh::Domain(444) ).setDirichlet( smtk::mesh::Dirichlet(2) );
  c->meshes( smtk::mesh::Domain(444) ).setNeumann( smtk::mesh::Neumann(3) );
  c->meshes( smtk::mesh::Domain(446) ).setNeumann( smtk::mesh::Neumann(2) );

  const bool exportGood = smtk::io::ExportJSON::forSingleCollection(top, c);

  test(exportGood == 1, "Expected the Export of forSingleCollection to pass");

  manager->removeCollection(c);
  }

  //now import collection from json stream
  {
  //get the first child node which is a collection
  cJSON* collection = top->child;
  smtk::mesh::CollectionPtr c = smtk::io::ImportMesh::entireJSON(collection, manager);

  std::size_t numMeshes = c->numberOfMeshes();
  test( numMeshes != 0, "dataset once loaded should have more than zero meshes");
  test( numMeshes == 53, "dataset once loaded should have 53 meshes");

  //verify domains work
  smtk::mesh::MeshSet domain = c->meshes( smtk::mesh::Domain(444) );
  test( domain.size() == 1, "wrong number of domains loaded from json");

  //verify boundary condtions
  smtk::mesh::MeshSet dMeshes = c->meshes( smtk::mesh::Dirichlet(2) );
  smtk::mesh::MeshSet nMeshes = c->meshes( smtk::mesh::Neumann(2) );
  nMeshes.append( c->meshes( smtk::mesh::Neumann(3) ) );

  //verify not empty
  test( dMeshes.is_empty() == false, "wrong number of Dirichlet loaded from json");
  test( nMeshes.is_empty() == false, "wrong number of Neumann loaded from json");

  //verify correct size
  test( dMeshes.size() == 1, "wrong number of dirichlet sets");
  test( nMeshes.size() == 2, "wrong number of neumann sets");
  }

}

//----------------------------------------------------------------------------
void verify_loading_existing_collection_fails()
{
  std::string write_path(write_root);
  write_path += "/output.h5m";

  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();
  smtk::mesh::ManagerPtr meshManager = modelManager->meshes();

  create_simple_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  smtk::common::UUID cUUID = c->entity();

  cJSON* top = cJSON_CreateObject();
  c->writeLocation(write_path);
  const bool exportGood = smtk::io::ExportJSON::fromModelManager(top, modelManager);

  test(exportGood == 1, "Failed to export the mesh collections related to the model");

  const std::size_t numberOfCollections = meshManager->numberOfCollections();
  //now import collection from json stream, should fail as the collection
  //hasn't been removed
  const bool importGood = smtk::io::ImportJSON::ofMeshesOfModel(top,modelManager);

  //before we verify if the write was good, first remove the output file(s)
  cleanup( write_path );

  test(importGood == 0, "Import of mesh was supposed to fail");
  test(numberOfCollections == meshManager->numberOfCollections(),
       "Importing existing collections should not change the number of collections");
}

}

//----------------------------------------------------------------------------
int UnitTestReadWriteMeshJSON(int, char**)
{
  verify_writing_and_loading_collection();
  verify_writing_and_loading_multiple_collections();
  verify_writing_and_loading_collections_without_file_path();

  verify_writing_of_single_collection_to_disk();
  verify_writing_of_single_collection_to_json();

  verify_reading_of_single_collection_from_json();

  verify_loading_existing_collection_fails();

  return 0;
}
