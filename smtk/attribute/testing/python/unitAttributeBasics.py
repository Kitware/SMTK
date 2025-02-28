# =============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
# =============================================================================
"""
Manual port of smtk/attribute/testing/cxx/unitAttributeBasics.cxx
For verifying python wrappers

Requires smtkCorePython.so to be in module path
"""

import smtk
from smtk import attribute
from smtk import simulation

if __name__ == '__main__':
    import sys
    import os

    status = 0

    resource = smtk.attribute.Resource.create()
    print('Resource created')
    def_ = resource.createDefinition('testDef')
    if def_ is not None:
        print('Definition testDef created')
    else:
        print('ERROR: Definition testDef not created')
        status = -1
    def1 = resource.createDefinition("testDef")
    if def1 is None:
        print('Duplicated definition testDef not created')
    else:
        print('ERROR: Duplicated definition testDef created')
        status = -1
    att = resource.createAttribute('testAtt', 'testDef')
    if not att is None:
        print('Attribute testAtt created')
    else:
        print('ERROR: Attribute testAtt not created')
        status = -1

    att1 = resource.createAttribute('testAtt', 'testDef')
    if att1 is None:
        print('Duplicate Attribute testAtt not created')
    else:
        print('ERROR: Duplicate Attribute testAtt  created')
        status = -1

    if att.isColorSet():
        print("Color should not be set.")
        status = -1
    att.setColor(3, 24, 12, 6)
    tcol = att.color()
    if not att.isColorSet():
        print("Color should be set.\n")
        status = -1
    att.unsetColor()
    if att.isColorSet():
        print("Color should not be set.\n")
        status = -1
    if len(att.associatedModelEntityIds()) != 0:
        print("Should not have associated entities.\n")
        status = -1
    if att.associatedObjects():
        print("Should not have associated components.\n")
        status = -1
    if att.appliesToBoundaryNodes():
        print("Should not be applies to boundry node.\n")
        status = -1
    att.setAppliesToBoundaryNodes(True)
    if not att.appliesToBoundaryNodes():
        print("Should be applies to boundry node.\n")
        status = -1
    att.setAppliesToBoundaryNodes(False)
    if att.appliesToBoundaryNodes():
        print("Should not be applies to boundry node.\n")
        status = -1
    if att.appliesToInteriorNodes():
        print("Should not be applies to interior node.\n")
        status = -1
    att.setAppliesToInteriorNodes(True)
    if not att.appliesToInteriorNodes():
        print("Should be applies to interior node.\n")
        status = -1
    att.setAppliesToInteriorNodes(False)
    if att.appliesToInteriorNodes():
        print("Should not applies to interior node.\n")
        status = -1
    # Testing User Data methods
    dataI = smtk.simulation.UserDataInt.New()
    dataI.setValue(10)
    att.setUserData("dataInt", dataI)
    dataS = smtk.simulation.UserDataString.New()
    dataS.setValue("foo")
    att.setUserData("dataString", dataS)
    data = att.userData("bar")
    if data is not None:
        print("Should not have found user data bar.\n")
        status = -1
    data = att.userData("dataInt")
    if data is None:
        print("Can not find user data dataInt.\n")
        status = -1
    else:
        if data.value() != 10:
            print("dataInt is not 10.\n")
            status = -1
    data = att.userData("dataString")
    if data is None:
        print("Can not find user data dataString.\n")
        status = -1
    else:
        if data.value() != "foo":
            print("dataString is not foo.\n")
            status = -1
    att.clearUserData("dataInt")
    data = att.userData("dataInt")
    if data is not None:
        print("Found user data dataInt after trying to clear it.\n")
        status = -1
    data = att.userData("dataString")
    if data is None:
        print(
            "Can not find user data dataString after removing dataInt from user data.\n")
        status = -1
    att.clearAllUserData()
    data = att.userData("dataString")
    if data is not None:
        print("Found user data dataString after removing all user data.\n")
        status = -1
    if status == 0:
        print("Passed all User Data Tests.\n")
#    if att.resource() is not None:
#       print("Should not be null.\n")
#       status = -1
    del resource
    print('Resource destroyed')

    sys.exit(status)
