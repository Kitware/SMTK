//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/System.h"
#include "smtk/common/ResourceSet.h"

#include <iostream>

// Basic tests for smtk::common::ResourceSet

int main(int /* argc */, const char* /* argv */[])
{
  int status = 0;
  bool result;
  unsigned n;
  smtk::common::ResourceSet resourceSet;

  // Create and add attribute system
  smtk::attribute::SystemPtr system1(new  smtk::attribute::System());
  result = resourceSet.addResource(system1, "system1", "",
                                   smtk::common::ResourceSet::TEMPLATE);
  n = resourceSet.numberOfResources();
  if (!result)
    {
    std::cerr << "addResource() call failed" << std::endl;
    status += 1;
    }
  else if (n != 1)
    {
    std::cerr << "Wrong number of resources: " << n
              << ", should be 1" << std::endl;
    status += 1;
    }

  // Create amd add 2nd attribute system
  smtk::attribute::SystemPtr system2(new  smtk::attribute::System());
  result = resourceSet.addResource(system2, "system2", "path2",
                                   smtk::common::ResourceSet::INSTANCE);
  n = resourceSet.numberOfResources();
  if (!result)
    {
    std::cerr << "addResource() call failed" << std::endl;
    status += 1;
    }
  else if (n != 2)
    {
    std::cerr << "Wrong number of resources: " << n
              << ", should be 2" << std::endl;
    status += 1;
    }

  // Add 1st system w/different id and role
  result = resourceSet.addResource(system1, "system1-different-id", "",
                                   smtk::common::ResourceSet::SCENARIO);
  n = resourceSet.numberOfResources();
  if (!result)
    {
    std::cerr << "addResource() call failed" << std::endl;
    status += 1;
    }
  else if (n != 3)
    {
    std::cerr << "Wrong number of resources: " << n
              << ", should be 3" << std::endl;
    status += 1;
    }

  // Try using same id twice
  result = resourceSet.addResource(system2, "system2");
  n = resourceSet.numberOfResources();
  if (result)
    {
    std::cerr << "addResource() call didn't fail" << std::endl;
    status += 1;
    }
  else if (n != 3)
    {
    std::cerr << "Wrong number of resources: " << n
              << ", should be 3" << std::endl;
    status += 1;
    }

  // Check resource ids
  std::vector<std::string> ids = resourceSet.resourceIds();
  if (ids.size() != 3)
    {
    std::cerr << "Wrong number of ids: " << ids.size()
              << ", should be 3" << std::endl;
    status +=1;
    }
  else
    {
    const char * expectedNames[] =
      { "system1", "system2", "system1-different-id" };
    for (unsigned i = 0; i<ids.size(); i++)
      {
      if (ids[i] != expectedNames[i])
        {
        std::cerr << "Wrong resource name " << ids[i]
                  << ", should be " << expectedNames[i]
                  << std::endl;
        status += 1;
        }
      }
    }

  // Check resource info
  smtk::common::Resource::Type rtype;
  smtk::common::ResourceSet::ResourceRole role;
  smtk::common::ResourceSet::ResourceState state;
  std::string link;
  result = resourceSet.resourceInfo("system2", rtype, role, state, link);
  if (!result)
    {
    std::cerr << "info() call failed" << std::endl;
    status += 1;
    }
  else
    {
    if (rtype != smtk::common::Resource::ATTRIBUTE)
      {
      std::cerr << "Incorrect resource type " << rtype
                << ", should be " << smtk::common::Resource::ATTRIBUTE
                << std::endl;
      status += 1;
      }
    if (role != smtk::common::ResourceSet::INSTANCE)
      {
      std::cerr << "Incorrect resource role " << role
                << ", should be " << smtk::common::ResourceSet::INSTANCE
                << std::endl;
      status += 1;
      }
    if (state != smtk::common::ResourceSet::LOADED)
      {
      std::cerr << "Incorrect resource state " << state
                << ", should be " << smtk::common::ResourceSet::LOADED
                << std::endl;
      status += 1;
      }
    if (link != "path2")
      {
      std::cerr << "Incorrect resource link \"" << link
                << "\", should be \"path2\"" << std::endl;
      status += 1;
      }
    }

  // Retrieve resource
  smtk::common::ResourcePtr resource;
  result = resourceSet.get("system2", resource);
  if (!result)
    {
    std::cerr << "get() failed" << std::endl;
    status += 1;
    }
  rtype = resource->resourceType();
  if (rtype != smtk::common::Resource::ATTRIBUTE)
    {
    std::cerr << "Incorrect resource type " << rtype
              << ", should be " << smtk::common::Resource::ATTRIBUTE
              << std::endl;
    status += 1;
    }


  std::cout << "Number of errors: " << status << std::endl;
  return status;
}
