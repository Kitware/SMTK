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

SBT_FILENAME   = 'Basic2DFluid.sbt'
SBI_FILENAME   = 'Basic2DFluid.sbi'

# ---------------------------------------------------------------------
if __name__ == '__main__':
  # First (and) only argument is the path to the smtk data directory
  if len(sys.argv) < 2:
    print
    print 'Test smtk.attribute.Manager.copyDefinition()'
    print 'Usage: python %s path-to-SMTKTestData'
    print
    sys.exit(-1)

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

  # Copy Pressure definition, which should also copy base definition
  pressure_def = input_manager.findDefinition('Pressure')
  test_manager.copyDefinition(pressure_def)
  for def_type in ['Pressure', 'BoundaryCondition']:
    defn = test_manager.findDefinition(def_type)
    if defn is None:
      logging.error('Expected %s definition, found None' % def_type)
      err_count += 1

  # Copy Velocity definition, which should reuse base definition
  def_type = 'Velocity'
  velocity_def = input_manager.findDefinition(def_type)
  test_manager.copyDefinition(velocity_def)
  defn = test_manager.findDefinition(def_type)
  if defn is None:
    logging.error('Expected %s definition, found None' % def_type)
    err_count += 1

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
