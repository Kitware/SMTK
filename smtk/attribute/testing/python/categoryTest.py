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
Manual port of SMTK/smtk/attribute/Testing/categoryTest.cxx
For verifying python wrappers

Requires smtkCorePython.so to be in module path
"""

import smtk
from smtk import attribute

if __name__ == '__main__':
    import sys
    import os

    status = 0

    resource = smtk.attribute.Resource.create()
    print('Resource created')

    expDef = resource.createDefinition("ExpDef")
    expDef.setBriefDescription("Sample Expression")
    expDef.setDetailedDescription(
        "Sample Expression for testing\nThere is not much here!")
    eitemdef = smtk.attribute.StringItemDefinition.New("Expression String")
    expDef.addItemDefinition(eitemdef)
    eitemdef2 = smtk.attribute.StringItemDefinition.New("Aux String")
    expDef.addItemDefinition(eitemdef2)
    eitemdef.setDefaultValue("sample")

    base = resource.createDefinition("BaseDef")
    # Lets add some item definitions
    iitemdef = smtk.attribute.IntItemDefinition.New("IntItem1")
    base.addItemDefinition(iitemdef)
    iitemdef.localCategories().insertInclusion("Flow")
    iitemdef = smtk.attribute.IntItemDefinition.New("IntItem2")
    base.addItemDefinition(iitemdef)
    iitemdef.setDefaultValue(10)
    iitemdef.localCategories().insertInclusion("Heat")

    def1 = resource.createDefinition("Derived1", "BaseDef")
    # Lets add some item definitions
    ditemdef = smtk.attribute.DoubleItemDefinition.New("DoubleItem1")
    def1.addItemDefinition(ditemdef)
    # Allow this one to hold an expression
    ditemdef.localCategories().insertInclusion("Veg")
    ditemdef.setExpressionDefinition(expDef)
    # Check to make sure we can use expressions
    if not ditemdef.allowsExpressions():
        print("ERROR - Item Def does not allow expressions")
        status = -1
    ditemdef = smtk.attribute.DoubleItemDefinition.New("DoubleItem2")
    def1.addItemDefinition(ditemdef)
    ditemdef.setDefaultValue(-35.2)
    ditemdef.localCategories().insertInclusion("Constituent")

    def2 = resource.createDefinition("Derived2", "Derived1")
    # Lets add some item definitions
    sitemdef = smtk.attribute.StringItemDefinition.New("StringItem1")
    def2.addItemDefinition(sitemdef)
    sitemdef.localCategories().insertInclusion("Flow")
    sitemdef = smtk.attribute.StringItemDefinition.New("StringItem2")
    def2.addItemDefinition(sitemdef)
    sitemdef.setDefaultValue("Default")
    sitemdef.localCategories().insertInclusion("General")

    # Process Definition Information
    resource.finalizeDefinitions()
    # Lets see what categories the attribute definitions think they are
    if expDef.categories().size() != 0:
        print("ERROR: ExpDef has categories")
    else:
        print("ExpDef has no categories")
    if def2.categories().size() != 0:
        print("Def2 has categories")
    else:
        print("ERROR: Def2 has no categories!")
    # Lets test creating an attribute by passing in the expression definition
    # explicitly
    expAtt = resource.createAttribute("Exp1", expDef)
    att = resource.createAttribute("testAtt", "Derived2")
    if att is not None:
        print("Attribute testAtt created")
    else:
        print("ERROR: Attribute testAtt not created")
        status = -1

    # Find the expression enabled item
    item = att.item(2)
    vitem = smtk.attribute.ValueItem.CastTo(item)
    if vitem.allowsExpressions():
        vitem.setExpression(expAtt)
        print("Expression Set on %s" % vitem.name())
    else:
        print("ERROR: Can not set expression on %s" % vitem.name())
        status = -1

    n = att.numberOfItems()
    print("Items of testAtt:\n")
    for i in range(0, n, 1):
        item = att.item(i)
        print("\t %s Type = %s" %
              (item.name(), smtk.attribute.Item.type2String(item.type())))
        vitem = smtk.attribute.ValueItem.CastTo(item)
        if vitem is not None:
            if vitem.isExpression():
                print("\t\tusing Expression: %s" % vitem.expression().name())
            else:
                print("\t\tValue = %s" % vitem.valueAsString())
#    writer = smtk.attribute.XmlV1StringWriter(resource)
#    print(writer.convertToString())

    del resource
    print('Resource destroyed')

    sys.exit(status)
