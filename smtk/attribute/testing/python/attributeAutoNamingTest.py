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
"""
Manual port of SMTK/smtk/attribute/Testing/attributeAutoNamingTest.cxx
For verifying python wrappers

Requires smtkCorePython.so to be in module path
"""

import smtk
from smtk import attribute

if __name__ == '__main__':
    import sys

    status = 0

    collection = smtk.attribute.Collection.create()
    print('Collection Created')
    def_ = collection.createDefinition("testDef")
    if def_ is not None:
        print('Definition testDef created')
    else:
        print('ERROR: Definition testDef not created')
        status = -1

    att = collection.createAttribute("testDef")
    if att is not None:
        print('Attribute %s created' % att.name())
    else:
        print("ERROR: 1st Attribute not created")
        status = -1

    att = collection.createAttribute("testDef")
    if att is not None:
        print("Attribute %s created" % att.name())
    else:
        print("ERROR: 2nd Attribute not created")
        status = -1

    att = collection.createAttribute("testDef")
    if att is not None:
        print("Attribute %s created" % att.name())
    else:
        print("ERROR: 3rd Attribute not created")
        status = -1

    del collection
    print('Collection destroyed')

    sys.exit(status)
