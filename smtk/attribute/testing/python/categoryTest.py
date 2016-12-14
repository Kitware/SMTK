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
Manual port of SMTK/smtk/attribute/Testing/categoryTest.cxx
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

    expDef = system.createDefinition("ExpDef")
    expDef.setBriefDescription("Sample Expression")
    expDef.setDetailedDescription("Sample Expression for testing\nThere is not much here!")
    eitemdef = smtk.attribute.StringItemDefinition.New("Expression String")
    expDef.addItemDefinition(eitemdef)
    eitemdef2 = smtk.attribute.StringItemDefinition.New("Aux String")
    expDef.addItemDefinition(eitemdef2)
    eitemdef.setDefaultValue("sample")

    base = system.createDefinition("BaseDef")
    #Lets add some item definitions
    iitemdef = smtk.attribute.IntItemDefinition.New("IntItem1")
    base.addItemDefinition(iitemdef)
    iitemdef.addCategory("Flow")
    iitemdef = smtk.attribute.IntItemDefinition.New("IntItem2")
    base.addItemDefinition(iitemdef)
    iitemdef.setDefaultValue(10)
    iitemdef.addCategory("Heat")

    def1 = system.createDefinition("Derived1", "BaseDef")
    # Lets add some item definitions
    ditemdef = smtk.attribute.DoubleItemDefinition.New("DoubleItem1")
    def1.addItemDefinition(ditemdef)
    # Allow this one to hold an expression
    ditemdef.addCategory("Veg")
    ditemdef.setExpressionDefinition(expDef)
    # Check to make sure we can use expressions
    if not ditemdef.allowsExpressions():
      print "ERROR - Item Def does not allow expressions"
      status = -1
    ditemdef = smtk.attribute.DoubleItemDefinition.New("DoubleItem2")
    def1.addItemDefinition(ditemdef)
    ditemdef.setDefaultValue(-35.2)
    ditemdef.addCategory("Constituent")

    def2 = system.createDefinition("Derived2", "Derived1")
    # Lets add some item definitions
    sitemdef = smtk.attribute.StringItemDefinition.New("StringItem1")
    def2.addItemDefinition(sitemdef)
    sitemdef.addCategory("Flow")
    sitemdef = smtk.attribute.StringItemDefinition.New("StringItem2")
    def2.addItemDefinition(sitemdef)
    sitemdef.setDefaultValue("Default")
    sitemdef.addCategory("General")

    # Process Categories
    system.updateCategories()
    # Lets see what categories the attribute definitions think they are
    if expDef.numberOfCategories() != 0:
      categories = expDef.categories()
      print "ERROR: ExpDef's categories: "
      for it in categories:
        print "\t \"%s\""%it
    else:
      print "ExpDef has no categories"
    if def2.numberOfCategories() != 0:
      categories = def2.categories()
      print "Def2's categories: "
      for it in categories:
        print "\t \"%s\""%it
    else:
      print "ERROR: Def2 has no categories!"
    # Lets test creating an attribute by passing in the expression definition explicitly
    expAtt = system.createAttribute("Exp1", expDef)
    att = system.createAttribute("testAtt", "Derived2")
    if att is not None:
      print "Attribute testAtt created"
    else:
      print "ERROR: Attribute testAtt not created"
      status = -1

    # Find the expression enabled item
    item = att.item(2)
    vitem = smtk.attribute.ValueItem.CastTo(item)
    if vitem.allowsExpressions():
      vitem.setExpression(expAtt)
      print "Expression Set on %s"%vitem.name()
    else:
      print "ERROR: Can not set expression on %s"%vitem.name()
      status = -1

    n = att.numberOfItems()
    print "Items of testAtt:\n"
    for i in range(0,n,1):
      item = att.item(i)
      print "\t %s Type = %s"%(item.name(), smtk.attribute.Item.type2String(item.type()))
      vitem = smtk.attribute.ValueItem.CastTo(item)
      if vitem is not None:
        if vitem.isExpression(0):
          print "\t\tusing Expression: %s"%vitem.expression(0).name()
        else:
          print "\t\tValue = %s"%vitem.valueAsString()
#    writer = smtk.attribute.XmlV1StringWriter(system)
#    print writer.convertToString()

    del system
    print 'System destroyed'

    sys.exit(status)
