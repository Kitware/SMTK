"""
Manual port of SMTK/smtk/attribute/Testing/basicAttributeXMLWriterTest.cxx
For verifying python-shiboken wrappers

Requires SMTKCorePython.so to be in module path
"""

def addItemDefinition( ato, data_type, name):
  def_ = data_type.New(name)
  if def_ is None:
    print "could not create"
    return None
  idef = data_type.ToItemDefinition(def_)
  if idef is None:
    print "could not convert"
    return None
  if not ato.addItemDefinition(idef):
    print "could not add"
    return None
  return def_


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
    eitemdef = addItemDefinition( expDef, smtk.attribute.StringItemDefinition, 'Expression String')
    eitemdef2 = addItemDefinition( expDef, smtk.attribute.StringItemDefinition, 'Aux String')
    eitemdef.setDefaultValue('sample')

    base = manager.createDefinition('BaseDef')
    # Lets add some item definitions
    iitemdef = addItemDefinition( base, smtk.attribute.IntItemDefinition, 'TEMPORAL')
    iitemdef.setCommonValueLabel('Time')
    iitemdef.addDiscreteValue(0, 'Seconds')
    iitemdef.addDiscreteValue(1, 'Minutes')
    iitemdef.addDiscreteValue(2, 'Hours')
    iitemdef.addDiscreteValue(3, 'Days')
    iitemdef.setDefaultDiscreteIndex(0)
    iitemdef.addCategory('Time')
    iitemdef = addItemDefinition( base, smtk.attribute.IntItemDefinition, 'IntItem2')
    iitemdef.setDefaultValue(10);
    iitemdef.addCategory('Heat');

    def1 = manager.createDefinition('Derived1', 'BaseDef')
    def1.setAssociationMask(0x20) # belongs on domains
    # Lets add some item definitions
    ditemdef = addItemDefinition(def1, smtk.attribute.DoubleItemDefinition, 'DoubleItem1')
    # Allow this one to hold an expression
    ditemdef.addCategory('Veg')
    ditemdef.setExpressionDefinition(expDef)
    # Check to make sure we can use expressions
    if not ditemdef.allowsExpressions():
      print 'ERROR - Item Def does not allow expressions'
      status = -1
    ditemdef = addItemDefinition( def1, smtk.attribute.DoubleItemDefinition, 'DoubleItem2')
    ditemdef.setDefaultValue(-35.2)
    ditemdef.setMinRange(-100, True)
    ditemdef.setMaxRange(125.0, False)
    ditemdef.addCategory('Constituent')
    vdef = addItemDefinition( def1, smtk.attribute.VoidItemDefinition, 'VoidItem')
    vdef.setIsOptional(True)
    vdef.setLabel('Option 1')

    def2 = manager.createDefinition('Derived2', 'Derived1')
    def2.setAssociationMask(0x7)
    # Lets add some item definitions
    sitemdef = addItemDefinition( def2, smtk.attribute.StringItemDefinition, 'StringItem1' )
    sitemdef.setIsMultiline(True)
    sitemdef.addCategory('Flow')
    sitemdef = addItemDefinition( def2, smtk.attribute.StringItemDefinition, 'StringItem2' )
    sitemdef.setDefaultValue('Default')
    sitemdef.addCategory('General')
    dirdef = addItemDefinition( def2, smtk.attribute.DirectoryItemDefinition, 'DirectoryItem')
    dirdef.setShouldExist(True)
    dirdef.setShouldBeRelative(True)
    fdef = addItemDefinition( def2, smtk.attribute.FileItemDefinition, 'FileItem' )
    fdef.setShouldBeRelative(True);
    gdef = addItemDefinition( def2, smtk.attribute.GroupItemDefinition, 'GroupItem' )
    addItemDefinition( gdef, smtk.attribute.FileItemDefinition, 'File1' )
    gdef1 = addItemDefinition( gdef, smtk.attribute.GroupItemDefinition, 'SubGroup')
    sitemdef = addItemDefinition( gdef1, smtk.attribute.StringItemDefinition, 'GroupString')
    sitemdef.setDefaultValue('Something Cool')
    sitemdef.addCategory('General')
    sitemdef.addCategory('Flow')

    # Add in a Attribute definition with a reference to another attribute
    attrefdef = manager.createDefinition('AttributeReferenceDef')
    aritemdef = addItemDefinition( attrefdef, smtk.attribute.AttributeRefItemDefinition, 'BaseDefItem' )
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
    writer = smtk.attribute.AttributeWriter()
    if writer.write(manager, sys.argv[1]):
        sys.stderr.write('Errors encountered creating Attribute File:\n')
        sys.stderr.write(writer.errorMessages())
        sys.stderr.write('\n')

    del manager
    print 'Manager destroyed'

    sys.exit(status)
