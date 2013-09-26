"""
Manual port of SMTK/smtk/attribute/Testing/basicAttributeDerivationTest.cxx
For verifying python-shiboken wrappers

Requires SMTKCorePython.so to be in module path
"""

import smtk

if __name__ == '__main__':
    import sys

    status = 0

    manager = smtk.attribute.Manager()
    print 'Manager created'
    base = manager.createDefinition("BaseDef");

    #Lets add some item definitions
    icompdef = smtk.attribute.IntItemDefinition.New('IntComp1')
    itemdef = smtk.attribute.IntItemDefinition.ToItemDefinition(icompdef)
    base.addItemDefinition(itemdef);
    icompdef2 = smtk.attribute.IntItemDefinition.New('IntComp2')
    icompdef2.setDefaultValue(10);
    itemdef2 = smtk.attribute.IntItemDefinition.ToItemDefinition(icompdef2)
    base.addItemDefinition(itemdef2);

    def1 = manager.createDefinition('Derived1', 'BaseDef')
    # Lets add some item definitions
    dcompdef = smtk.attribute.DoubleItemDefinition.New('DoubleComp1')
    itemdef = smtk.attribute.DoubleItemDefinition.ToItemDefinition(dcompdef)
    def1.addItemDefinition(itemdef)
    dcompdef2 = smtk.attribute.DoubleItemDefinition.New('DoubleComp2')
    dcompdef2.setDefaultValue(-35.2)
    itemdef2 = smtk.attribute.DoubleItemDefinition.ToItemDefinition(dcompdef2)
    def1.addItemDefinition(itemdef2)

    def2 = manager.createDefinition('Derived2', 'Derived1')
    # Lets add some item definitions
    scompdef = smtk.attribute.StringItemDefinition.New('StringComp1')
    itemdef = smtk.attribute.StringItemDefinition.ToItemDefinition(scompdef)
    def1.addItemDefinition(itemdef)
    scompdef2 = smtk.attribute.StringItemDefinition.New('StringComp2')
    scompdef2.setDefaultValue('Default')
    itemdef2 = smtk.attribute.StringItemDefinition.ToItemDefinition(scompdef2)
    def1.addItemDefinition(itemdef2)

    att = manager.createAttribute('testAtt', 'Derived2')
    if not att is None:
        print 'Attribute testAtt created'
    else:
        print 'ERROR: Attribute testAtt not created'
        status = -1

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
