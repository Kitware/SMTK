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
Manual port of SMTK/smtk/attribute/Testing/basicAttributeDefinitionTest.cxx
For verifying python-shiboken wrappers

Requires SMTKCorePython.so to be in module path
"""

import smtk

if __name__ == '__main__':
    import sys

    status = 0

    manager = smtk.attribute.Manager()
    print 'Manager created'
    def_ = manager.createDefinition('testDef')
    if def_ is not None:
        print 'Definition testDef created'
    else:
        print 'ERROR: Definition testDef not created'
        status = -1

    # Lets add some item definitions
    icompdef = smtk.attribute.IntItemDefinition.New('IntComp1')
    # Must explicitly cast from ItemDefinition in order to add to Definition
    # Use ToItemDefinition() method that was added to typesystem
    itemdef = smtk.attribute.IntItemDefinition.ToItemDefinition(icompdef)
    def_.addItemDefinition(itemdef)

    icompdef2 = smtk.attribute.IntItemDefinition.New('IntComp2')
    icompdef2.setDefaultValue(10);
    itemdef2 = smtk.attribute.IntItemDefinition.ToItemDefinition(icompdef2)
    def_.addItemDefinition(itemdef2);
    def1 = manager.createDefinition("testDef");
    if def1 is None:
        print 'Duplicated definition testDef not created'
    else:
        print 'ERROR: Duplicated definition testDef created'
        status = -1

    att = manager.createAttribute('testAtt', 'testDef')
    if not att is None:
        print 'Attribute testAtt created'
    else:
        print 'ERROR: Attribute testAtt not created'
        status = -1

    if att.numberOfItems() != 2:
        print 'ERROR: attribute has %d items not 2' % att.numberOfItems()
        status = -1
    elif att.item(0).name() != 'IntComp1':
        print 'ERROR: Attribute\'s 0th item is named %s not IntComp1' % att.item(0).name()
        status = -1
    elif att.item(1).name() != 'IntComp2':
        print 'ERROR: Attribute\'s 1st item is named %s not IntComp2' % att.item(0).name()
        status = -1
    else:
        # Use CastTo method to get IntItem from Item
        int_item = smtk.attribute.IntItem.CastTo(att.item(0))
        print 'Found IntComp1 - value = %s' % int_item.valueAsString()
        int_item = smtk.attribute.IntItem.CastTo(att.item(1))
        print 'Found IntComp2 - value = %s' % int_item.valueAsString()

    att1 = manager.createAttribute('testAtt', 'testDef')
    if att1 is None:
        print 'Duplicate Attribute testAtt not created'
    else:
        print 'ERROR: Duplicate Attribute testAtt  created'
        status = -1

    del manager
    print 'Manager destroyed'

    sys.exit(status)
