"""
Manual port of SMTK/smtk/attribute/Testing/basicAttributeDerivationTest.cxx
For verifying python-shiboken wrappers

Requires SMTKCorePython.so to be in module path
"""

import smtk

itemNames = ["IntComp1", "IntComp2", "DoubleComp1",
             "DoubleComp2", "StringComp1", "StringComp2"]

if __name__ == '__main__':
    import sys

    status = 0

    manager = smtk.attribute.Manager()
    print 'Manager created'
    base = manager.createDefinition("BaseDef");

    #Lets add some item definitions
    icompdef = smtk.attribute.IntItemDefinition.New(itemNames[0])
    itemdef = smtk.attribute.IntItemDefinition.ToItemDefinition(icompdef)
    base.addItemDefinition(itemdef);
    icompdef2 = smtk.attribute.IntItemDefinition.New(itemNames[1])
    icompdef2.setDefaultValue(10);
    itemdef2 = smtk.attribute.IntItemDefinition.ToItemDefinition(icompdef2)
    base.addItemDefinition(itemdef2);

    def1 = manager.createDefinition('Derived1', 'BaseDef')
    # Lets add some item definitions
    dcompdef = smtk.attribute.DoubleItemDefinition.New(itemNames[2])
    itemdef = smtk.attribute.DoubleItemDefinition.ToItemDefinition(dcompdef)
    def1.addItemDefinition(itemdef)
    dcompdef2 = smtk.attribute.DoubleItemDefinition.New(itemNames[3])
    dcompdef2.setDefaultValue(-35.2)
    itemdef2 = smtk.attribute.DoubleItemDefinition.ToItemDefinition(dcompdef2)
    def1.addItemDefinition(itemdef2)

    def2 = manager.createDefinition('Derived2', 'Derived1')
    # Lets add some item definitions
    scompdef = smtk.attribute.StringItemDefinition.New(itemNames[4])
    itemdef = smtk.attribute.StringItemDefinition.ToItemDefinition(scompdef)
    def1.addItemDefinition(itemdef)
    scompdef2 = smtk.attribute.StringItemDefinition.New(itemNames[5])
    scompdef2.setDefaultValue('Default')
    itemdef2 = smtk.attribute.StringItemDefinition.ToItemDefinition(scompdef2)
    def1.addItemDefinition(itemdef2)

    # Lets test out the find item position method
    pstatus = 0;
    for j in range(0,6):
      if (def2.findItemPosition(itemNames[j]) != j):
        print "Incorrect Position Returned for %s, position returned is %d, but it should be %d\n" % (itemNames[j], def2.findItemPosition(itemNames[j]), j)
        pstatus = -1
    if pstatus == 0:
      print "Initial Position Test Passed!\n"
    else:
      print "Initial Position Test Failed!\n"
      status = -1

    # Lets add a  component to the base def and verify that positions are reordered
    base.addItemDefinitionStr(smtk.attribute.StringItemDefinition, "InsertStringItem")
    pstatus = 0
    for j in range(2,6):
      if def2.findItemPosition(itemNames[j]) != (j+1):
        print "Incorrect Position Returned for %s, position returned is %d, but it should be %d\n" % (itemNames[j], def2.findItemPosition(itemNames[j]), j+1)
        pstatus = -1;

    if pstatus == 0:
      print "Insertion Position Test Passed!\n"
    else:
      print "Insertion Position Test Failed!\n"
      status = -1

    att = manager.createAttribute('testAtt', 'Derived2')
    if not att is None:
        print 'Attribute testAtt created'
    else:
        print 'ERROR: Attribute testAtt not created'
        status = -1
    comp = att.find("DoubleComp1")
    if not comp is None:
       vvcomp = smtk.attribute.ValueItem.CastTo(comp)
       print " Value = %s"%vvcomp.valueAsString()
    else:
       print "ERROR: could not find the base's item"
       status = -1;

    n = att.numberOfItems()
    for i in range(n):
        comp = att.item(i)
        print '\t%s Type = %s, ' % (comp.name(), \
          smtk.attribute.Item.type2String(comp.type())),
        vcomp = smtk.attribute.ValueItem.CastTo(comp)
        if vcomp.type() == smtk.attribute.Item.DOUBLE:
          print ' Value = %s' % vcomp.valueAsString()
        elif vcomp.type() == smtk.attribute.Item.INT:
          print ' Value = %s' % vcomp.valueAsString()
        elif vcomp.type() == smtk.attribute.Item.STRING:
          print 'String Val = %s' % vcomp.valueAsString()

    del manager
    print 'Manager destroyed'

    sys.exit(status)
