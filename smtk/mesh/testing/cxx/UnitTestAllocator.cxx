//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/json/Interface.h"
#include "smtk/mesh/moab/Interface.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

void verify_moab_allocator_creation()
{
  smtk::mesh::InterfacePtr iface = smtk::mesh::moab::make_interface();
  smtk::mesh::ResourcePtr resource = smtk::mesh::Resource::create(iface);

  test(resource->isValid(), "resource should be valid");
  test(!resource->isModified(), "resource shouldn't be modified");

  //at this point extract the allocator from json and verify that it
  //is NOT null
  smtk::mesh::AllocatorPtr allocator = resource->interface()->allocator();
  test(!!allocator, "moab allocator should be valid");

  //verify that is modified is true
  test(resource->isModified(), "resource should be modified once the allocator is accessed");
}

void verify_json_allocator_creation()
{
  smtk::mesh::InterfacePtr iface = smtk::mesh::json::make_interface();
  smtk::mesh::ResourcePtr resource = smtk::mesh::Resource::create(iface);

  test(resource->isValid(), "resource should be valid");
  test(!resource->isModified(), "resource shouldn't be modified");

  //at this point extract the allocator from json and verify that it
  //is null
  smtk::mesh::AllocatorPtr allocator = resource->interface()->allocator();
  test(!allocator, "json allocator should be nullptr");

  //verify that is modified is true
  test(!resource->isModified(), "resource shouldn't be modified");
}
} // namespace

int UnitTestAllocator(int /*unused*/, char** const /*unused*/)
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
