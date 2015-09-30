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

  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  smtk::common::UUID cUUID = c->entity();
  smtk::common::UUIDArray associations = c->meshes().modelEntityIds();

  cJSON* top = cJSON_CreateObject();
  const bool exportGood = smtk::io::ExportJSON::forMeshesOfModel(top,
                                                                 modelManager,
                                                                 meshManager,
                                                                 write_path);
  //ExportJson uses C style error codes
  test(exportGood == 0, "Failed to export the mesh collections related to the model");

  //now remove the collection
  meshManager->removeCollection(c);
  test(meshManager->collection(cUUID) == NULL);
  c.reset(); //actually remove the collection from memory


  //now import collection from json stream
  const bool importGood = smtk::io::ImportJSON::ofMeshesOfModel(top,
                                                                modelManager,
                                                                meshManager);
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
  write_path += "/output.h5m";

  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  smtk::mesh::CollectionPtr c2 = convert(meshManager,modelManager);

  cJSON* top = cJSON_CreateObject();
  const bool exportGood = smtk::io::ExportJSON::forMeshesOfModel(top,
                                                                 modelManager,
                                                                 meshManager,
                                                                 write_path);
  //ExportJson uses C style error codes
  test(exportGood == 0, "Failed to export the mesh collections related to the model");

  //now remove the collection
  meshManager->removeCollection(c);
  meshManager->removeCollection(c2);
  c.reset();
  c2.reset();

  //now import collection from json stream
  const bool importGood = smtk::io::ImportJSON::ofMeshesOfModel(top,
                                                                modelManager,
                                                                meshManager);

  //before we verify if the write was good, first remove the output file(s)
  cleanup( write_path );

  test(importGood == 0, "Failed to import the mesh collections related to the model");

  test(meshManager->numberOfCollections() == 2);

  std::vector<smtk::mesh::CollectionPtr> collections =
                            meshManager->collectionsWithAssociations();
  test(collections.size() == 2);
}

//----------------------------------------------------------------------------
void verify_writing_collection_with_no_association_fails()
{
  std::string write_path(write_root);
  write_path += "/output.h5m";

  std::string file_path(data_root);
  file_path += "/mesh/twoassm_out.h5m";

  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  smtk::io::ImportMesh::entireFile(file_path, meshManager);

  cJSON* top = cJSON_CreateObject();
  const bool exportGood = smtk::io::ExportJSON::forMeshesOfModel(top,
                                                                 modelManager,
                                                                 meshManager,
                                                                 write_path);

  //before we verify if the write was good, first remove the output file(s)
  cleanup( write_path );

  //ExportJson uses C style error codes
  test(exportGood == 1, "Expected the Export of MeshesOfModel to fail");
}

//----------------------------------------------------------------------------
void verify_writing_of_single_collection()
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
  cJSON* top = cJSON_CreateObject();
  const bool exportGood = smtk::io::ExportJSON::forSingleCollection(top,
                                                                    c,
                                                                    write_path);

  test(exportGood == 0, "Expected the Export of forSingleCollection to pass");
}

//----------------------------------------------------------------------------
void verify_loading_existing_collection_fails()
{
  std::string write_path(write_root);
  write_path += "/output.h5m";

  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  smtk::common::UUID cUUID = c->entity();

  cJSON* top = cJSON_CreateObject();
  const bool exportGood = smtk::io::ExportJSON::forMeshesOfModel(top,
                                                                 modelManager,
                                                                 meshManager,
                                                                 write_path);
  //ExportJson uses C style error codes
  test(exportGood == 0, "Failed to export the mesh collections related to the model");

  const std::size_t numberOfCollections = meshManager->numberOfCollections();
  //now import collection from json stream, should fail as the collection
  //hasn't been removed
  const bool importGood = smtk::io::ImportJSON::ofMeshesOfModel(top,
                                                                modelManager,
                                                                meshManager);

  //before we verify if the write was good, first remove the output file(s)
  cleanup( write_path );

  test(importGood == 0);
  test(numberOfCollections == meshManager->numberOfCollections(),
       "Importing existing collections should not change the number of collections");
}

}

//----------------------------------------------------------------------------
int UnitTestReadWriteMeshJSON(int, char**)
{
  verify_writing_and_loading_collection();
  verify_writing_and_loading_multiple_collections();

  verify_writing_collection_with_no_association_fails();

  verify_writing_of_single_collection();

  verify_loading_existing_collection_fails();

  return 0;
}
