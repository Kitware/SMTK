#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================
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
import unittest

try:
    import smtk
    if smtk.wrappingProtocol() == 'pybind11':
        import smtk.io
        import smtk.model
    import smtk.testing
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
XREF_FILENAME = 'test2D.xref'
SBT_FILENAME = 'Basic2DFluid.sbt'
SBI_FILENAME = 'Basic2DFluid.sbi'


class TestModelAttributes(unittest.TestCase):

    def load_xref(self, scope, folder=None):
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

                # print line
                parts = line.split()
                # print parts[2], parts[-1]
                entity_type = parts[2]
                uuid = parts[-1]
                entity_list = list_map.get(entity_type)
                entity_list.append(uuid)
            done = True

        if not done:
            logging.error('Problem loading %s' % path)
            sys.exit(3)

    def generate_attributes(self, scope):
        '''Builds and returns attribute system

        Also adds boundary groups to the model
        '''
        # Load attribute file
        att_folder = os.path.join(self.model_folder, 'attribute')
        att_folder = os.path.join(
            smtk.testing.DATA_DIR, 'attribute', 'attribute_system')
        att_path = os.path.join(att_folder, SBT_FILENAME)
        logging.info('Reading %s' % att_path)
        system = smtk.attribute.System.create()
        # system.setRefModelManager(scope.store)
        reader = smtk.io.AttributeReader()
        logger = smtk.io.Logger()
        err = reader.read(system, att_path, logger)
        if err:
            logging.error("Unable to load template file")
            logging.error(logger.convertToString())
            sys.exit(4)

        # Create material attribute & associate to model face
        system.setRefModelManager(scope.store)
        defn = system.findDefinition('Material')
        value = 1.01
        for i, face in enumerate(scope.face_list, start=1):
            att_name = 'material %d' % i
            att = system.createAttribute(att_name, defn)

            for item_name in ['Density', 'Viscosity']:
                item = att.find(item_name)
                concrete_item = smtk.attribute.to_concrete(item)
                concrete_item.setValue(0, value)
                value += 0.491

            face_id = uuid.UUID(face)
            logging.debug('Associate attribute \"%s\" to face %s' %
                          (att_name, face_id))
            att.associateEntity(face_id)

            # Save attribute name and model entity (uuid) for checking later
            meta = (att.name(), face_id)
            scope.att_data.append(meta)

        # Generate boundary groups, hard-code to specific model edges
        flags = smtk.model.MODEL_BOUNDARY | smtk.model.DIMENSION_1
        left_edges = scope.store.addGroup(flags, 'left_edges')
        uuid_list = list()
        for i in [0, 1, 2]:
            uuid_list.append(uuid.UUID(scope.edge_list[i]))
        if smtk.wrappingProtocol() == 'pybind11':
            scope.store.addToGroup(left_edges.entity(), set(uuid_list))
        else:
            scope.store.addToGroup(left_edges.entity(), uuid_list)

        right_edges = scope.store.addGroup(flags, 'right_edges')
        del uuid_list[:]
        for i in [6, 9]:
            uuid_list.append(uuid.UUID(scope.edge_list[i]))
        if smtk.wrappingProtocol() == 'pybind11':
            scope.store.addToGroup(right_edges.entity(), set(uuid_list))
        else:
            scope.store.addToGroup(right_edges.entity(), uuid_list)

        # Create boundary condition attributes
        defn = system.findDefinition('Velocity')
        left_att = system.createAttribute('leftBC', defn)
        item = left_att.item(0)
        concrete_item = smtk.attribute.to_concrete(item)
        concrete_item.setValue(0, 3.14159)
        concrete_item.setValue(1, 2.71828)
        logging.debug('Associate attribute \"%s\" to boundary group %s' %
                      (left_att.name(), left_edges.name()))
        ok = left_att.associateEntity(left_edges.entity())
        meta = (left_att.name(), left_edges.entity())
        scope.att_data.append(meta)

        defn = system.findDefinition('Pressure')
        right_att = system.createAttribute('rightBC', defn)
        item = left_att.item(0)
        concrete_item = smtk.attribute.to_concrete(item)
        concrete_item.setValue(0, 14.1)
        logging.debug('Associate attribute \"%s\" to boundary group %s' %
                      (right_att.name(), right_edges.name()))
        ok = right_att.associateEntity(right_edges.entity())
        meta = (right_att.name(), right_edges.entity())
        scope.att_data.append(meta)

        return system

    def check_attributes(self, scope, system):
        '''Checks for attributes and associations

        Returns number of errors found
        '''
        error_count = 0  # return value

        for t in scope.att_data:
            att_name, entity_uuid = t
            # logging.debug('att_name %s, uuid %s' % t)
            att = system.findAttribute(att_name)
            if not att:
                logging.error('Missing attribute %s' % att_name)
                error_count += 1
            entity_id_set = att.associatedModelEntityIds()
            if not entity_id_set:
                logging.error(
                    'Missing model entity on attribute %s' % att_name)
                error_count += 1

            entity_ids = list(entity_id_set)
            entity_id = entity_ids[0]
            # Compare uuid strings
            if entity_id.hex != entity_uuid.hex:
                logging.error('Unexpected model entity %s on attribute %s' %
                              (entity_ids[0], att_name))
                error_count += 1

        logging.debug('check_attributes error_count %d' % error_count)
        return error_count

    def testAssociation(self):

        # Define scope object to store shared data
        ScopeType = type('Scope', (object,), dict())
        scope = ScopeType()
        self.model_folder = os.path.join(
            smtk.testing.DATA_DIR, 'model', '2d', 'smtk')

        # Load the model file
        model_path = os.path.join(self.model_folder, MODEL_FILENAME)
        logging.info('Reading %s' % model_path)
        json_string = None
        with open(model_path, 'r') as f:
            json_string = f.read()
        if json_string is None:
            logging.error('Unable to load input file')
            sys.exit(2)
        scope.store = smtk.model.Manager.create()
        ok = smtk.io.LoadJSON.intoModelManager(json_string, scope.store)

        # Load cross-reference file
        self.load_xref(scope, self.model_folder)

        # Build attributes and write to file
        scope.att_data = list()
        system = self.generate_attributes(scope)
        logging.info('Writing %s' % SBI_FILENAME)
        writer = smtk.io.AttributeWriter()
        logger = smtk.io.Logger()
        err = writer.write(system, SBI_FILENAME, logger)
        if err:
            logging.error('Unable to write attribute file')
            logging.error(logger.convertToString())
            sys.exit(5)

        # Delete model & attributes
        del scope.store
        del system

        # Re-import model
        test_store = smtk.model.Manager.create()
        ok = smtk.io.LoadJSON.intoModelManager(json_string, test_store)
        scope.store = test_store

        # Re-read attribute file
        logging.info('Reading back %s' % SBI_FILENAME)
        test_system = smtk.attribute.System.create()
        reader = smtk.io.AttributeReader()
        err = reader.read(test_system, SBI_FILENAME, logger)
        if err:
            logging.error("Unable to read attribute file")
            logging.error(logger.convertToString())
        self.assertTrue(not err, "Unable to read attribute file")

        # Set model and verify attributes
        test_system.setRefModelManager(scope.store)
        error_count = self.check_attributes(scope, test_system)
        self.assertEqual(error_count, 0, "At least one error occurred.")


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
