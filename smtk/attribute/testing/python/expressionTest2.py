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
  Manual port of SMTK/smtk/attribute/Testing/expressionTest.cxx
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

    # Lets create an attribute to represent an expression
    expDef = system.createDefinition("ExpDef")
    eitemdef = smtk.attribute.StringItemDefinition.New("Expression String")
    expDef.addItemDefinition(eitemdef)
    eitemdef2 = smtk.attribute.StringItemDefinition.New("Aux String")
    expDef.addItemDefinition(eitemdef2)
    eitemdef.setDefaultValue("sample")

    base = system.createDefinition("BaseDef")
    # Lets add some item definitions
    iitemdef = smtk.attribute.IntItemDefinition.New("IntItem1")
    base.addItemDefinition(iitemdef)
    iitemdef = smtk.attribute.IntItemDefinition.New("IntItem2")
    base.addItemDefinition(iitemdef)
    iitemdef.setDefaultValue(10)

    def1 = system.createDefinition("Derived1", "BaseDef")
    # add some item definitions
    ditemdef = smtk.attribute.DoubleItemDefinition.New("DoubleItem1")
    def1.addItemDefinition(ditemdef)
    # Allow this one to hold an expression
    ditemdef.setExpressionDefinition(expDef)
    # Check to make sure we can use expressions
    if not ditemdef.allowsExpressions():
        print "ERROR - Item Def does not allow expressions"
        status = -1
    ditemdef = smtk.attribute.DoubleItemDefinition.New("DoubleItem2")
    def1.addItemDefinition(ditemdef)
    ditemdef.setDefaultValue(-35.2)

    def2 = system.createDefinition("Derived2", "Derived1")
    # Lets add some item definitions
    sitemdef = smtk.attribute.StringItemDefinition.New("StringItem1")
    def2.addItemDefinition(sitemdef)
    sitemdef = smtk.attribute.StringItemDefinition.New("StringItem2")
    def2.addItemDefinition(sitemdef)
    sitemdef.setDefaultValue("Default")

    # Lets test creating an attribute by passing in the expression definition
    # explicitly
    expAtt = system.createAttribute("Exp1", expDef)
    att = system.createAttribute("testAtt", "Derived2")
    if att is not None:
        print "Attribute testAtt created"
    else:
        print "ERROR: Attribute testAtt not created"
        status = -1

    # Find the expression enabled item
    vitem = smtk.attribute.ValueItem.CastTo(att.item(2))
    if vitem.allowsExpressions():
        vitem.setExpression(expAtt)
        print "Expression Set on %s" % vitem.name()
    else:
        print "ERROR: Can not set expression on %s" % vitem.name()
        status = -1

    n = att.numberOfItems()
    print "Items of testAtt:"
    for i in range(0, n, 1):
        item = att.item(i)
        sys.stdout.write("\t%s Type = %s, " % (
            item.name(), smtk.attribute.Item.type2String(item.type())))
        vitem = smtk.attribute.ValueItem.CastTo(item)
        if vitem is not None:
            if vitem.isExpression(0):
                print " using Expression: %s" % vitem.expression(0).name()
            else:
                print " Value = %s" % vitem.valueAsString()

    del system
    print 'System destroyed'

    sys.exit(status)
