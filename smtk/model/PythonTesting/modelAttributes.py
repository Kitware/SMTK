"""
Test attribute association with smtk model

Uses test2D.json model file in the SMTKTestData repo.
Also uses test2D.xref, a cross-reference file between test2D.json
and an old-generation model file, test2D.cmb, that can be found in
the CMBTestingData repo.
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

# Define input filenames here
MODEL_FILENAME = 'test2D.json'
XREF_FILENAME  = 'test2D.xref'
SBT_FILENAME   = 'Basic2DFluid.sbt'
SBI_FILENAME   = 'Basic2DFluid.sbi'



# ---------------------------------------------------------------------
def load_xref(scope, folder=None):
  '''Parses cross-reference file to initialize lists of entity uuids.

  List has same order as entities in original (CMB) model
  '''
  scope.vertex_list = list()
  scope.edge_list = list()
  scope.face_list = list()

  list_map = {
    'vertex': scope.vertex_list,
    'edge': scope.edge_list,
    'face': scope.face_list
  }

  filename = 'test2D.xref'
  path = filename
  if folder is not None:
    path = os.path.join(folder, filename)
  logging.info('Loading %s' % path)
  done = False
  with open(path, 'r') as f:
    for line in f.readlines():
      if line.startswith('Reading'):
        continue

      #print line
      parts = line.split()
      #print parts[2], parts[-1]
      entity_type = parts[2]
      uuid = parts[-1]
      entity_list = list_map.get(entity_type)
      entity_list.append(uuid)
    done = True

  if not done:
    logging.error('Problem loading %s' % path)
    sys.exit(3)


# ---------------------------------------------------------------------
def generate_attributes(scope):
  '''Builds and returns attribute manager

  Also adds boundary groups to the model
  '''
    # Load attribute file
  att_folder = os.path.join(model_folder, 'attribute')
  att_path = os.path.join(att_folder, SBT_FILENAME)
  logging.info('Reading %s' % att_path)
  manager = smtk.attribute.Manager()
  #manager.setRefStorage(scope.store)
  reader = smtk.util.AttributeReader()
  logger = smtk.util.Logger()
  err = reader.read(manager, att_path, logger)
  if err:
    logging.error("Unable to load template file")
    logging.error(logger.convertToString())
    sys.exit(4)

  # Create material attribute & associate to model face
  manager.setRefStorage(scope.store)
  defn = manager.findDefinition('Material')
  value = 1.01
  for i,face in enumerate(scope.face_list, start=1):
    att_name = 'material %d' % i
    att = manager.createAttribute(att_name, defn)

    for item_name in ['Density', 'Viscosity']:
      item = att.find(item_name)
      concrete_item = smtk.attribute.to_concrete(item)
      concrete_item.setValue(0, value)
      value += 0.491

    face_id = uuid.UUID(face)
    logging.debug('Associate attribute \"%s\" to face %s' % \
      (att_name, face_id))
    att.associateEntity(face_id)

  # Generate boundary groups, hard-code to specific model edges
  flags = smtk.model.MODEL_BOUNDARY | smtk.model.DIMENSION_1
  left_edges = scope.store.addGroup(flags, 'left_edges')
  uuid_list = list()
  for i in [0, 1, 2]:
    uuid_list.append(uuid.UUID(scope.edge_list[i]))
  scope.store.addToGroup(left_edges.entity(), uuid_list)

  right_edges = scope.store.addGroup(flags, 'right_edges')
  del uuid_list[:]
  for i in [6, 9]:
    uuid_list.append(uuid.UUID(scope.edge_list[i]))
  scope.store.addToGroup(right_edges.entity(), uuid_list)

  # Create boundary condition attributes
  return manager


# ---------------------------------------------------------------------
def check_attributes(scope, manager):
  '''Checks for attributes and associations

  Returns number of errors found
  '''
  error_count = 0  # return value

  # Get material attributes and sort by name
  att_list = manager.findAttributes('Material')
  att_list.sort(key=lambda att: att.name())
  for i, att in enumerate(att_list):
    face_id = uuid.UUID(scope.face_list[i])
    if not att.isEntityAssociated(face_id):
      logging.error('Missing association between attribute %s and face %s') % \
        (att.name(), scope.face_list[i])
      error_count += 1
  return error_count


# ---------------------------------------------------------------------
if __name__ == '__main__':
  # First (and) only script argument is the path to the smtk data directory
  if len(sys.argv) < 2:
    print
    print 'Test attribute association with smtk model'
    print 'Usage: python %s path-to-SMTKTestData'
    print
    sys.exit(1)

  # Define scope object to store shared data
  ScopeType = type('Scope', (object,), dict())
  scope = ScopeType()
  smtk_test_data = sys.argv[1]
  model_folder = os.path.join(smtk_test_data, 'smtk')

  # Load the model file
  model_path = os.path.join(model_folder, MODEL_FILENAME)
  logging.info('Reading %s' % model_path)
  json_string = None
  with open(model_path, 'r') as f:
    json_string = f.read()
  if json_string is None:
    logging.error('Unable to load input file')
    sys.exit(2)
  scope.store = smtk.model.Storage.create()
  ok = smtk.model.ImportJSON.intoModel(json_string, scope.store)

  # Load cross-reference file
  load_xref(scope, model_folder)

  # Build attributes and write to file
  manager = generate_attributes(scope)
  logging.info('Writing %s' % SBI_FILENAME)
  writer = smtk.util.AttributeWriter()
  logger = smtk.util.Logger()
  err = writer.write(manager, SBI_FILENAME, logger)
  if err:
    logging.error('Unable to write attribute file')
    logging.error(logger.convertToString())
    sys.exit(5)

  # Delete attributes and reload from file
  del manager
  logging.info('Reading back %s' % SBI_FILENAME)
  test_manager = smtk.attribute.Manager()
  reader = smtk.util.AttributeReader()
  err = reader.read(test_manager, SBI_FILENAME, logger)
  if err:
    logging.error("Unable to read attribute file")
    logging.error(logger.convertToString())
    sys.exit(6)

  # Set model and verify attributes
  test_manager.setRefStorage(scope.store)
  error_count = check_attributes(scope, test_manager)
  if error_count > 0:
    sys.exit(7)

  sys.exit(0)
