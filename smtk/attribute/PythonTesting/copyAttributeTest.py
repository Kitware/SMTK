"""
Test smtk.attribute.Manager.copyAttribute() method

Uses copyAttributeTest.sbi in the SMTKTestData repo.
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

INPUT_FILENAME  = 'copyAttributeTest.sbi'
OUTPUT_FILENAME = 'copyAttributeOut.sbi'

# ---------------------------------------------------------------------
if __name__ == '__main__':
  # First (and) only argument is the path to the smtk data directory
  if len(sys.argv) < 2:
    print
    print 'Test smtk.attribute.Manager.copyAttribute()'
    print 'Usage: python %s path-to-SMTKTestData'
    print
    sys.exit(-1)

  # To help with debugging, write some environment vars
  logging.debug('LD_LIBRARY_PATH = %s' % os.environ.get('LD_LIBRARY_PATH'))
  logging.debug('PYTHONPATH = %s' % os.environ.get('PYTHONPATH'))

    # Load attribute file into manager
  smtk_test_data = sys.argv[1]
  att_folder = os.path.join(smtk_test_data, 'smtk', 'attribute')
  att_path = os.path.join(att_folder, INPUT_FILENAME)
  logging.info('Reading %s' % att_path)
  input_manager = smtk.attribute.Manager()
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

  # Copy SecondConcrete attribute, which should copy alot of stuff
  att_list = input_manager.findAttributes('SecondConcrete')
  if not att_list:
    logging.error("Unabled to find SecondConcrete attribute")
    sys.exit(-2)

  source_att = att_list[0]
  test_manager.copyAttribute(source_att)
  expected_deftypes = [
    'SecondConcrete', 'AnotherAbstractBase', 'CommonBase',
    'FirstConcrete', 'PolyLinearFunction'
  ]
  for def_type in expected_deftypes:
    defn = test_manager.findDefinition(def_type)
    if defn is None:
      logging.error('Expected %s definition, found None' % def_type)
      err_count += 1

  expected_atttypes = ['FirstConcrete', 'SecondConcrete', 'PolyLinearFunction']
  for att_type in expected_atttypes:
    att_list = test_manager.findAttributes(att_type)
    if len(att_list) != 1:
      logging.error('Expected %s attribute, found %d' %
        (att_type, len(att_list)))
      err_count += 1

  # Note there is ALOT more that could & should be verified here
  logging.debug('Writing manager')

  # Write data out FYI
  writer = smtk.util.AttributeWriter()
  err = writer.write(test_manager, OUTPUT_FILENAME, logger)
  if err:
    logging.error("Unable to write output file")
    sys.exit(-3)
  logging.info('Wrote %s' % OUTPUT_FILENAME)

  # Check error count
  if err_count > 0:
    sys.exit(err_count)

  sys.exit(0)
