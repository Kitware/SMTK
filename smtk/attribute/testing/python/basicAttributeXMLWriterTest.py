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

Requires smtkCorePython.so to be in module path
"""

import smtk
if smtk.wrappingProtocol() == 'pybind11':
    from smtk import attribute
    from smtk import io
    from smtk import model

# print(dir(smtk.io))

if __name__ == '__main__':
    import sys

    status = 0

    if len(sys.argv) != 2:
        print 'Usage: %s filename' % sys.argv[0]
        sys.exit(-1)

    system = smtk.attribute.System()
    print 'System created'
    # Let's add some analyses
    analysis = set()
    analysis.add('Flow')
    analysis.add('General')
    analysis.add('Time')
    if smtk.wrappingProtocol() == 'pybind11':
        system.defineAnalysis('CFD Flow', analysis)
    else:
        # Note pass analysis in as list, not set
        system.defineAnalysis('CFD Flow', list(analysis))
    analysis.clear()

    analysis.add('Flow')
    analysis.add('Heat')
    analysis.add('General')
    analysis.add('Time')
    if smtk.wrappingProtocol() == 'pybind11':
        system.defineAnalysis('CFD Flow with Heat Transfer', analysis)
    else:
        system.defineAnalysis('CFD Flow with Heat Transfer', list(analysis))
    analysis.clear()

    analysis.add('Constituent')
    analysis.add('General')
    analysis.add('Time')
    if smtk.wrappingProtocol() == 'pybind11':
        system.defineAnalysis('Constituent Transport', analysis)
    else:
        system.defineAnalysis('Constituent Transport', list(analysis))
    analysis.clear()

    # Lets create an attribute to represent an expression
    expDef = system.createDefinition('ExpDef')
    expDef.setBriefDescription('Sample Expression')
    expDef.setDetailedDescription(
        'Sample Expression for testing\nThere is not much here!')
    eitemdef = smtk.attribute.StringItemDefinition.New('Expression String')
    expDef.addItemDefinition(eitemdef)
    eitemdef2 = smtk.attribute.StringItemDefinition.New('Aux String')
    expDef.addItemDefinition(eitemdef2)
    eitemdef.setDefaultValue('sample')
    eitemdef3 = smtk.attribute.ModelEntityItemDefinition.New(
        'Model Expression')
    expDef.addItemDefinition(eitemdef3)
    eitemdef4 = smtk.attribute.ModelEntityItemDefinition.New('Aux String')
    expDef.addItemDefinition(eitemdef4)

    base = system.createDefinition('BaseDef')
    # Lets add some item definitions
    iitemdef = smtk.attribute.IntItemDefinition.New('TEMPORAL')
    base.addItemDefinition(iitemdef)
    iitemdef.setCommonValueLabel('Time')
    iitemdef.addDiscreteValue(0, 'Seconds')
    iitemdef.addDiscreteValue(1, 'Minutes')
    iitemdef.addDiscreteValue(2, 'Hours')
    iitemdef.addDiscreteValue(3, 'Days')
    iitemdef.setDefaultDiscreteIndex(0)
    iitemdef.addCategory('Time')
    iitemdef = smtk.attribute.IntItemDefinition.New('IntItem2')
    base.addItemDefinition(iitemdef)
    iitemdef.setDefaultValue(10)
    iitemdef.addCategory('Heat')

    def1 = system.createDefinition('Derived1', 'BaseDef')
    def1.setAssociationMask(int(smtk.model.MODEL_DOMAIN))  # belongs on model
    # Lets add some item definitions
    ditemdef = smtk.attribute.DoubleItemDefinition.New('DoubleItem1')
    def1.addItemDefinition(ditemdef)
    # Allow this one to hold an expression
    ditemdef.addCategory('Veg')
    ditemdef.setExpressionDefinition(expDef)
    # Check to make sure we can use expressions
    if not ditemdef.allowsExpressions():
        print 'ERROR - Item Def does not allow expressions'
        status = -1
    ditemdef = smtk.attribute.DoubleItemDefinition.New('DoubleItem2')
    def1.addItemDefinition(ditemdef)
    ditemdef.setDefaultValue(-35.2)
    ditemdef.setMinRange(-100, True)
    ditemdef.setMaxRange(125.0, False)
    ditemdef.addCategory('Constituent')
    vdef = smtk.attribute.VoidItemDefinition.New('VoidItem')
    def1.addItemDefinition(vdef)
    vdef.setIsOptional(True)
    vdef.setLabel('Option 1')

    def2 = system.createDefinition('Derived2', 'Derived1')
    def2.setAssociationMask(int(smtk.model.VOLUME))
    # Lets add some item definitions
    sitemdef = smtk.attribute.StringItemDefinition.New('StringItem1')
    def2.addItemDefinition(sitemdef)
    sitemdef.setIsMultiline(True)
    sitemdef.addCategory('Flow')
    sitemdef = smtk.attribute.StringItemDefinition.New('StringItem2')
    def2.addItemDefinition(sitemdef)
    sitemdef.setDefaultValue('Default')
    sitemdef.addCategory('General')
    uitemdef = smtk.attribute.ModelEntityItemDefinition.New('ModelEntityItem1')
    def2.addItemDefinition(uitemdef)
    uitemdef.addCategory('Flow')
    uitemdef.setMembershipMask(int(smtk.model.FACE))
    uitemdef = smtk.attribute.ModelEntityItemDefinition.New('ModelEntityItem2')
    def2.addItemDefinition(uitemdef)
    uitemdef.addCategory('General')
    uitemdef.setMembershipMask(
        int(smtk.model.GROUP_ENTITY | smtk.model.HOMOGENOUS_GROUP))
    dirdef = smtk.attribute.DirectoryItemDefinition.New('DirectoryItem')
    def2.addItemDefinition(dirdef)
    dirdef.setShouldExist(True)
    dirdef.setShouldBeRelative(True)
    fdef = smtk.attribute.FileItemDefinition.New('FileItem')
    def2.addItemDefinition(fdef)
    fdef.setShouldBeRelative(True)
    gdef = smtk.attribute.GroupItemDefinition.New('GroupItem')
    def2.addItemDefinition(gdef)
    fdef2 = smtk.attribute.FileItemDefinition.New('File1')
    gdef.addItemDefinition(fdef2)
    gdef1 = smtk.attribute.GroupItemDefinition.New('SubGroup')
    gdef.addItemDefinition(gdef1)
    sitemdef = smtk.attribute.StringItemDefinition.New('GroupString')
    gdef1.addItemDefinition(sitemdef)
    sitemdef.setDefaultValue('Something Cool')
    sitemdef.addCategory('General')
    sitemdef.addCategory('Flow')

    # Add in a Attribute definition with a reference to another attribute
    attrefdef = system.createDefinition('AttributeReferenceDef')
    aritemdef = smtk.attribute.RefItemDefinition.New('BaseDefItem')
    attrefdef.addItemDefinition(aritemdef)
    aritemdef.setCommonValueLabel('A reference to another attribute')
    aritemdef.setAttributeDefinition(base)

    # Process Categories
    system.updateCategories()
    # Lets test creating an attribute by passing in the expression definition
    # explicitly
    expAtt = system.createAttribute('Exp1', expDef)
    att = system.createAttribute('testAtt', 'Derived2')
    if att is None:
        print 'ERROR: Attribute testAtt not created'
        status = -1

    # Find the expression enabled item
    item = att.item(2)
    vitem = smtk.attribute.ValueItem.CastTo(item)
    writer = smtk.io.AttributeWriter()
    logger = smtk.io.Logger()
    if writer.write(system, sys.argv[1], logger):
        sys.stderr.write('Errors encountered creating Attribute File:\n')
        sys.stderr.write(logger.convertToString())
        sys.stderr.write('\n')
        status = -1

    del system
    print 'System destroyed'

    sys.exit(status)
