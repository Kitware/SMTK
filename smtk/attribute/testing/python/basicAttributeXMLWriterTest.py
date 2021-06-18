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
Manual port of SMTK/smtk/attribute/Testing/basicAttributeXMLWriterTest.cxx
For verifying python wrappers

Requires smtkCorePython.so to be in module path
"""

import smtk
from smtk import attribute
from smtk import io
from smtk import model

# print(dir(smtk.io))

if __name__ == '__main__':
    import sys

    status = 0

    if len(sys.argv) != 2:
        print('Usage: %s filename' % sys.argv[0])
        sys.exit(-1)

    resource = smtk.attribute.Resource.create()
    print('Resource created')
    # Let's add some analyses
    analyses = resource.analyses()
    cats = set()
    cats.add('Flow')
    cats.add('General')
    cats.add('Time')
    analysis = analyses.create('CFD Flow')
    analysis.setLocalCategories(cats)
    cats.clear()

    cats.add('Flow')
    cats.add('Heat')
    cats.add('General')
    cats.add('Time')
    analysis = analyses.create('CFD Flow with Heat Transfer')
    analysis.setLocalCategories(cats)
    cats.clear()

    cats.add('Constituent')
    cats.add('General')
    cats.add('Time')
    analysis = analyses.create('Constituent Transport')
    analysis.setLocalCategories(cats)
    cats.clear()

    # Lets create an attribute to represent an expression
    expDef = resource.createDefinition('ExpDef')
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

    base = resource.createDefinition('BaseDef')
    # Lets add some item definitions
    iitemdef = smtk.attribute.IntItemDefinition.New('TEMPORAL')
    base.addItemDefinition(iitemdef)
    iitemdef.setCommonValueLabel('Time')
    iitemdef.addDiscreteValue(0, 'Seconds')
    iitemdef.addDiscreteValue(1, 'Minutes')
    iitemdef.addDiscreteValue(2, 'Hours')
    iitemdef.addDiscreteValue(3, 'Days')
    iitemdef.setDefaultDiscreteIndex(0)
    iitemdef.localCategories().insertInclusion('Time')
    iitemdef = smtk.attribute.IntItemDefinition.New('IntItem2')
    base.addItemDefinition(iitemdef)
    iitemdef.setDefaultValue(10)
    iitemdef.localCategories().insertInclusion('Heat')

    def1 = resource.createDefinition('Derived1', 'BaseDef')
    def1.setLocalAssociationMask(
        int(smtk.model.MODEL_DOMAIN))  # belongs on model
    # Lets add some item definitions
    ditemdef = smtk.attribute.DoubleItemDefinition.New('DoubleItem1')
    def1.addItemDefinition(ditemdef)
    # Allow this one to hold an expression
    ditemdef.localCategories().insertInclusion('Veg')
    ditemdef.setExpressionDefinition(expDef)
    # Check to make sure we can use expressions
    if not ditemdef.allowsExpressions():
        print('ERROR - Item Def does not allow expressions')
        status = -1
    ditemdef = smtk.attribute.DoubleItemDefinition.New('DoubleItem2')
    def1.addItemDefinition(ditemdef)
    ditemdef.setDefaultValue(-35.2)
    ditemdef.setMinRange(-100, True)
    ditemdef.setMaxRange(125.0, False)
    ditemdef.localCategories().insertInclusion('Constituent')
    vdef = smtk.attribute.VoidItemDefinition.New('VoidItem')
    def1.addItemDefinition(vdef)
    vdef.setIsOptional(True)
    vdef.setLabel('Option 1')

    def2 = resource.createDefinition('Derived2', 'Derived1')
    def2.setLocalAssociationMask(int(smtk.model.VOLUME))
    # Lets add some item definitions
    sitemdef = smtk.attribute.StringItemDefinition.New('StringItem1')
    def2.addItemDefinition(sitemdef)
    sitemdef.setIsMultiline(True)
    sitemdef.localCategories().insertInclusion('Flow')
    sitemdef = smtk.attribute.StringItemDefinition.New('StringItem2')
    def2.addItemDefinition(sitemdef)
    sitemdef.setDefaultValue('Default')
    sitemdef.localCategories().insertInclusion('General')
    uitemdef = smtk.attribute.ModelEntityItemDefinition.New('ModelEntityItem1')
    def2.addItemDefinition(uitemdef)
    uitemdef.localCategories().insertInclusion('Flow')
    uitemdef.setMembershipMask(int(smtk.model.FACE))
    uitemdef = smtk.attribute.ModelEntityItemDefinition.New('ModelEntityItem2')
    def2.addItemDefinition(uitemdef)
    uitemdef.localCategories().insertInclusion('General')
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
    sitemdef.localCategories().insertInclusion('General')
    sitemdef.localCategories().insertInclusion('Flow')

    # Add in a Attribute definition with a reference to another attribute
    attcompdef = resource.createDefinition('AttributeComponentDef')
    acitemdef = smtk.attribute.ComponentItemDefinition.New('BaseDefItem')
    attcompdef.addItemDefinition(acitemdef)
    acitemdef.setCommonValueLabel('A reference to another attribute')
    queryString = smtk.attribute.Resource.createAttributeQuery(base)
    acitemdef.setAcceptsEntries(resource.typeName(), queryString, True)

    # Process Definition Information
    resource.finalizeDefinitions()
    # Lets test creating an attribute by passing in the expression definition
    # explicitly
    expAtt = resource.createAttribute('Exp1', expDef)
    att = resource.createAttribute('testAtt', 'Derived2')
    if att is None:
        print('ERROR: Attribute testAtt not created')
        status = -1

    # Find the expression enabled item
    item = att.item(2)
    vitem = smtk.attribute.ValueItem.CastTo(item)
    writer = smtk.io.AttributeWriter()
    logger = smtk.io.Logger()
    if writer.write(resource, sys.argv[1], logger):
        sys.stderr.write('Errors encountered creating Attribute File:\n')
        sys.stderr.write(logger.convertToString())
        sys.stderr.write('\n')
        status = -1

    del resource
    print('Resource destroyed')

    sys.exit(status)
