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
Manual port of SMTK/smtk/attribute/Testing/attributeReferencingTest.cxx
For verifying python-shiboken wrappers

Requires smtkCorePython.so to be in module path
"""

import smtk
from smtk import attribute

if __name__ == '__main__':
    import sys
    import os

    status = 0

    system = smtk.attribute.System()
    print 'System created'

    #Lets create an attribute to represent an expression
    expDef = system.createDefinition("ExpDef")
    eitemdef = smtk.attribute.StringItemDefinition.New("Expression String")
    expDef.addItemDefinition(eitemdef)
    eitemdef2 = smtk.attribute.StringItemDefinition.New("Aux String")
    expDef.addItemDefinition(eitemdef2)
    eitemdef.setDefaultValue("sample");
    print eitemdef.name()
    print eitemdef2.name()

    base = system.createDefinition("BaseDef");

    #Lets add some item definitions
    ditemdef = smtk.attribute.DoubleItemDefinition.New("DoubleItem1")
    base.addItemDefinition(ditemdef)
    # Allow this one to hold an expression
    ditemdef.setExpressionDefinition(expDef)

    # Lets test creating an attribute by passing in the expression definition explicitly
    expAtt1 = system.createAttribute("Exp1", expDef);
    expAtt2 = system.createAttribute("Exp2", expDef);
    att  = system.createAttribute("testAtt1", "BaseDef");
    att1 = system.createAttribute("testAtt2", "BaseDef");
    att2 = system.createAttribute("testAtt3", "BaseDef");

    smtk.attribute.ValueItem.CastTo(att.item(0)).setExpression(expAtt1)
    smtk.attribute.ValueItem.CastTo(att1.item(0)).setExpression(expAtt1)
    smtk.attribute.ValueItem.CastTo(att2.item(0)).setExpression(expAtt2)

    #Lets see what attributes are being referenced
    refs = expAtt1.references()
    print "Number of Items referencing expAtt1: %d" % len(refs)
    for i in refs:
      print "\tAtt: %s Item: %s " % (i.attribute().name(), i.owningItem().name())

    refs = expAtt2.references()
    print "Number of Items referencing expAtt2: %d" % len(refs)
    for i in refs:
      print "\tAtt: %s Item: %s " % (i.attribute().name(), i.owningItem().name())

    system.removeAttribute(att1)
    del att1 #Should delete att1
    print "testAtt1 deleted"
    refs = expAtt1.references()
    print "Number of Items referencing expAtt1: %d" % len(refs)
    for i in refs:
      print "\tAtt: %s Item: %s " % (i.attribute().name(), i.owningItem().name())

    refs = expAtt2.references()
    print "Number of Items referencing expAtt2: %d" % len(refs)
    for i in refs:
      print "\tAtt: %s Item: %s " % (i.attribute().name(), i.owningItem().name())

    smtk.attribute.ValueItem.CastTo(att2.item(0)).setExpression(expAtt1)
    print "testAtt3 now using Exp2"

    refs = expAtt1.references()
    print "Number of Items referencing expAtt1: %d" % len(refs)
    for i in refs:
      print "\tAtt: %s Item: %s " % (i.attribute().name(), i.owningItem().name())

    refs = expAtt2.references()
    print "Number of Items referencing expAtt2: %d" % len(refs)
    for i in refs:
      print "\tAtt: %s Item: %s " % (i.attribute().name(), i.owningItem().name())

    del system
    print 'System destroyed'

    sys.exit(status)
