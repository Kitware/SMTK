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


"""Test vector expression."""

import smtk
from smtk import attribute
from smtk import io
from smtk import operation

import smtk.testing
import os


class TestVectorExpression(smtk.testing.TestCase):

    def load_template(self):
        resource = smtk.attribute.Resource.create()
        logger = smtk.io.Logger()
        reader = smtk.io.AttributeReader()
        filename = os.path.join(smtk.testing.DATA_DIR, 'attribute',
                                'attribute_collection', 'VectorExpressionExample.sbt')
        status = reader.read(resource, filename, logger)
        print(
            '\n'.join([logger.record(i).message for i in range(logger.numberOfRecords())]))
        self.assertFalse(status, 'Could not read {}'.format(filename))
        return resource

    def build_resource(self, resource, smtk_path):
        # Create expression att
        exp_def = resource.findDefinition('vector-function')
        exp_att = resource.createAttribute('velocity_function1', exp_def)

        # Create bc att
        bc_def = resource.findDefinition('boundary-condition')
        bc_att = resource.createAttribute('velocity_bc', bc_def)

        velocity_item = bc_att.findDouble('velocity')
        self.assertTrue(velocity_item.setExpression(exp_att))
        self.assertTrue(velocity_item.isSet())

        # Write resource
        resource.setLocation(smtk_path)

        writer = smtk.attribute.Write.create()
        writer.parameters().associate(resource)
        self.assertTrue(writer.ableToOperate())

        result = writer.operate()
        outcome = result.findInt('outcome').value()
        self.assertEqual(outcome, int(
            smtk.operation.Operation.Outcome.SUCCEEDED))
        print('Wrote', smtk_path)

    def check_resource(self, smtk_path):
        reader = smtk.attribute.Read.create()
        reader.parameters().findFile('filename').setValue(smtk_path)
        result = reader.operate()
        outcome = result.findInt('outcome').value()
        self.assertEqual(outcome, int(
            smtk.operation.Operation.Outcome.SUCCEEDED))

        input_resource = result.findResource('resourcesCreated').value()
        self.assertIsNotNone(input_resource)

        bc_att = input_resource.findAttribute('velocity_bc')
        velocity_item = bc_att.findDouble('velocity')
        self.assertTrue(velocity_item.isSet())
        self.assertTrue(velocity_item.isExpression())

    def test_expression(self):
        if smtk.testing.DATA_DIR == '':
            self.skipTest('SMTK test-data directory not provided')
        resource = self.load_template()

        smtk_path = os.path.join(smtk.testing.TEMP_DIR, 'vector-example.smtk')
        self.build_resource(resource, smtk_path)
        resource = None

        self.check_resource(smtk_path)

        # Remove file
        os.remove(smtk_path)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
