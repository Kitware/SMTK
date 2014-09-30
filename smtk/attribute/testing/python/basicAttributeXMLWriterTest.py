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
Manual port of SMTK/smtk/attribute/Testing/basicAttributeXMLWriterTest.cxx
For verifying python-shiboken wrappers

Requires SMTKCorePython.so to be in module path
"""

import smtk

if __name__ == '__main__':
    import sys

    status = 0

    if len(sys.argv) != 2:
      print 'Usage: %s filename' % sys.argv[0]
      sys.exit(-1)

    manager = smtk.attribute.Manager()
    print 'Manager created'
    # Let's add some analyses
    analysis = set()
    analysis.add('Flow')
    analysis.add('General')
    analysis.add('Time')
    # Note pass analysis in as list, not set
    manager.defineAnalysis('CFD Flow', list(analysis))
    analysis.clear()

    analysis.add('Flow')
    analysis.add('Heat')
    analysis.add('General')
    analysis.add('Time')
    manager.defineAnalysis('CFD Flow with Heat Transfer', list(analysis))
    analysis.clear()

    analysis.add('Constituent')
    analysis.add('General')
    analysis.add('Time')
    manager.defineAnalysis('Constituent Transport', list(analysis))
    analysis.clear()

    # Lets create an attribute to represent an expression
    expDef = manager.createDefinition('ExpDef')
    expDef.setBriefDescription('Sample Expression')
    expDef.setDetailedDescription('Sample Expression for testing\nThere is not much here!')
    eitemdef = expDef.addItemDefinitionStr( smtk.attribute.StringItemDefinition, 'Expression String')
    eitemdef2 = expDef.addItemDefinitionStr( smtk.attribute.StringItemDefinition, 'Aux String')
    eitemdef.setDefaultValue('sample')
    eitemdef3 = expDef.addItemDefinitionStr( smtk.attribute.ModelEntityItemDefinition, 'ModelEntity Expression')
    eitemdef4 = expDef.addItemDefinitionStr( smtk.attribute.ModelEntityItemDefinition, 'Aux String')

    base = manager.createDefinition('BaseDef')
    # Lets add some item definitions
    iitemdef = base.addItemDefinitionStr(smtk.attribute.IntItemDefinition, 'TEMPORAL')
    iitemdef.setCommonValueLabel('Time')
    iitemdef.addDiscreteValue(0, 'Seconds')
    iitemdef.addDiscreteValue(1, 'Minutes')
    iitemdef.addDiscreteValue(2, 'Hours')
    iitemdef.addDiscreteValue(3, 'Days')
    iitemdef.setDefaultDiscreteIndex(0)
    iitemdef.addCategory('Time')
    iitemdef = base.addItemDefinitionStr(smtk.attribute.IntItemDefinition, 'IntItem2')
    iitemdef.setDefaultValue(10);
    iitemdef.addCategory('Heat');

    def1 = manager.createDefinition('Derived1', 'BaseDef')
    def1.setAssociationMask(smtk.model.MODEL_DOMAIN) # belongs on model
    # Lets add some item definitions
    ditemdef = def1.addItemDefinitionStr(smtk.attribute.DoubleItemDefinition, 'DoubleItem1')
    # Allow this one to hold an expression
    ditemdef.addCategory('Veg')
    ditemdef.setExpressionDefinition(expDef)
    # Check to make sure we can use expressions
    if not ditemdef.allowsExpressions():
      print 'ERROR - Item Def does not allow expressions'
      status = -1
    ditemdef = def1.addItemDefinitionStr(smtk.attribute.DoubleItemDefinition, 'DoubleItem2')
    ditemdef.setDefaultValue(-35.2)
    ditemdef.setMinRange(-100, True)
    ditemdef.setMaxRange(125.0, False)
    ditemdef.addCategory('Constituent')
    vdef = def1.addItemDefinitionStr(smtk.attribute.VoidItemDefinition, 'VoidItem')
    vdef.setIsOptional(True)
    vdef.setLabel('Option 1')

    def2 = manager.createDefinition('Derived2', 'Derived1')
    def2.setAssociationMask(smtk.model.VOLUME)
    # Lets add some item definitions
    sitemdef = def2.addItemDefinitionStr( smtk.attribute.StringItemDefinition, 'StringItem1' )
    sitemdef.setIsMultiline(True)
    sitemdef.addCategory('Flow')
    sitemdef = def2.addItemDefinitionStr( smtk.attribute.StringItemDefinition, 'StringItem2' )
    sitemdef.setDefaultValue('Default')
    sitemdef.addCategory('General')
    uitemdef = def2.addItemDefinitionStr( smtk.attribute.ModelEntityItemDefinition, 'ModelEntityItem1' )
    uitemdef.addCategory('Flow')
    uitemdef.setMembershipMask(smtk.model.FACE);
    uitemdef = def2.addItemDefinitionStr( smtk.attribute.ModelEntityItemDefinition, 'ModelEntityItem2' )
    uitemdef.addCategory('General')
    uitemdef.setMembershipMask(smtk.model.GROUP_ENTITY | smtk.model.HOMOGENOUS_GROUP);
    dirdef = def2.addItemDefinitionStr( smtk.attribute.DirectoryItemDefinition, 'DirectoryItem')
    dirdef.setShouldExist(True)
    dirdef.setShouldBeRelative(True)
    fdef = def2.addItemDefinitionStr( smtk.attribute.FileItemDefinition, 'FileItem' )
    fdef.setShouldBeRelative(True);
    gdef = def2.addItemDefinitionStr( smtk.attribute.GroupItemDefinition, 'GroupItem' )
    gdef.addItemDefinitionStr( smtk.attribute.FileItemDefinition, 'File1' )
    gdef1 = gdef.addItemDefinitionStr( smtk.attribute.GroupItemDefinition, 'SubGroup')
    sitemdef = gdef1.addItemDefinitionStr( smtk.attribute.StringItemDefinition, 'GroupString')
    sitemdef.setDefaultValue('Something Cool')
    sitemdef.addCategory('General')
    sitemdef.addCategory('Flow')

    # Add in a Attribute definition with a reference to another attribute
    attrefdef = manager.createDefinition('AttributeReferenceDef')
    aritemdef = attrefdef.addItemDefinitionStr( smtk.attribute.RefItemDefinition, 'BaseDefItem' )
    aritemdef.setCommonValueLabel('A reference to another attribute')
    aritemdef.setAttributeDefinition(base)

    # Process Categories
    manager.updateCategories()
    # Lets test creating an attribute by passing in the expression definition explicitly
    expAtt = manager.createAttribute('Exp1', expDef)
    att = manager.createAttribute('testAtt', 'Derived2')
    if att is None:
      print 'ERROR: Attribute testAtt not created'
      status = -1

    #Find the expression enabled item
    item = att.item(2)
    vitem = smtk.attribute.ValueItem.CastTo(item)
    writer = smtk.io.AttributeWriter()
    logger = smtk.io.Logger()
    if writer.write(manager, sys.argv[1], logger):
        sys.stderr.write('Errors encountered creating Attribute File:\n')
        sys.stderr.write(logger.convertToString())
        sys.stderr.write('\n')
        status = -1

    del manager
    print 'Manager destroyed'

    sys.exit(status)
