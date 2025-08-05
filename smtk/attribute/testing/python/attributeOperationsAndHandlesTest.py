import sys
import os
# =============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
# =============================================================================

import smtk
import smtk.string
import smtk.common
import smtk.io
import smtk.resource
import smtk.attribute
import smtk.operation
import smtk.view
from smtk import attribute
from smtk import io

import smtk.testing


class TestAttributeOperationsAndHandles(smtk.testing.TestCase):

    def setUp(self):
        print('Setup')
        if smtk.testing.DATA_DIR == '':
            self.skipTest('SMTK test-data directory not provided')

        self.app = smtk.applicationContext()
        print(f'    Application context? {self.app != None}')
        self.resource_mgr = self.app.get('smtk.resource.Manager')
        self.operation_mgr = self.app.get('smtk.operation.Manager')
        print(f'    Resource manager? {self.resource_mgr != None}')
        print(f'    Operation manager? {self.operation_mgr != None}')
        if self.resource_mgr == None:
            # This test should only be run if ParaView support was enabled.
            raise Exception('Required plugins were not loaded.')

        # Attribute registrar
        print('    Attribute registrar')
        smtk.attribute.Registrar.registerTo(self.resource_mgr)
        smtk.attribute.Registrar.registerTo(self.operation_mgr)
        # Operation registrar
        print('    Operation registrar')
        smtk.operation.Registrar.registerTo(self.operation_mgr)

        print('    Resource')
        self.resource = self.resource_mgr.createResource(
            'smtk::attribute::Resource')
        print('        created.')
        logger = smtk.io.Logger()
        reader = smtk.io.AttributeReader()
        filenm = os.path.join(smtk.testing.DATA_DIR, 'attribute',
                              'attribute_collection', 'HydraTemplateV1.sbt')
        status = reader.read(self.resource, filenm, logger)
        print('        read from template.')
        print(
            '\n'.join([logger.record(i).message for i in range(logger.numberOfRecords())]))
        self.assertFalse(status, 'Could not read {fn}'.format(fn=filenm))

    def checkAttributeCreation(self, op, result):
        """When the creation operation launched by testAttributeOperations() completes,
        this method is called to test its success."""
        outcome = smtk.operation.outcome(result)
        self.assertEqual(
            outcome, smtk.operation.Operation.Outcome.SUCCEEDED, 'Creation failed')
        self.att = result.findComponent('created').values()[0]
        self.attHandle = smtk.view.HandleManager.instance().handle(self.att)
        print(f'    Attribute {str(self.att)} handle {self.attHandle}')

    def checkAttributeRename(self, op, result):
        """When the rename operation launched by testAttributeOperations() completes,
        this method is called to test its success."""
        outcome = smtk.operation.outcome(result)
        self.assertEqual(
            outcome, smtk.operation.Operation.Outcome.SUCCEEDED, 'Rename failed')
        self.assertEqual(self.att, smtk.view.HandleManager.instance().object(self.attHandle),
                         'Attribute should still be fetchable from handle.')
        self.assertEqual(self.att.name(), 'thingy',
                         'Name should have been changed.')

    def checkAttributeDeletion(self, op, result):
        """When the deletion operation launched by testAttributeOperations() completes,
        this method is called to test its success."""
        outcome = smtk.operation.outcome(result)
        op.log().setFlushToStdout(True)
        self.assertEqual(
            outcome, smtk.operation.Operation.Outcome.SUCCEEDED, 'Delete failed')

    def handleEvents(self, handleMap, event):
        """Called when handles are created/modified/expunged."""
        print('   Handle updates', event, handleMap)

    def testAttributeOperations(self):
        print('1. Insert handle observer')
        key = smtk.view.HandleManager.instance().observers().insert(
            self.handleEvents, 0, 'Handle observer')
        print(f'   Key assigned? {key.assigned()}')

        # Test creation
        print('2. Create an attribute')
        op = self.operation_mgr.createOperation(
            'smtk::attribute::CreateAttribute')
        print(f'   Operation? {op != None}')
        op.parameters().associate(self.resource)
        op.parameters().findString('definition').setValue('hydrostat')
        print(f'   Operation able to run? {op.ableToOperate()}')
        res = op.operate()
        print(f'   Operation outcome: {smtk.operation.outcome(res)}')
        self.checkAttributeCreation(op, res)

        # Test rename
        print('3. Rename an attribute')
        op = self.operation_mgr.createOperation(
            'smtk::attribute::RenameAttribute')
        print(f'   Operation? {op != None}')
        op.parameters().associate(self.att)
        op.parameters().findString('name').appendValue('thingy')
        print(f'   Operation able to run? {op.ableToOperate()}')
        res = op.operate()
        print(f'   Operation outcome: {smtk.operation.outcome(res)}')
        self.checkAttributeRename(op, res)

        # Test deletion
        print('4. Delete an attribute')
        op = self.operation_mgr.createOperation(
            'smtk::attribute::DeleteAttribute')
        print(f'   Operation? {op != None}')
        op.parameters().associate(self.att)
        print(f'   Operation able to run? {op.ableToOperate()}')
        res = op.operate()
        print(f'   Operation outcome: {smtk.operation.outcome(res)}')
        self.checkAttributeDeletion(op, res)
        self.assertEqual(None, smtk.view.HandleManager.instance().object(self.attHandle),
                         'Attribute should not still be fetchable from handle.')


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
