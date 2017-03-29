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
Manual port of SMTK/smtk/attribute/Testing/atributeAutoNameingTest.cxx
For verifying python-shiboken wrappers

Requires smtkCorePython.so to be in module path
"""

import smtk
from smtk import attribute

if __name__ == '__main__':
    import sys

    status = 0

    system = smtk.attribute.System()
    print "System Created"
    def_ = system.createDefinition("testDef")
    if def_ is not None:
        print "Definition testDef created"
    else:
        print "ERROR: Definition testDef not created"
        status = -1

    att = system.createAttribute("testDef")
    if att is not None:
        print "Attribute %s created" % att.name()
    else:
        print "ERROR: 1st Attribute not created"
        status = -1

    att = system.createAttribute("testDef")
    if att is not None:
        print "Attribute %s created" % att.name()
    else:
        print "ERROR: 2nd Attribute not created"
        status = -1

    att = system.createAttribute("testDef")
    if att is not None:
        print "Attribute %s created" % att.name()
    else:
        print "ERROR: 3rd Attribute not created"
        status = -1

    del system
    print 'System destroyed'

    sys.exit(status)
