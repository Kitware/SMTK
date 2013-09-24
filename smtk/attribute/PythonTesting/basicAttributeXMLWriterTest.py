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

    """
    TODO Add rest of code
    """

    writer = smtk.attribute.AttributeWriter()
    if writer.write(manager, sys.argv[1]):
        sys.stderr.write('Errors encountered creating Attribute File:\n')
        sys.stderr.write(writer.errorMessages())
        sys.stderr.write('\n')

    print
    input = raw_input('Hit <Enter> to delete Managar and exit')
    del manager
    print 'Manager destroyed'

    sys.exit(status)
