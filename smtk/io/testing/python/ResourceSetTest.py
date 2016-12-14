#!/usr/bin/python
import sys
#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================
from pprint import pprint
import smtk
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.attribute
    import smtk.common
from smtk.simple import *

def RSTest():
    """Tests for ResourceSet
    Currently does not test for ResourceInfo
    as it is not implemented in the wrapper yet
    Otherwise this is a direct manual port of the cxx test"""

    status = 0
    result = False
    n = 0
    resourceSet = smtk.common.ResourceSet()

    system1 = smtk.attribute.System.New()
    print system1
    print system1.resourceType()
    result = resourceSet.addResource(system1, "system1", "", smtk.common.ResourceSet.TEMPLATE);

    n = resourceSet.numberOfResources()
    if result == False:
        print("addResource() call failed")
        status = status + 1
    elif n != 1:
        print("Wrong number of resources: %i, should be 1" % n)
        status = status + 1



    system2 = smtk.attribute.System.New()
    result = resourceSet.addResource(system2, "system2", "path2", smtk.common.ResourceSet.INSTANCE);

    n = resourceSet.numberOfResources()
    if result == False:
        print("addResource() call failed")
        status = status + 1
    elif n != 2:
        print("Wrong number of resources: %i, should be 2" % n)
        status = status + 1


    result = resourceSet.addResource(system1, "system1-different-id", "", smtk.common.ResourceSet.SCENARIO);
    n = resourceSet.numberOfResources()
    if result == False:
        print("addResource() call failed")
        status = status + 1
    elif n != 3:
        print("Wrong number of resources: %i, should be 3" % n)
        status = status + 1


    result = resourceSet.addResource(system2, "system2")
    n = resourceSet.numberOfResources()
    if result == True:
        print("addResource() call didn't fail failed")
        status = status + 1
    elif n != 3:
        print("Wrong number of resources: %i, should be 3" % n)
        status = status + 1


    ids = resourceSet.resourceIds()
    if len(ids) != 3:
        print("Wrong number of ids: %i, should be 3")
        status = status + 1
    else:
        expectedNames = ["system1", "system2", "system1-different-id"]
        for i in range(len(ids)):
            if ids[i] != expectedNames[i]:
                print("Wrong resource name %s, should be %s" % (ids[i], expectedNames[i]) )
                status = status + 1


    # Missing: ResourceInfo tests (function not implemented)

    # Note: ResourcePtr is not implemented (and cannot be due Resource being abstract -- shiboken issues)
    # Note: ResourceSet.get is modified by shiboken to return a ResourcePtr/shared_ptr<Resource>

    resource = resourceSet.get("system2")
    if resource == None:
        print("get() failed")
        status = status + 1
    rtype = resource.resourceType()
    if rtype != smtk.common.Resource.ATTRIBUTE:
        print("Incorrect resource type %s, should be smtk.common.Resource.ATTRIBUTE" % rtype)
        status = status + 1

    print("Number of errors: %i" % status)
    return status

def test():
    return RSTest()

if __name__ == '__main__':
    sys.exit(test())
