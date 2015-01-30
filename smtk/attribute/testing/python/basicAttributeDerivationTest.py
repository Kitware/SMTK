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
Manual port of SMTK/smtk/attribute/Testing/basicAttributeDerivationTest.cxx
For verifying python-shiboken wrappers

Requires SMTKCorePython.so to be in module path
"""

import smtk

itemNames = ["IntComp1", "IntComp2", "DoubleComp1",
             "DoubleComp2", "StringComp1", "StringComp2",
             "ModelComp1_FACE", "ModelComp2_GROUP"]

if __name__ == '__main__':
    import sys

    status = 0

    system = smtk.attribute.System()
    print 'System created'
    base = system.createDefinition("BaseDef");

    #Lets add some item definitions
    icompdef = smtk.attribute.IntItemDefinition.New(itemNames[0])
    itemdef = smtk.attribute.IntItemDefinition.ToItemDefinition(icompdef)
    base.addItemDefinition(itemdef);
    icompdef2 = smtk.attribute.IntItemDefinition.New(itemNames[1])
    icompdef2.setDefaultValue(10);
    itemdef2 = smtk.attribute.IntItemDefinition.ToItemDefinition(icompdef2)
    base.addItemDefinition(itemdef2);

    def1 = system.createDefinition('Derived1', 'BaseDef')
    # Lets add some item definitions
    dcompdef = smtk.attribute.DoubleItemDefinition.New(itemNames[2])
    itemdef = smtk.attribute.DoubleItemDefinition.ToItemDefinition(dcompdef)
    def1.addItemDefinition(itemdef)
    dcompdef2 = smtk.attribute.DoubleItemDefinition.New(itemNames[3])
    dcompdef2.setDefaultValue(-35.2)
    itemdef2 = smtk.attribute.DoubleItemDefinition.ToItemDefinition(dcompdef2)
    def1.addItemDefinition(itemdef2)

    def2 = system.createDefinition('Derived2', 'Derived1')
    # Lets add some item definitions
    scompdef = smtk.attribute.StringItemDefinition.New(itemNames[4])
    itemdef = smtk.attribute.StringItemDefinition.ToItemDefinition(scompdef)
    def2.addItemDefinition(itemdef)
    scompdef2 = smtk.attribute.StringItemDefinition.New(itemNames[5])
    scompdef2.setDefaultValue('Default')
    itemdef2 = smtk.attribute.StringItemDefinition.ToItemDefinition(scompdef2)
    def2.addItemDefinition(itemdef2)

    def3 = system.createDefinition('Derived3', 'Derived1')
    # Lets add some item definitions
    scompdef = smtk.attribute.ModelEntityItemDefinition.New(itemNames[6])
    scompdef.setMembershipMask(smtk.model.FACE)
    scompdef.setIsExtensible(True) # Need to set this or numberOfRequiredValues
    itemdef = smtk.attribute.ModelEntityItemDefinition.ToItemDefinition(scompdef)
    def3.addItemDefinition(itemdef)
    scompdef3 = smtk.attribute.ModelEntityItemDefinition.New(itemNames[7])
    scompdef3.setMembershipMask(smtk.model.CELL_ENTITY | smtk.model.GROUP_ENTITY | smtk.model.HOMOGENOUS_GROUP)
    scompdef3.setIsExtensible(True) # Need to set this or numberOfRequiredValues
    itemdef3 = smtk.attribute.ModelEntityItemDefinition.ToItemDefinition(scompdef3)
    def3.addItemDefinition(itemdef3)

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

    att = system.createAttribute('testAtt', 'Derived2')
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
        if vcomp:
          if vcomp.type() == smtk.attribute.Item.DOUBLE:
            print ' Value = %s' % vcomp.valueAsString()
          elif vcomp.type() == smtk.attribute.Item.INT:
            print ' Value = %s' % vcomp.valueAsString()
          elif vcomp.type() == smtk.attribute.Item.STRING:
            print 'String Val = %s' % vcomp.valueAsString()
        else:
          ecomp = smtk.attribute.ModelEntityItem.CastTo(comp)
          if ecomp:
            if ecomp.type() == smtk.attribute.Item.MODEL_ENTITY:
              print ' Value = %s' % ecomp.valueAsString()
          else:
            print comp

    # ======
    # Now test setting entity-valued attribute-items
    # I. Create a model manager and add some entities to it:
    mmgr = smtk.model.Manager.create()
    system.setRefModelManager(mmgr)
    mdl = mmgr.addModel(3,3,'TestModel')
    edg = mmgr.addEdge()
    fac = mmgr.addFace()
    gr0 = mmgr.addGroup(smtk.model.HOMOGENOUS_GROUP, 'GroupA')
    gr1 = mmgr.addGroup(smtk.model.PARTITION, 'GroupB')
    gr0.addEntity(fac)
    gr1.addEntity(edg)

    # II. Have the attribute system create an attribute with model-entity
    att = system.createAttribute('testMEAtt', 'Derived3')
    if not att is None:
        print 'Attribute testMEAtt created'
    else:
        print 'ERROR: Attribute testMEAtt not created'
        status = -1
    # III. Find the face-only attribute item and try fuzzing its arguments a bit
    comp = att.find("ModelComp1_FACE")
    if not comp is None:
       fcomp = smtk.attribute.ModelEntityItem.CastTo(comp)
       if fcomp.appendValue(edg):
         print 'ERROR: Face-only attribute value accepted an edge'
         status = -1
       if not fcomp.appendValue(fac):
         print 'ERROR: Face-only attribute value did not accept a face'
         status = -1
       print " Value = %s" % fcomp.valueAsString()
    else:
       print "ERROR: could not find the base's item"
       status = -1;
    # IV. Find the homogenous-group-only attribute item and try fuzzing its arguments a bit
    comp = att.find("ModelComp2_GROUP")
    if not comp is None:
       gcomp = smtk.attribute.ModelEntityItem.CastTo(comp)
       if not gcomp.appendValue(gr0):
         print 'ERROR: Homogenous-group-only attribute value did not accept a homogenous group'
         status = -1
       if gcomp.appendValue(gr1):
         print 'ERROR: Homogenous-group-only attribute value accepted a more relaxed group'
         status = -1
       print " Value = %s" % gcomp.valueAsString()
    else:
       print "ERROR: could not find the base's item"
       status = -1;

    n = att.numberOfItems()
    for i in range(n):
        comp = att.item(i)
        print '\t%s Type = %s, ' % (comp.name(), \
          smtk.attribute.Item.type2String(comp.type())),
        vcomp = smtk.attribute.ValueItem.CastTo(comp)
        if vcomp:
          if vcomp.type() == smtk.attribute.Item.DOUBLE:
            print ' Value = %s' % vcomp.valueAsString()
          elif vcomp.type() == smtk.attribute.Item.INT:
            print ' Value = %s' % vcomp.valueAsString()
          elif vcomp.type() == smtk.attribute.Item.STRING:
            print 'String Val = %s' % vcomp.valueAsString()
        else:
          ecomp = smtk.attribute.ModelEntityItem.CastTo(comp)
          if ecomp:
            if ecomp.type() == smtk.attribute.Item.MODEL_ENTITY:
              print ' Value = %s' % ecomp.valueAsString()
          else:
            print comp
    # ======

    del system
    print 'System destroyed'

    sys.exit(status)
