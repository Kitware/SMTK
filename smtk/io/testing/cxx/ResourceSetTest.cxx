//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/resource/Set.h"

#include "smtk/attribute/Resource.h"

#include <iostream>

// Basic tests for smtk::resource::Set

int main(int /* argc */, const char* /* argv */ [])
{
  int status = 0;
  bool result;
  unsigned n;
  smtk::resource::Set resourceSet;

  // Create and add attribute resource
  smtk::attribute::ResourcePtr resource1 = smtk::attribute::Resource::create();
  result = resourceSet.add(resource1, "resource1", "", smtk::resource::Set::TEMPLATE);
  n = static_cast<unsigned>(resourceSet.numberOfResources());
  if (!result)
  {
    std::cerr << "add() call failed" << std::endl;
    status += 1;
  }
  else if (n != 1)
  {
    std::cerr << "Wrong number of resources: " << n << ", should be 1" << std::endl;
    status += 1;
  }

  // Create amd add 2nd attribute resource
  smtk::attribute::ResourcePtr resource2 = smtk::attribute::Resource::create();
  result = resourceSet.add(resource2, "resource2", "path2", smtk::resource::Set::INSTANCE);
  n = static_cast<unsigned>(resourceSet.numberOfResources());
  if (!result)
  {
    std::cerr << "add() call failed" << std::endl;
    status += 1;
  }
  else if (n != 2)
  {
    std::cerr << "Wrong number of resources: " << n << ", should be 2" << std::endl;
    status += 1;
  }

  // Add 1st resource w/different id and role
  result = resourceSet.add(resource1, "resource1-different-id", "", smtk::resource::Set::SCENARIO);
  n = static_cast<unsigned>(resourceSet.numberOfResources());
  if (!result)
  {
    std::cerr << "add() call failed" << std::endl;
    status += 1;
  }
  else if (n != 3)
  {
    std::cerr << "Wrong number of resources: " << n << ", should be 3" << std::endl;
    status += 1;
  }

  // Try using same id twice
  result = resourceSet.add(resource2, "resource2");
  n = static_cast<unsigned>(resourceSet.numberOfResources());
  if (result)
  {
    std::cerr << "add() call didn't fail" << std::endl;
    status += 1;
  }
  else if (n != 3)
  {
    std::cerr << "Wrong number of resources: " << n << ", should be 3" << std::endl;
    status += 1;
  }

  // Check resource ids
  std::vector<std::string> ids = resourceSet.resourceIds();
  if (ids.size() != 3)
  {
    std::cerr << "Wrong number of ids: " << ids.size() << ", should be 3" << std::endl;
    status += 1;
  }
  else
  {
    const char* expectedNames[] = { "resource1", "resource2", "resource1-different-id" };
    for (size_t i = 0; i < ids.size(); i++)
    {
      if (ids[i] != expectedNames[i])
      {
        std::cerr << "Wrong resource name " << ids[i] << ", should be " << expectedNames[i]
                  << std::endl;
        status += 1;
      }
    }
  }

  // Check resource info
  smtk::resource::Set::Role role;
  smtk::resource::Set::State state;
  std::string link;
  result = resourceSet.resourceInfo("resource2", role, state, link);
  if (!result)
  {
    std::cerr << "info() call failed" << std::endl;
    status += 1;
  }
  else
  {
    if (role != smtk::resource::Set::INSTANCE)
    {
      std::cerr << "Incorrect resource role " << role << ", should be "
                << smtk::resource::Set::INSTANCE << std::endl;
      status += 1;
    }
    if (state != smtk::resource::Set::LOADED)
    {
      std::cerr << "Incorrect resource state " << state << ", should be "
                << smtk::resource::Set::LOADED << std::endl;
      status += 1;
    }
    if (link != "path2")
    {
      std::cerr << "Incorrect resource link \"" << link << "\", should be \"path2\"" << std::endl;
      status += 1;
    }
  }

  // Retrieve resource
  smtk::resource::ResourcePtr resource;
  result = resourceSet.get("resource2", resource);
  if (!result)
  {
    std::cerr << "get() failed" << std::endl;
    status += 1;
  }

  std::cout << "Number of errors: " << status << std::endl;
  return status;
}
