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

    Requires SMTKCorePython.so to be in module path
    """

import smtk

if __name__ == '__main__':
    import sys

    status = 0

    system = smtk.attribute.System()
    print 'System created'
    # Lets create an attribute to represent an expression
    expDef = system.createDefinition("ExpDef")
    eitemdef = expDef.addItemDefinitionStr(smtk.attribute.StringItemDefinition, "Expression String")

    base = system.createDefinition("BaseDef");
    # Lets add some item definitions
    iitemdef = base.addItemDefinitionStr(smtk.attribute.IntItemDefinition, "IntItem1")
    iitemdef2 = base.addItemDefinitionStr(smtk.attribute.IntItemDefinition, "IntItem2")
    iitemdef2.setDefaultValue(10)

    def1 = system.createDefinition("Derived1", "BaseDef");
    # Lets add some item definitions
    ditemdef = def1.addItemDefinitionStr(smtk.attribute.DoubleItemDefinition, "DoubleItem1");
    # Allow this one to hold an expression
    ditemdef.setExpressionDefinition(expDef);
    # Check to make sure we can use expressions
    if not ditemdef.allowsExpressions():
        print "ERROR - Item Def does not allow expressions"
        status = -1;
    ditemdef2 = def1.addItemDefinitionStr(smtk.attribute.DoubleItemDefinition, "DoubleItem2");
    ditemdef2.setDefaultValue(-35.2);

    def2 = system.createDefinition("Derived2", "Derived1");
    # Lets add some item definitions
    sitemdef = def1.addItemDefinitionStr(smtk.attribute.StringItemDefinition, "StringItem1")
    sitemdef2 = def1.addItemDefinitionStr(smtk.attribute.StringItemDefinition, "StringItem2");
    sitemdef2.setDefaultValue("Default");

    #Lets test creating an attribute by passing in the expression definition explicitly
    expAtt = system.createAttribute("Exp1", expDef);
    att = system.createAttribute("testAtt", "Derived2");
    if att is not None:
        print "Attribute testAtt created";
    else:
        print "ERROR: Attribute testAtt not created";
        status = -1;

    # Find the expression enabled item
    vitem = smtk.attribute.ValueItem.CastTo(att.item(2))
    if vitem.allowsExpressions():
        vitem.setExpression(expAtt);
        print "Expression Set on %s"% vitem.name()
    else:
        print "ERROR: Can not set expression on %s"%vitem.name()
        status = -1;

    n = att.numberOfItems();
    print "Items of testAtt:";
    for i in range(0,n,1):
        item = att.item(i);
        sys.stdout.write( "\t%s Type = %s, " % (item.name(), smtk.attribute.Item.type2String(item.type())));
        vitem = smtk.attribute.ValueItem.CastTo(item);
        if vitem is not None:
            if vitem.isExpression(0):
                print " using Expression: %s" % vitem.expression(0).name();
            else:
                t = vitem.type()
                if t == smtk.attribute.Item.DOUBLE or t == smtk.attribute.Item.INT:
                     print " Value = %s" % vitem.valueAsString();
                elif t == smtk.attribute.Item.STRING:
                     print " String Val = %s" % vitem.valueAsString();

    del system
    print 'System destroyed'

    sys.exit(status)
