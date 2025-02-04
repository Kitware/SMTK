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
import smtk.session.vtk
import smtk.testing
import sys


class TestVTKSession(smtk.testing.TestCase):

    def testReadSMTK(self):
        smtk_file = os.path.join(
            smtk.testing.DATA_DIR, 'model', '3d', 'exodus', 'disk_out_ref.smtk')
        print('smtk_file', smtk_file)

        # Import the input file
        readOp = smtk.session.vtk.LegacyRead.create()
        readOp.parameters().find('filename').setValue(smtk_file)
        readRes = readOp.operate()
        self.assertEqual(readRes.find('outcome').value(0),
                         int(smtk.operation.Operation.SUCCEEDED),
                         'read model failed')

        # Access the resource
        resource = smtk.model.Resource.CastTo(
            readRes.find('resourcesCreated').value())
        models = resource.findEntitiesOfType(int(smtk.model.MODEL_ENTITY))
        self.assertEqual(len(models), 1)
        model = models[0]

        self.assertTrue(model.isValid(), 'Imported model not valid')

        # Verify that the model is marked as discrete
        self.assertEqual(
            model.geometryStyle(), smtk.model.DISCRETE,
            'Expected discrete model, got {gs}'.format(gs=model.geometryStyle()))


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
