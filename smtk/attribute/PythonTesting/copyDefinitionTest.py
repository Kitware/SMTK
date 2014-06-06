"""
Test smtk.attribute.Manager.copyDefinition() method

Uses Basic2DFluid.sbt in the SMTKTestData repo.
"""

import logging
import os
import sys

try:
    import smtk
except ImportError:
    print
    print 'Not able to import smtk library. You might need to:'
    print '  - Use the PYTHONPATH variable to point to the smtk python lib'
    print '  - And/or use the LD_LIBRARY_PATH variable to point to the shiboken libraries'
    print
    sys.exit(-1)


logging.basicConfig(level=logging.DEBUG)

SBT_FILENAME   = 'copyDefinitionTest.sbt'
SBI_FILENAME   = 'copyDefinitionTest.sbi'

# ---------------------------------------------------------------------
if __name__ == '__main__':
  # First (and) only argument is the path to the smtk data directory
  if len(sys.argv) < 2:
    print
    print 'Test smtk.attribute.Manager.copyDefinition()'
    print 'Usage: python %s path-to-SMTKTestData'
    print
    sys.exit(-1)

  logging.debug('LD_LIBRARY_PATH = %s' % os.environ.get('LD_LIBRARY_PATH'))
  logging.debug('PYTHONPATH = %s' % os.environ.get('PYTHONPATH'))

  # Define scope object to store shared data
  ScopeType = type('Scope', (object,), dict())
  scope = ScopeType()
  smtk_test_data = sys.argv[1]
  model_folder = os.path.join(smtk_test_data, 'smtk')

  # Load attribute file into manager
  att_folder = os.path.join(model_folder, 'attribute')
  att_path = os.path.join(att_folder, SBT_FILENAME)
  logging.info('Reading %s' % att_path)
  input_manager = smtk.attribute.Manager()
  #manager.setRefStorage(scope.store)
  reader = smtk.util.AttributeReader()
  logger = smtk.util.Logger()
  err = reader.read(input_manager, att_path, logger)
  if err:
    logging.error("Unable to load template file")
    logging.error(logger.convertToString())
    sys.exit(-2)

  err_count = 0

  # Instantiate 2nd manager
  test_manager = smtk.attribute.Manager()

  # Copy SecondConcrete definition, which should copy alot of stuff
  source_def = input_manager.findDefinition('SecondConcrete')
  test_manager.copyDefinition(source_def)
  expected_types = [
    'SecondConcrete', 'AnotherAbstractBase', 'CommonBase',
    'FirstConcrete'  # , 'PolyLinearFunction'
  ]
  for def_type in expected_types:
    defn = test_manager.findDefinition(def_type)
    if defn is None:
      logging.error('Expected %s definition, found None' % def_type)
      err_count += 1

  # Note there is ALOT more that could & should be verified here
  logging.debug('Writing manager')

  # Write data out FYI
  writer = smtk.util.AttributeWriter()
  err = writer.write(test_manager, SBI_FILENAME, logger)
  if err:
    logging.error("Unable to write output file")
    sys.exit(-3)
  logging.info('Wrote %s' % SBI_FILENAME)

  # Check error count
  if err_count > 0:
    sys.exit(err_count)

  sys.exit(0)
