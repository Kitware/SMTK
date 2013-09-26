"""
  Manual port of SMTK/smtk/attribute/Testing/expressionTest.cxx
  For verifying python-shiboken wrappers
  
  Requires SMTKCorePython.so to be in module path
  """

import smtk

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


if __name__ == '__main__':
  import sys
  
  status = 0
  
  manager = smtk.attribute.Manager()
  print 'Manager created'
  
  # Lets create an attribute to represent an expression
  expDef = manager.createDefinition("ExpDef");
  eitemdef = addItemDefinition(expDef, smtk.attribute.StringItemDefinition, "Expression String");
  eitemdef2 = addItemDefinition( expDef, smtk.attribute.StringItemDefinition, "Aux String");
  eitemdef.setDefaultValue("sample");
  
  base = manager.createDefinition("BaseDef");
  #Lets add some item definitions
  iitemdef = addItemDefinition( base, smtk.attribute.IntItemDefinition, "IntItem1");
  iitemdef = addItemDefinition( base, smtk.attribute.IntItemDefinition, "IntItem2");
  iitemdef.setDefaultValue(10);

  def1 = manager.createDefinition("Derived1", "BaseDef");
  # add some item definitions
  ditemdef = addItemDefinition(def1, smtk.attribute.DoubleItemDefinition, "DoubleItem1");
  # Allow this one to hold an expression
  ditemdef.setExpressionDefinition(expDef);
  # Check to make sure we can use expressions
  if not ditemdef.allowsExpressions():
    print  "ERROR - Item Def does not allow expressions";
    status = -1;
  ditemdef = addItemDefinition(def1, smtk.attribute.DoubleItemDefinition, "DoubleItem2");
  ditemdef.setDefaultValue(-35.2);

  def2 = manager.createDefinition("Derived2", "Derived1");
  # Lets add some item definitions
  sitemdef = addItemDefinition(def2, smtk.attribute.StringItemDefinition, "StringItem1");
  sitemdef = addItemDefinition(def2, smtk.attribute.StringItemDefinition, "StringItem2");
  sitemdef.setDefaultValue("Default");
  
  # Lets test creating an attribute by passing in the expression definition explicitly
  expAtt = manager.createAttribute("Exp1", expDef);
  att = manager.createAttribute("testAtt", "Derived2");
  if att is not None:
      print "Attribute testAtt created";
  else:
      print "ERROR: Attribute testAtt not created";
      status = -1;

  # Find the expression enabled item
  vitem = smtk.attribute.ValueItem.CastTo(att.item(2))
  if vitem.allowsExpressions():
    vitem.setExpression(expAtt);
    print "Expression Set on %s" % vitem.name()
  else:
    print "ERROR: Can not set expression on %s" % vitem.name();
    status = -1

  n = att.numberOfItems();
  print "Items of testAtt:";
  for i in range(0,n,1):
    item = att.item(i);
    sys.stdout.write("\t%s Type = %s, " % (item.name(), smtk.attribute.Item.type2String(item.type())))
    vitem = smtk.attribute.ValueItem.CastTo(item);
    if vitem is not None:
      if vitem.isExpression(0):
        print " using Expression: %s" % vitem.expression(0).name();
      else:
        print " Value = %s" % vitem.valueAsString();

  print
  input = raw_input('Hit <Enter> to delete Managar and exit')
  del manager
  print 'Manager destroyed'
  
  sys.exit(status)
