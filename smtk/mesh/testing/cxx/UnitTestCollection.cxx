//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

//----------------------------------------------------------------------------
void verify_invalid_constructor()
{
  smtk::mesh::CollectionPtr null_collec;
  test( !null_collec, "collection  pointer should be invalid");

  smtk::mesh::CollectionPtr invalid_collection = smtk::mesh::Collection::create();
  test( !invalid_collection->isValid() , "collection should be invalid");

  smtk::common::UUID uid = invalid_collection->entity();
  test( (uid==smtk::common::UUID::null()) , "collection uuid should be null");
}

//----------------------------------------------------------------------------
void verify_valid_constructor()
{
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr collection = mgr->makeCollection();

  test( collection->isValid() , "collection should be valid");

  smtk::common::UUID uid = collection->entity();
  test( (uid!=smtk::common::UUID::null()) , "collection uuid should be valid");
}

//----------------------------------------------------------------------------
void verify_removal_from_collection()
{
  //add a collection to manager
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr collection = mgr->makeCollection();

  test( collection->isValid() , "collection should be valid as it related to a manager");

  //remove the collection
  const bool result = mgr->removeCollection( collection );
  (void)result;

  //verify the collection states that it is now invalid
  test( collection->isValid() == false, "removal from a manager should cause the collection to be invalid");

}

//----------------------------------------------------------------------------
void verify_reparenting()
{
  //add a collection to manager
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::mesh::ManagerPtr mgr2 = smtk::mesh::Manager::create();

  smtk::mesh::CollectionPtr collection = mgr->makeCollection();
  test( collection->isValid() , "collection should be valid as it related to a manager");

  //reparent to second manager
  bool reparenting_good = collection->reparent(mgr2);
  test( reparenting_good, "reparenting failed");
  test( collection->isValid() , "collection should be valid as it related to a manager");

  test( mgr->numberOfCollections() == 0, "Incorrect results from numberOfCollections");
  test( mgr2->numberOfCollections() == 1, "Incorrect results from numberOfCollections");
}

//----------------------------------------------------------------------------
void verify_reparenting_invalid_collection()
{
  smtk::mesh::CollectionPtr invalid_collection = smtk::mesh::Collection::create();

  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  bool reparenting_good = invalid_collection->reparent(mgr);

  test( reparenting_good, "reparenting failed");
  test( invalid_collection->isValid() , "collection should be valid as it related to a manager");
  test( mgr->numberOfCollections() == 1, "Incorrect results from numberOfCollections");
}
//----------------------------------------------------------------------------
void verify_reparenting_twice()
{
  smtk::mesh::CollectionPtr collection= smtk::mesh::Collection::create();

  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  bool reparenting_good = collection->reparent(mgr);

  test( reparenting_good, "reparenting failed");
  test( collection->isValid() , "collection should be valid as it related to a manager");
  test( mgr->numberOfCollections() == 1, "Incorrect results from numberOfCollections");

  reparenting_good = collection->reparent(mgr);
  test( reparenting_good, "reparenting failed");
  test( collection->isValid() , "collection should be valid as it related to a manager");
  test( mgr->numberOfCollections() == 1, "Incorrect results from numberOfCollections");

}
//----------------------------------------------------------------------------
void verify_reparenting_after_manager_deletion()
{
  //add a collection to manager
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();

  smtk::mesh::CollectionPtr collection = mgr->makeCollection();
  test( collection->isValid() , "collection should be valid as it related to a manager");
  test( mgr->numberOfCollections() == 1, "Incorrect results from numberOfCollections");

  //remove the manager
  mgr.reset();
  test( !collection->isValid() , "collection shouldn't be valid as manager is deleted");

  //reparent to second manager
  smtk::mesh::ManagerPtr mgr2 = smtk::mesh::Manager::create();
  bool reparenting_good = collection->reparent(mgr2);
  test( reparenting_good, "reparenting failed");
  test( collection->isValid() , "collection should be valid as it related to a manager");
  test( mgr2->numberOfCollections() == 1, "Incorrect results from numberOfCollections");
}

}

//----------------------------------------------------------------------------
int UnitTestCollection(int, char**)
{
  verify_invalid_constructor();
  verify_valid_constructor();

  verify_removal_from_collection();


  verify_reparenting();
  verify_reparenting_invalid_collection();
  verify_reparenting_twice();
  verify_reparenting_after_manager_deletion();
  return 0;
}
