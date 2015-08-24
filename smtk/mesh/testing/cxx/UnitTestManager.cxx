//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/Manager.h"
#include "smtk/mesh/Collection.h"
#include "smtk/io/ModelToMesh.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Volume.h"
#include "smtk/model/EntityIterator.h"

#include "smtk/model/testing/cxx/helpers.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include <algorithm>

namespace {

const std::size_t num_volumes_in_model = 1;
const std::size_t num_models = 1;

//----------------------------------------------------------------------------
void create_simple_model( smtk::model::ManagerPtr modelManager )
{
  using namespace smtk::model::testing;

  smtk::model::SessionRef sess = modelManager->createSession("native");
  smtk::model::Model model = modelManager->addModel();
  smtk::common::UUIDArray uids = createTet(modelManager);
  model.addCell( smtk::model::Volume(modelManager, uids[21]));

  model.setSession(sess);
  modelManager->assignDefaultNames();

}

//----------------------------------------------------------------------------
void verify_constructors()
{
  smtk::mesh::ManagerPtr mgrPtr;
  test( !mgrPtr, "empty shared_ptr to Manager test failed");

  smtk::mesh::ManagerPtr mgrPtr2 = smtk::mesh::Manager::create();
  test( !!mgrPtr2, "failed create method on Manager");

  mgrPtr = mgrPtr2;
  test( !!mgrPtr, "failed copy assignment of manager shared_ptr");
  test( !!mgrPtr2, "failed copy assignment of manager shared_ptr");

  smtk::mesh::ManagerPtr mgrPtr3(mgrPtr);
  test( !!mgrPtr3, "failed copy assignment of manager shared_ptr");
}

//----------------------------------------------------------------------------
void verify_collection_queries()
{
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();

  smtk::mesh::CollectionPtr collection = mgr->makeCollection();
  test( mgr->numberOfCollections() == 1, "Incorrect results from numberOfCollections");

  const bool has = mgr->hasCollection(collection);
  test( has, "Incorrect results from hasCollection");

  smtk::mesh::CollectionPtr collection_bad = smtk::mesh::Collection::create();
  bool doesnt_have = !mgr->hasCollection(collection_bad);
  test( doesnt_have, "Incorrect results from hasCollection when giving a bad collection");

  //test search capabilities
  smtk::mesh::CollectionPtr findBad = mgr->collection( smtk::common::UUID() );
  smtk::mesh::CollectionPtr findBad2 = mgr->collection( smtk::common::UUID() );
  smtk::mesh::CollectionPtr findRes = mgr->collection( collection->entity() );
  smtk::mesh::CollectionPtr findRes2 = mgr->findCollection( collection->entity() )->second;

  test( !findBad, "Failed to return an invalid Collection from a bad find call");
  test( !findBad2, "Failed to return an invalid Collection from a bad find call");
  test( findRes && findRes->isValid(), "Failed to return an valid Collection from a find call");
  test( findRes2 && findRes2->isValid(), "Failed to return an valid Collection from a find call");
}

//----------------------------------------------------------------------------
void verify_collection_iterators()
{
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();

  //first test iterators given no items in the manager
  test( mgr->numberOfCollections() == 0, "Incorrect results from numberOfCollections");

  smtk::mesh::Manager::const_iterator begin = mgr->collectionBegin();
  smtk::mesh::Manager::const_iterator end = mgr->collectionEnd();
  std::size_t distance = std::distance(begin,end);
  test( distance == 0, "Incorrect iterators given from an empty manager");

  smtk::mesh::CollectionPtr collection= mgr->makeCollection();
  test( mgr->numberOfCollections() == 1, "Incorrect results from numberOfCollections");

  //now that we have an item verify the iterators work
  begin = mgr->collectionBegin();
  end = mgr->collectionEnd();
  distance = std::distance(begin,end);
  test( distance == 1, "Incorrect iterators given from a manager with items");
}

//----------------------------------------------------------------------------
void verify_add_remove_collection()
{
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();

  const int size=1024;
  for( int i=0 ; i < size; ++i)
    {
    mgr->makeCollection(); //make a collection and add it to manager
    }
  test( mgr->numberOfCollections() == size, "Incorrect results from numberOfCollections");


  std::vector< smtk::common::UUID > uids_to_remove;
  for (int i=0; i < 10; ++i)
    { //remove the first item in the collection each time through
    smtk::mesh::Manager::const_iterator begin = mgr->collectionBegin();
    uids_to_remove.push_back( begin->first );
    const bool result = mgr->removeCollection( mgr->collection( begin->first ) );
    test( result == true, "failed to remove a collection from a manager");
    }

  test( mgr->numberOfCollections() == size-10, "Incorrect results from numberOfCollections");

  smtk::mesh::Manager::const_iterator end = mgr->collectionEnd();
  for(std::vector< smtk::common::UUID >::const_iterator i=uids_to_remove.begin();
      i != uids_to_remove.end();
      ++i)
    {
    smtk::mesh::CollectionPtr badCollection = mgr->collection( *i );
    smtk::mesh::Manager::const_iterator badIterator = mgr->findCollection( *i );
    test( !badCollection, "remove failed to actually remove a collection item");
    test( badIterator == end, "remove failed to actually remove a collection item");
    }

}

//----------------------------------------------------------------------------
void verify_has_association()
{
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::model::EntityRef invalidModelRef;

  //verify that the null uuid isn't part of a collection
  test( mgr->isAssociatedToACollection( invalidModelRef ) == false );

  //add some empty collection to the manager
  const int size=256;
  for( int i=0 ; i < size; ++i) { mgr->makeCollection(); }

  //verify that at this point no collections have associations
  test(mgr->collectionsWithAssociations().size() == 0);

  //next we are going to create a simple smtk model
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();
  create_simple_model(modelManager);

  //now that we have a model, convert it to a mesh
  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr collectionWithAssoc = convert(mgr,modelManager);
  test( mgr->numberOfCollections() == (size+1) );

  //now verify that the manager can see the association
  test(mgr->collectionsWithAssociations().size() == 1);

  //verify that the null uuid isn't part of a collection even after we
  //have an association
  test( mgr->isAssociatedToACollection( invalidModelRef ) == false );

  //walk the model and verify that we can query the Manager to get the
  //collection that is associated to it.
  smtk::model::EntityIterator it;
  smtk::model::EntityRefs models =
    modelManager->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(smtk::model::MODEL_ENTITY);
  it.traverse(models.begin(), models.end(), smtk::model::ITERATE_MODELS);

  std::size_t count = 0;
  for (it.begin(); !it.isAtEnd(); ++it)
    {
    const bool isAssociated = mgr->isAssociatedToACollection(*it);
    count += isAssociated ? 1 : 0;

    if( isAssociated )
      {
      std::vector< smtk::mesh::CollectionPtr > assocCollections =
                                            mgr->associatedCollections(*it);
      test( assocCollections.size() == 1);
      test( assocCollections[0] == collectionWithAssoc);
      }
    }
  test( num_models == count );

  //walk all the collections and get the model uuids that is aware of
  smtk::mesh::MeshSet meshes = collectionWithAssoc->meshes();
  smtk::common::UUIDArray knownAssociations = meshes.modelEntityIds();
  test( knownAssociations.size() == num_volumes_in_model);

  typedef smtk::common::UUIDArray::const_iterator cit;
  for(cit i=knownAssociations.begin(); i != knownAssociations.end(); ++i)
    {
    smtk::model::EntityRef eref(modelManager, *i);
    test( mgr->isAssociatedToACollection( eref ) == true);
    }
}

//----------------------------------------------------------------------------
void verify_has_multiple_association()
{

  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  //add some collections to the manager that are associated to the same model
  create_simple_model(modelManager); //single model
  smtk::io::ModelToMesh convert;
  const int size=256;
  for( int i=0 ; i < size; ++i)
    { //convert the model into 256 different mesh collections
    convert(mgr,modelManager);
    }

  //verify that at this point no collections have associations
  typedef std::vector< smtk::mesh::CollectionPtr > AssocCollections;
  AssocCollections assocCollections = mgr->collectionsWithAssociations();
  test(assocCollections.size() == size);

  //next verify that we get a multiple collections back when we request
  //an association
  smtk::model::EntityIterator it;
  smtk::model::EntityRefs models =
    modelManager->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(smtk::model::MODEL_ENTITY);
  it.traverse(models.begin(), models.end(), smtk::model::ITERATE_MODELS);

  //nothing should be associated
  for (it.begin(); !it.isAtEnd(); ++it)
    {
    if( mgr->isAssociatedToACollection(*it)  )
      {
      test(  (mgr->associatedCollections(*it).size() == size) );
      }
    }
}

//----------------------------------------------------------------------------
void verify_add_remove_association()
{
  //first add a collection that is associated.
  //collected the uuids that are associated
  //remove the collection
  //verify none of the uuids are associated
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  //add some collections to the manager that are associated to the same model
  create_simple_model(modelManager); //single model
  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr collectionWithAssoc = convert(mgr,modelManager);

  typedef std::vector< smtk::mesh::CollectionPtr > AssocCollections;
  AssocCollections assocCollections = mgr->collectionsWithAssociations();
  test(assocCollections.size() == 1);

  smtk::mesh::MeshSet meshes = collectionWithAssoc->meshes();
  smtk::common::UUIDArray knownAssociations = meshes.modelEntityIds();

  typedef smtk::common::UUIDArray::const_iterator cit;
  for(cit i=knownAssociations.begin(); i != knownAssociations.end(); ++i)
    {
    smtk::model::EntityRef eref(modelManager, *i);
    test( mgr->isAssociatedToACollection( eref ) == true);
    }


  //now remove the collection.
  mgr->removeCollection( collectionWithAssoc );
  for(cit i=knownAssociations.begin(); i != knownAssociations.end(); ++i)
    {
    smtk::model::EntityRef eref(modelManager, *i);
    test( mgr->isAssociatedToACollection( eref ) == false);
    }

  assocCollections = mgr->collectionsWithAssociations();
  test(assocCollections.size() == 0);
}

//----------------------------------------------------------------------------
void verify_no_association()
{
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::model::EntityRef invalidModelRef;

  //verify that the null uuid isn't part of any collection
  test( mgr->isAssociatedToACollection( invalidModelRef ) == false );

  //next we are going to create a simple smtk model
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();
  create_simple_model(modelManager);

  //now verify that none of the valid model uuids are part of the association
  smtk::model::EntityIterator it;
  smtk::model::EntityRefs models =
    modelManager->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(smtk::model::MODEL_ENTITY);
  it.traverse(models.begin(), models.end(), smtk::model::ITERATE_MODELS);

  //nothing should be associated
  for (it.begin(); !it.isAtEnd(); ++it)
    {
    test( mgr->isAssociatedToACollection(*it) == false );
    }
}

} // anonymous namespace

//----------------------------------------------------------------------------
int UnitTestManager(int, char**)
{
  verify_constructors();

  verify_collection_queries();
  verify_collection_iterators();

  verify_add_remove_collection();

  verify_has_association();
  verify_has_multiple_association();
  verify_add_remove_association();
  verify_no_association();

  return 0;
}
