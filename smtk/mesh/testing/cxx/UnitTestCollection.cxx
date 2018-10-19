//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/core/Collection.h"

#include "smtk/mesh/json/Interface.h"
#include "smtk/mesh/moab/Interface.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

void verify_valid_constructor()
{
  smtk::mesh::CollectionPtr collection = smtk::mesh::Collection::create();

  test(collection->isValid(), "collection should be valid");
  test(collection->isModified() == false, "collection shouldn't be marked as modified");

  smtk::common::UUID uid = collection->entity();
  test((uid != smtk::common::UUID::null()), "collection uuid should be valid");

  //verify the name
  test((collection->name() == std::string()));
  collection->name("example");
  test((collection->name() == std::string("example")));

  //verify the interface name
  test((collection->interfaceName() != std::string()));

  //verify the read and write location
  test((collection->readLocation() == std::string()));
  test((collection->writeLocation() == std::string()));
}

void verify_collection_info_moab()
{
  smtk::mesh::InterfacePtr iface = smtk::mesh::moab::make_interface();
  smtk::mesh::CollectionPtr collection = smtk::mesh::Collection::create(iface);

  test(collection->isValid(), "collection should be valid");

  smtk::common::UUID uid = collection->entity();
  test((uid != smtk::common::UUID::null()), "collection uuid should be valid");

  //verify the name
  test((collection->name() == std::string()));
  collection->name("example");
  test((collection->name() == std::string("example")));

  //verify the interface name
  test((collection->interfaceName() != std::string()));
  test((collection->interfaceName() == std::string("moab")));

  //verify the read and write location
  test((collection->readLocation() == std::string()));
  test((collection->writeLocation() == std::string()));

  //set and check read/write location
  collection->writeLocation("foo");
  test((collection->readLocation() == std::string()));
  test((collection->writeLocation() == std::string("foo")));
}

void verify_collection_info_json()
{
  smtk::mesh::InterfacePtr iface = smtk::mesh::json::make_interface();
  smtk::mesh::CollectionPtr collection = smtk::mesh::Collection::create(iface);

  test(collection->isValid(), "collection should be valid");

  smtk::common::UUID uid = collection->entity();
  test((uid != smtk::common::UUID::null()), "collection uuid should be valid");

  //verify the name
  test((collection->name() == std::string()));
  collection->name("example");
  test((collection->name() == std::string("example")));

  //verify the interface name
  test((collection->interfaceName() != std::string()));
  test((collection->interfaceName() == std::string("json")));

  //verify the read and write location
  test((collection->readLocation() == std::string()));
  test((collection->writeLocation() == std::string()));

  //set and check read/write location
  collection->writeLocation("foo");
  test((collection->readLocation() == std::string()));
  test((collection->writeLocation() == std::string("foo")));
}
}

int UnitTestCollection(int, char** const)
{
  verify_valid_constructor();

  verify_collection_info_moab();
  verify_collection_info_json();

  return 0;
}
