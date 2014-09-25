"""
Test smtk.attribute.Manager.copyAttribute() method

Uses copyAttributeTest.sbi in the SMTKTestData repo.
"""

import logging
import os
import sys
import uuid

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

MODEL_FILENAME  = 'test2D.json'
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

  smtk_test_data = sys.argv[1]

  #
  # Load smtk model file
  #
  model_path = os.path.join(smtk_test_data, 'smtk', MODEL_FILENAME)
  logging.info('Reading %s' % model_path)
  json_string = None
  with open(model_path, 'r') as f:
    json_string = f.read()
  if json_string is None:
    logging.error('Unable to load input file %s' % model_path)
    sys.exit(-3)
  model_manager = smtk.model.Manager.create()
  ok = smtk.model.ImportJSON.intoModel(json_string, model_manager)
  if not ok:
      logging.error("Unable to create model from contents of %s" % model_path)
      sys.exit(-4)

  #
  # Load attribute file into manager
  #
  att_folder = os.path.join(smtk_test_data, 'smtk', 'attribute')
  att_path = os.path.join(att_folder, INPUT_FILENAME)
  logging.info('Reading %s' % att_path)
  input_manager = smtk.attribute.Manager()
  input_manager.setRefModelManager(model_manager)

  reader = smtk.util.AttributeReader()
  logger = smtk.util.Logger()
  err = reader.read(input_manager, att_path, logger)
  if err:
    logging.error("Unable to load template file")
    logging.error(logger.convertToString())
    sys.exit(-2)

  err_count = 0

  # Add model associations, using known UUID values (see test2D.xref)
  att_list = input_manager.findAttributes('FirstConcrete')
  if not att_list:
    logging.error("Unabled to find FirstConcrete attribute")
    sys.exit(-2)
  first_concrete = att_list[0]

  vertex09 = '80640078-ee31-4117-ac6b-59261a7991d4'
  vertex_id = uuid.UUID(vertex09)
  first_concrete.associateEntity(vertex_id)


  att_list = input_manager.findAttributes('SecondConcrete')
  if not att_list:
    logging.error("Unabled to find SecondConcrete attribute")
    sys.exit(-2)
  second_concrete = att_list[0]

  vertex14 = 'f90b9b63-6e55-4ba3-b690-bfb9d7bc3be1'
  vertex_id = uuid.UUID(vertex14)
  second_concrete.associateEntity(vertex_id)

  vertex15 =  '05772ac6-920a-484c-b6b2-fc87b305bc17'
  vertex_id = uuid.UUID(vertex15)
  second_concrete.associateEntity(vertex_id)


  # Also test model entity item
  item = second_concrete.find('ModelFace')
  model_entity_item = smtk.attribute.to_concrete(item)
  face26 = '0bbc9685-e506-4fe7-b52f-ae52888d744b'
  face_id = uuid.UUID(face26)
  cursor = smtk.model.Cursor(model_manager, face_id)
  model_entity_item.setValue(0, cursor)


  #
  # Instantiate 2nd/test manager
  #
  test_manager = smtk.attribute.Manager()
  test_manager.setRefModelManager(model_manager)

  # Copy SecondConcrete attribute
  # Note: shiboken refuses to wrap smtk::attribute::Manager::CopyOptions enum, saying:
  #   enum 'smtk::model::Manager::CopyOptions' is specified in typesystem, but not declared
  # Rather than trying to reason with shiboken, the options are specified numerically
  options = 0x00000001  # should be smtk.attribute.Manager.CopyOptions.COPY_ASSOCIATIONS
  test_manager.copyAttribute(second_concrete, options)
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
    sys.exit(-6)
  logging.info('Wrote %s' % OUTPUT_FILENAME)

  # Check error count
  if err_count > 0:
    sys.exit(err_count)

  sys.exit(0)
