/*=========================================================================

Copyright (c) 1998-2014 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/


#include "smtk/attribute/Manager.h"
#include "smtk/common/ResourceSet.h"

#include <iostream>

// Basic tests for smtk::common::ResourceSet

int main(int /* argc */, const char* /* argv */[])
{
  int status = 0;
  bool result;
  unsigned n;
  smtk::common::ResourceSet resourceSet;

  // Create and add attribute manager
  smtk::attribute::ManagerPtr manager1(new  smtk::attribute::Manager());
  result = resourceSet.addResource(manager1, "manager1", "",
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

  // Create amd add 2nd attribute manager
  smtk::attribute::ManagerPtr manager2(new  smtk::attribute::Manager());
  result = resourceSet.addResource(manager2, "manager2", "path2",
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

  // Add 1st manager w/different id and role
  result = resourceSet.addResource(manager1, "manager1-different-id", "",
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
  result = resourceSet.addResource(manager2, "manager2");
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
      { "manager1", "manager2", "manager1-different-id" };
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
  result = resourceSet.resourceInfo("manager2", rtype, role, state, link);
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
  result = resourceSet.get("manager2", resource);
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
