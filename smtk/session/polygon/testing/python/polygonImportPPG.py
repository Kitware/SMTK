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

test2d = """
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

three_face = """
v 0 0
v 5 0
v 5 4
v 1 4
v 0 4
v 5 7
v 1 7

v 3 1
v 4 1
v 3 3

v 2 5
v 3 5
v 4 6
v 2 6
f 1 2 3 4 5    # face 1
f 3 6 7 4      # face 2
h 8 9 10       # hole in face 2
e 11 12 13 14  # face 3 embedded in face 2
"""

OP_SUCCEEDED = int(smtk.operation.Operation.Outcome.SUCCEEDED)


class TestImportPPG(smtk.testing.TestCase):
    def setUp(self):
        pass

    def test_test2d(self):
        path = os.path.join(smtk.testing.TEMP_DIR, 'test2d.smtk')

        # Remove existing file
        if os.path.isfile(path):
            os.remove(path)

        # Create model
        model_resource = self.create_model(test2d)
        self.check_model(model_resource, 12, 12, 1)

        # Write to file system
        self.write_model(model_resource, path)
        del (model_resource)

        # Read from file system
        model_res2 = self.read_model(path)
        self.check_model(model_res2, 12, 12, 1)

    def test_three_face(self):
        path = os.path.join(smtk.testing.TEMP_DIR, 'three-face.smtk')

        # Remove existing file
        if os.path.isfile(path):
            os.remove(path)

        # Create model
        model_resource = self.create_model(three_face)
        self.check_model(model_resource, 14, 15, 3)

        # Write to file system
        self.write_model(model_resource, path)
        del (model_resource)

        # Read from file system
        model_res2 = self.read_model(path)
        self.check_model(model_res2, 14, 15, 3)

    def check_model(self, model_resource, *expected):
        """Check number of entities

        Arguments:
            model_resource
            expected: 3 args with expected numbers of vertices, edges, faces
        """
        def check(model_resource, ent_type, expected, text):
            """Internal function to check one dimension."""
            ent_list = model_resource.entitiesMatchingFlags(ent_type)
            msg = 'Expected number of model {} {}, not {}'.format(
                text, expected, len(ent_list))
            self.assertEqual(len(ent_list), expected, msg)

        check(model_resource, smtk.model.VERTEX, expected[0], 'vertices')
        check(model_resource, smtk.model.EDGE, expected[1], 'edges')
        check(model_resource, smtk.model.FACE, expected[2], 'faces')

    def create_model(self, input_ppg):
        op = smtk.session.polygon.ImportPPG.create()
        op.parameters().findString('string').setIsEnabled(True)
        op.parameters().findString('string').setValue(input_ppg)
        result = op.operate()
        outcome = result.findInt('outcome').value()
        self.assertEqual(outcome, OP_SUCCEEDED,
                         'ImportPPG operation failed w/outcome {}'.format(outcome))

        resource = result.findResource('resourcesCreated').value()
        model_resource = smtk.model.Resource.CastTo(resource)
        return model_resource

    def read_model(self, path):
        op = smtk.session.polygon.Read.create()
        op.parameters().findFile('filename').setValue(path)
        result = op.operate()
        outcome = result.findInt('outcome').value()
        self.assertEqual(outcome, OP_SUCCEEDED,
                         'Read operation failed w/outcome {}'.format(outcome))

        resource = result.findResource('resourcesCreated').value()
        model_resource = smtk.model.Resource.CastTo(resource)
        return model_resource

    def write_model(self, model_resource, path):
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
