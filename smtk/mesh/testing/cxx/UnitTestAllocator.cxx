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

void verify_moab_allocator_creation()
{
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::mesh::InterfacePtr iface = smtk::mesh::moab::make_interface();
  smtk::mesh::CollectionPtr collection = mgr->makeCollection(iface);

  test( collection->isValid(), "collection should be valid");
  test( !collection->isModified(), "collection shouldn't be modified");

  //at this point extract the allocator from json and verify that it
  //is NOT null
  smtk::mesh::AllocatorPtr allocator = collection->interface()->allocator();
  test( !!allocator, "moab allocator should be valid");

  //verify that is modified is true
  test( collection->isModified(), "collection should be modified once the allocator is accessed");
}

void verify_json_allocator_creation()
{
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::mesh::InterfacePtr iface = smtk::mesh::json::make_interface();
  smtk::mesh::CollectionPtr collection = mgr->makeCollection(iface);

  test( collection->isValid(), "collection should be valid");
  test( !collection->isModified(), "collection shouldn't be modified");

  //at this point extract the allocator from json and verify that it
  //is null
  smtk::mesh::AllocatorPtr allocator = collection->interface()->allocator();
  test( !allocator, "json allocator should be NULL");

  //verify that is modified is true
  test( !collection->isModified(), "collection shouldn't be modified");

}

}

int UnitTestAllocator(int, char** const)
{
  verify_moab_allocator_creation();
  verify_json_allocator_creation();

  //we need to verify simple allocation of:
  //points
  //1D cells
  //2D cells
  //3D cells

  //verify that you get errors when allocation cells of 0D

  //show how to add explicit points to a MeshSet

  return 0;
}
