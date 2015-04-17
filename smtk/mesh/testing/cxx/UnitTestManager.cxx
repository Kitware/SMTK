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

#include "smtk/mesh/testing/cxx/helpers.h"

#include <algorithm>

namespace
{
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
  smtk::mesh::Manager::const_iterator begin = mgr->collectionBegin();
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
  //todo need to test all of this

}

//----------------------------------------------------------------------------
void verify_add_remove_association()
{
  //todo need to test all of this

}

}

//----------------------------------------------------------------------------
int UnitTestManager(int argc, char** argv)
{
  verify_constructors();

  verify_collection_queries();
  verify_collection_iterators();

  verify_add_remove_collection();

  verify_has_association();
  verify_add_remove_association();

  return 0;
}
