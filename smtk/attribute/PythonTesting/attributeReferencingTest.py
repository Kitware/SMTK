"""
Manual port of SMTK/smtk/attribute/Testing/attributeReferencingTest.cxx
For verifying python-shiboken wrappers

Requires SMTKCorePython.so to be in module path
"""

import smtk

if __name__ == '__main__':
    import sys

    status = 0
    
    manager = smtk.attribute.Manager()
    print 'Manager created'
    
    #Lets create an attribute to represent an expression
    expDef = manager.createDefinition("ExpDef")
    eitemdef =  expDef.addItemDefinitionStr(smtk.attribute.StringItemDefinition,"Expression String")
    eitemdef2 = expDef.addItemDefinitionStr(smtk.attribute.StringItemDefinition,"Aux String")
    eitemdef.setDefaultValue("sample");
    print eitemdef.name()
    print eitemdef2.name()
    
    base = manager.createDefinition("BaseDef");

    #Lets add some item definitions
    ditemdef = base.addItemDefinitionStr(smtk.attribute.DoubleItemDefinition, "DoubleItem1");
    # Allow this one to hold an expression
    ditemdef.setExpressionDefinition(expDef);
    
    # Lets test creating an attribute by passing in the expression definition explicitly
    expAtt1 = manager.createAttribute("Exp1", expDef);
    expAtt2 = manager.createAttribute("Exp2", expDef);
    att  = manager.createAttribute("testAtt1", "BaseDef");
    att1 = manager.createAttribute("testAtt2", "BaseDef");
    att2 = manager.createAttribute("testAtt3", "BaseDef");
    
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
      
    manager.removeAttribute(att1)
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

    del manager
    print 'Manager destroyed'

    sys.exit(status)
