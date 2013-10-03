"""
Manual port of SMTK/smtk/attribute/Testing/atributeAutoNameingTest.cxx
For verifying python-shiboken wrappers

Requires SMTKCorePython.so to be in module path
"""

import smtk

if __name__ == '__main__':
    import sys

    status = 0

    manager = smtk.attribute.Manager()
    print "Manager Created"
    def_ = manager.createDefinition("testDef")
    if def_ is not None:
      print "Definition testDef created"
    else:
      print "ERROR: Definition testDef not created"
      status = -1
    
    att = manager.createAttribute("testDef")
    if att is not None:
      print "Attribute %s created" % att.name()
    else:
      print "ERROR: 1st Attribute not created"
      status = -1
    
    att = manager.createAttribute("testDef")
    if att is not None:
      print "Attribute %s created" % att.name()
    else:
      print "ERROR: 2nd Attribute not created"
      status = -1
    
    att = manager.createAttribute("testDef")
    if att is not None:
      print "Attribute %s created" % att.name()
    else:
      print "ERROR: 3rd Attribute not created"
      status = -1

    del manager
    print 'Manager destroyed'

    sys.exit(status)
