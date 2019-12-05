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

void verify_valid_constructor()
{
  smtk::mesh::ResourcePtr resource = smtk::mesh::Resource::create();

  test(resource->isValid(), "resource should be valid");
  test(resource->isModified() == false, "resource shouldn't be marked as modified");

  smtk::common::UUID uid = resource->entity();
  test((uid != smtk::common::UUID::null()), "resource uuid should be valid");

  //verify the name
  test(resource->name().empty());
  resource->setName("example");
  test((resource->name() == std::string("example")));

  //verify the interface name
  test(!resource->interfaceName().empty());

  //verify the read and write location
  test((resource->readLocation() == std::string()));
  test((resource->writeLocation() == std::string()));
}

void verify_resource_info_moab()
{
  smtk::mesh::InterfacePtr iface = smtk::mesh::moab::make_interface();
  smtk::mesh::ResourcePtr resource = smtk::mesh::Resource::create(iface);

  test(resource->isValid(), "resource should be valid");

  smtk::common::UUID uid = resource->entity();
  test((uid != smtk::common::UUID::null()), "resource uuid should be valid");

  //verify the name
  test(resource->name().empty());
  resource->setName("example");
  test((resource->name() == std::string("example")));

  //verify the interface name
  test(!resource->interfaceName().empty());
  test((resource->interfaceName() == std::string("moab")));

  //verify the read and write location
  test((resource->readLocation() == std::string()));
  test((resource->writeLocation() == std::string()));

  //set and check read/write location
  resource->writeLocation("foo");
  test((resource->readLocation() == std::string()));
  test((resource->writeLocation() == std::string("foo")));
}

void verify_resource_info_json()
{
  smtk::mesh::InterfacePtr iface = smtk::mesh::json::make_interface();
  smtk::mesh::ResourcePtr resource = smtk::mesh::Resource::create(iface);

  test(resource->isValid(), "resource should be valid");

  smtk::common::UUID uid = resource->entity();
  test((uid != smtk::common::UUID::null()), "resource uuid should be valid");

  //verify the name
  test(resource->name().empty());
  resource->setName("example");
  test((resource->name() == std::string("example")));

  //verify the interface name
  test(!resource->interfaceName().empty());
  test((resource->interfaceName() == std::string("json")));

  //verify the read and write location
  test((resource->readLocation() == std::string()));
  test((resource->writeLocation() == std::string()));

  //set and check read/write location
  resource->writeLocation("foo");
  test((resource->readLocation() == std::string()));
  test((resource->writeLocation() == std::string("foo")));
}
}

int UnitTestResource(int, char** const)
{
  verify_valid_constructor();

  verify_resource_info_moab();
  verify_resource_info_json();

  return 0;
}
