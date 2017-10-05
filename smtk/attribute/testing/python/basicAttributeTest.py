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
Manual port of SMTK/smtk/attribute/Testing/basicAttributeTest.cxx
For verifying python wrappers

Requires smtkCorePython.so to be in module path
"""

import smtk
from smtk import attribute

if __name__ == '__main__':
    import sys
    import os

    status = 0

    collection = smtk.attribute.Collection.create()
    print 'Collection created'
    t = collection.type()
    if t != smtk.resource.Resource.ATTRIBUTE:
        print 'ERROR: Returned wrong resource type'
        status = -1
    print 'Resource type:', smtk.resource.Resource.type2String(t)
    def_ = collection.createDefinition('testDef')
    if def_ is not None:
        print 'Definition testDef created'
    else:
        print 'ERROR: Definition testDef not created'
        status = -1
    def1 = collection.createDefinition("testDef")
    if def1 is None:
        print 'Duplicated definition testDef not created'
    else:
        print 'ERROR: Duplicated definition testDef created'
        status = -1
    att = collection.createAttribute('testAtt', 'testDef')
    if not att is None:
        print 'Attribute testAtt created'
    else:
        print 'ERROR: Attribute testAtt not created'
        status = -1

    att1 = collection.createAttribute('testAtt', 'testDef')
    if att1 is None:
        print 'Duplicate Attribute testAtt not created'
    else:
        print 'ERROR: Duplicate Attribute testAtt  created'
        status = -1

    if att.isColorSet():
        print "Color should not be set."
        status = -1
    att.setColor(3, 24, 12, 6)
    tcol = att.color()
    if not att.isColorSet():
        print "Color should be set.\n"
        status = -1
    att.unsetColor()
    if att.isColorSet():
        print "Color should not be set.\n"
        status = -1
    if len(att.associatedModelEntityIds()) != 0:
        print "Should not have associated entities.\n"
        status = -1
    if att.appliesToBoundaryNodes():
        print "Should not be applies to boundry node.\n"
        status = -1
    att.setAppliesToBoundaryNodes(True)
    if not att.appliesToBoundaryNodes():
        print "Should be applies to boundry node.\n"
        status = -1
    att.setAppliesToBoundaryNodes(False)
    if att.appliesToBoundaryNodes():
        print "Should not be applies to boundry node.\n"
        status = -1
    if att.appliesToInteriorNodes():
        print "Should not be applies to interior node.\n"
        status = -1
    att.setAppliesToInteriorNodes(True)
    if not att.appliesToInteriorNodes():
        print "Should be applies to interior node.\n"
        status = -1
    att.setAppliesToInteriorNodes(False)
    if att.appliesToInteriorNodes():
        print "Should not applies to interior node.\n"
        status = -1
#    if att.collection() is not None:
#       print "Should not be null.\n"
#       status = -1
    del collection
    print 'Collection destroyed'

    sys.exit(status)
