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
For verifying python-shiboken wrappers

Requires smtkCorePython.so to be in module path
"""

import smtk

if __name__ == '__main__':
    import sys

    status = 0

    system = smtk.attribute.System()
    print 'System created'
    t = system.resourceType()
    if t != smtk.common.Resource.ATTRIBUTE:
      print 'ERROR: Returned wrong resource type'
      status = -1
    print 'Resource type:', smtk.common.Resource.type2String(t)
    def_ = system.createDefinition('testDef')
    if def_ is not None:
        print 'Definition testDef created'
    else:
        print 'ERROR: Definition testDef not created'
        status = -1
    def1 = system.createDefinition("testDef")
    if def1 is None:
        print 'Duplicated definition testDef not created'
    else:
        print 'ERROR: Duplicated definition testDef created'
        status = -1
    att = system.createAttribute('testAtt', 'testDef')
    if not att is None:
        print 'Attribute testAtt created'
    else:
        print 'ERROR: Attribute testAtt not created'
        status = -1

    att1 = system.createAttribute('testAtt', 'testDef')
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
#    if att.system() is not None:
#       print "Should not be null.\n"
#       status = -1
    del system
    print 'System destroyed'

    sys.exit(status)
