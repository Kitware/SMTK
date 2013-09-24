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

    att1 = manager.createAttribute('testAtt', 'testDef')
    if att1 is None:
        print 'Duplicate Attribute testAtt not created'
    else:
        print 'ERROR: Duplicate Attribute testAtt  created'
        status = -1

    print
    input = raw_input('Hit <Enter> to delete Managar and exit')
    del manager
    print 'Manager destroyed'

    sys.exit(status)
