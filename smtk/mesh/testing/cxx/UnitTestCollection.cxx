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

#include "smtk/mesh/json/Interface.h"
#include "smtk/mesh/moab/Interface.h"

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
  test( collection->isModified() == false, "collection shouldn't be marked as modified");

  smtk::common::UUID uid = collection->entity();
  test( (uid!=smtk::common::UUID::null()) , "collection uuid should be valid");

  //verify the name
  test( (collection->name() == std::string()) );
  collection->name("example");
  test( (collection->name() == std::string("example")) );

  //verify the interface name
  test( (collection->interfaceName() != std::string()) );

  //verify the read and write location
  test( (collection->readLocation() == std::string()) );
  test( (collection->writeLocation() == std::string()) );
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

//----------------------------------------------------------------------------
void verify_collection_unique_name()
{
  //very simple test for unique name assignment
  //a far more robust test is done in UnitTestManager
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();

  smtk::mesh::CollectionPtr c1 = mgr->makeCollection();
  smtk::mesh::CollectionPtr c2 = mgr->makeCollection();

  c1->name( std::string("a") );
  c1->assignUniqueNameIfNotAlready();
  test( c1->name() == std::string("a"), "assignUniqueNameIfNotAlready overwrote existing unique name" );

  c2->name( std::string("a") );
  c1->assignUniqueNameIfNotAlready();
  test( c1->name() != std::string("a"), "assignUniqueNameIfNotAlready didn't generate an unique name" );
}

//----------------------------------------------------------------------------
void verify_collection_info_moab()
{
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::mesh::InterfacePtr iface = smtk::mesh::moab::make_interface();
  smtk::mesh::CollectionPtr collection = mgr->makeCollection(iface);

  test( collection->isValid() , "collection should be valid");

  smtk::common::UUID uid = collection->entity();
  test( (uid!=smtk::common::UUID::null()) , "collection uuid should be valid");

  //verify the name
  test( (collection->name() == std::string()) );
  collection->name("example");
  test( (collection->name() == std::string("example")) );

  //verify the interface name
  test( (collection->interfaceName() != std::string()) );
  test( (collection->interfaceName() == std::string("moab")) );

  //verify the read and write location
  test( (collection->readLocation() == std::string()) );
  test( (collection->writeLocation() == std::string()) );

  //set and check read/write location
  collection->writeLocation("foo");
  test( (collection->readLocation() == std::string()) );
  test( (collection->writeLocation() == std::string("foo")) );
}

//----------------------------------------------------------------------------
void verify_collection_info_json()
{
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::mesh::InterfacePtr iface = smtk::mesh::json::make_interface();
  smtk::mesh::CollectionPtr collection = mgr->makeCollection(iface);

  test( collection->isValid() , "collection should be valid");

  smtk::common::UUID uid = collection->entity();
  test( (uid!=smtk::common::UUID::null()) , "collection uuid should be valid");

  //verify the name
  test( (collection->name() == std::string()) );
  collection->name("example");
  test( (collection->name() == std::string("example")) );

  //verify the interface name
  test( (collection->interfaceName() != std::string()) );
  test( (collection->interfaceName() == std::string("json")) );

  //verify the read and write location
  test( (collection->readLocation() == std::string()) );
  test( (collection->writeLocation() == std::string()) );

  //set and check read/write location
  collection->writeLocation("foo");
  test( (collection->readLocation() == std::string()) );
  test( (collection->writeLocation() == std::string("foo")) );
}
}

//----------------------------------------------------------------------------
int UnitTestCollection(int, char** const)
{
  verify_invalid_constructor();
  verify_valid_constructor();

  verify_removal_from_collection();

  verify_reparenting();
  verify_reparenting_invalid_collection();
  verify_reparenting_twice();
  verify_reparenting_after_manager_deletion();

  verify_collection_unique_name();
  verify_collection_info_moab();
  verify_collection_info_json();

  return 0;
}
