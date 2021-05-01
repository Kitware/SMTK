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
import os

import smtk
import smtk.model
import smtk.operation
import smtk.session.polygon
import smtk.io
import smtk.testing

ppg_text = """
# Example input (Kitware's "Test2D" geometry)
# Note that vertex indices start with 1
# Vertices 1-8 for the outer polygon
v  0.0  2.0
v  1.0  0.0
v  9.0  0.0
v  9.0  2.0
v  8.0  4.0
v  6.0  5.0
v  3.0  5.0
v  1.0  4.0
# Vertices 9-12 for the inner polygon (hole)
v  7.0  1.0
v  8.0  1.0
v  8.0  2.0
v  7.0  2.0
# Outer polygon
f  1 2 3 4 5 6 7 8
# Inner polygon (hole)
h  9 10 11 12
"""

OP_SUCCEEDED = int(smtk.operation.Operation.Outcome.SUCCEEDED)


class TestImportPPG(smtk.testing.TestCase):
    def setUp(self):
        # Set file system path for resource file
        resource_filename = 'ppg2d.smtk'
        self.resource_path = os.path.join(
            smtk.testing.TEMP_DIR, resource_filename)

        # Delete file if one already exists
        if os.path.isfile(self.resource_path):
            os.remove(self.resource_path)

    def test_import_ppg(self):
        model_resource = self.create_model()
        self.check_model(model_resource)
        self.write_model(model_resource)

        del(model_resource)
        model_res2 = self.read_model(self.resource_path)
        self.check_model(model_res2)

    def check_model(self, model_resource):
        vlist = model_resource.entitiesMatchingFlags(smtk.model.VERTEX)
        self.assertEqual(len(vlist), 12, 'Expected 12 model vertices')
        elist = model_resource.entitiesMatchingFlags(smtk.model.EDGE)
        self.assertEqual(len(elist), 12, 'Expected 12 model edges')
        flist = model_resource.entitiesMatchingFlags(smtk.model.FACE)
        self.assertEqual(len(flist), 1, 'Expected 1 model face')

    def create_model(self):
        op = smtk.session.polygon.ImportPPG.create()
        op.parameters().findString('string').setIsEnabled(True)
        op.parameters().findString('string').setValue(ppg_text)
        result = op.operate()
        outcome = result.findInt('outcome').value()
        self.assertEqual(outcome, OP_SUCCEEDED,
                         'ImportPPG operation failed w/outcome {}'.format(outcome))

        resource = result.findResource('resource').value()
        model_resource = smtk.model.Resource.CastTo(resource)
        return model_resource

    def read_model(self, path=None):
        if path is None:
            path = self.resource_path
        op = smtk.session.polygon.Read.create()
        op.parameters().findFile('filename').setValue(path)
        result = op.operate()
        outcome = result.findInt('outcome').value()
        self.assertEqual(outcome, OP_SUCCEEDED,
                         'Read operation failed w/outcome {}'.format(outcome))

        resource = result.findResource('resource').value()
        model_resource = smtk.model.Resource.CastTo(resource)
        return model_resource

    def write_model(self, model_resource, path=None):
        if path is None:
            path = self.resource_path
        # Note: no filename parameter, so set path on the resource directly
        model_resource.setLocation(path)
        op = smtk.session.polygon.Write.create()
        op.parameters().associate(model_resource)
        result = op.operate()
        outcome = result.findInt('outcome').value()
        self.assertEqual(outcome, OP_SUCCEEDED,
                         'Write operation failed w/outcome {}'.format(outcome))

        # Sanity check
        self.assertTrue(os.path.isfile(path))
        print('Wrote file', path)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
