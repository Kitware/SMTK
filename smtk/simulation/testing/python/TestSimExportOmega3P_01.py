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
import os
import shutil
import sys
import unittest

import smtk
import smtk.attribute
import smtk.io
import smtk.operation
import smtk.resource
import smtk.session.mesh

import smtk.testing

import util

OPERATION_SUCCEEDED = int(smtk.operation.Operation.SUCCEEDED)  # 3
MODEL_NAME = 'pillbox4'
OUTPUT_FOLDER = 'export_{}'.format(MODEL_NAME)


class TestSimExportACE3P(smtk.testing.TestCase):

    def setUp(self):
        if smtk.testing.DATA_DIR == '':
            self.skipTest('SMTK test-data directory not provided')

        # Make sure last test result is removed
        export_folder = os.path.join(smtk.testing.TEMP_DIR, OUTPUT_FOLDER)
        if os.path.exists(export_folder):
            shutil.rmtree(export_folder)

        self.res_manager = smtk.resource.Manager.create()
        self.op_manager = smtk.operation.Manager.create()

        smtk.attribute.Registrar.registerTo(self.res_manager)
        smtk.attribute.Registrar.registerTo(self.op_manager)

        smtk.session.mesh.Registrar.registerTo(self.res_manager)
        smtk.session.mesh.Registrar.registerTo(self.op_manager)

        smtk.operation.Registrar.registerTo(self.op_manager)
        self.op_manager.registerResourceManager(self.res_manager)

    def tearDown(self):
        self.res_manager = None
        self.op_manager = None

    def import_resource(self, path):
        import_op = self.op_manager.createOperation(
            'smtk::operation::ImportResource')
        import_op.parameters().find('filename').setValue(path)
        import_result = import_op.operate()
        import_outcome = import_result.findInt('outcome').value(0)
        self.assertEqual(import_outcome, OPERATION_SUCCEEDED)
        resource = import_result.find('resource').value()
        self.assertIsNotNone(resource)
        return resource

    def import_sbt(self, path):
        """Deprecated - use import_resource()"""
        att_resource = smtk.attribute.Resource.create()
        reader = smtk.io.AttributeReader()
        logger = smtk.io.Logger.instance()
        read_err = reader.read(att_resource, path, True, logger)
        self.assertFalse(read_err)
        return att_resource

    def import_python_op(self, path):
        import_op = self.op_manager.createOperation(
            'smtk::operation::ImportPythonOperation')
        self.assertIsNotNone(import_op)
        import_op.parameters().find('filename').setValue(path)
        import_result = import_op.operate()
        import_outcome = import_result.findInt('outcome').value(0)
        self.assertEqual(import_outcome, OPERATION_SUCCEEDED)

        op_unique_name = import_result.findString("unique_name").value()
        op = self.op_manager.createOperation(op_unique_name)
        return op

    def testPillbox4(self):
        # Minimal test case
        gen_filename = '{}.gen'.format(MODEL_NAME)
        gen_path = os.path.join(
            smtk.testing.DATA_DIR, 'model', '3d', 'genesis', gen_filename)
        resource = self.import_resource(gen_path)
        model_resource = smtk.model.Resource.CastTo(resource)
        self.assertIsNotNone(model_resource)

        # Set the model location in lieu of writing it to the file system
        model_filename = '{}.smtk'.format(MODEL_NAME)
        model_location = os.path.join(smtk.testing.TEMP_DIR, model_filename)
        model_resource.setLocation(model_location)

        # Load attribute template
        sbt_path = os.path.join(
            smtk.testing.DATA_DIR, 'simulation-workflows', 'ACE3P', 'ACE3P.sbt')
        self.assertTrue(os.path.exists(sbt_path))
        att_resource = self.import_resource(sbt_path)
        self.assertIsNotNone(att_resource)

        # Associate model to attribute resource
        self.assertTrue(att_resource.associate(model_resource))

        # Create attributes for intanced views
        util.create_instanced_atts(att_resource)

        # Populate simulation attributes
        bc_lookup = {
            1: 'Electric',
            2: 'Electric'
        }
        bc_default = 'Exterior'
        uuids = model_resource.entitiesMatchingFlags(smtk.model.FACE, True)
        for uuid in uuids:
            # print('UUID {}'.format(uuid))
            prop_list = model_resource.integerProperty(uuid, 'pedigree id')
            face_id = prop_list[0]
            # print('Face ID {}'.format(face_id))
            bc_type = bc_lookup.get(face_id, bc_default)
            bc_att = att_resource.createAttribute(bc_type)
            self.assertIsNotNone(bc_att)
            self.assertTrue(bc_att.associateEntity(uuid))

        finfo_att = att_resource.findAttributes('FrequencyInfo')[0]
        num_item = finfo_att.findInt('NumEigenvalues')
        self.assertTrue(num_item.setValue(3))

        # Get export operator
        py_path = os.path.join(
            smtk.testing.DATA_DIR, 'simulation-workflows', 'ACE3P', 'ACE3P.py')
        self.assertTrue(os.path.exists(py_path))
        export_op = self.import_python_op(py_path)
        self.assertIsNotNone(export_op)

        # Setup export
        params_att = export_op.parameters()
        att_item = params_att.findResource("attributes")
        att_item.setValue(att_resource)

        model_item = params_att.findResource("model")
        model_item.setValue(model_resource)

        analysis_item = params_att.findString('Analysis')
        analysis_item.setValue('omega3p')

        # Set MeshFile to the input (genesis) file
        file_item = params_att.findFile('MeshFile')
        file_item.setValue(gen_path)

        # Set the output items
        export_path = os.path.join(smtk.testing.TEMP_DIR, OUTPUT_FOLDER)
        dir_item = analysis_item.findChild(
            'OutputFolder', smtk.attribute.SearchStyle.IMMEDIATE_ACTIVE)
        dir_item.setValue(export_path)

        prefix_item = analysis_item.findChild(
            'OutputFilePrefix', smtk.attribute.SearchStyle.IMMEDIATE_ACTIVE)
        prefix_item.setValue(MODEL_NAME)

        # Run the operator
        result = export_op.operate()
        outcome = result.findInt('outcome')
        self.assertEqual(outcome.value(), OPERATION_SUCCEEDED)

        # Check that expected file was created
        expected_filename = '{}.omega3p'.format(MODEL_NAME)
        expected_path = os.path.join(export_path, expected_filename)
        self.assertTrue(os.path.exists(expected_path))

        # Sanity check number of lines
        num_lines = len(open(expected_path).readlines())
        self.assertGreater(num_lines, 12)

        # (On success) remove the export folder
        shutil.rmtree(export_path)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
