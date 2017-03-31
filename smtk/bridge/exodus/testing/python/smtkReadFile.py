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
import smtk
from smtk.simple import *
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.model
    import smtk.bridge.exodus
import smtk.testing
import sys


class TestExodusSession(smtk.testing.TestCase):

    def testReadSMTK(self):
        import os
        smtk_file = os.path.join(
            smtk.testing.DATA_DIR, 'model', '3d', 'exodus', 'disk_out_ref.smtk')
        print 'smtk_file', smtk_file

        model_manager = smtk.model.Manager.create()
        sess = model_manager.createSession('exodus')
        sess.assignDefaultName()
        SetActiveSession(sess)

        LoadSMTKModel(smtk_file)
        result = GetLastResult()

        outcome = result.findInt('outcome').value(0)
        self.assertEqual(
            outcome,
            smtk.model.OPERATION_SUCCEEDED,
            'Import operation failed')

        model_ent = result.findModelEntity('created').value(0)
        model = smtk.model.Model(model_ent)
        self.assertTrue(model.isValid(), 'Imported model not valid')

        # Verify that the model is marked as discrete
        self.assertEqual(
            model.geometryStyle(), smtk.model.DISCRETE,
            'Expected discrete model, got {gs}'.format(gs=model.geometryStyle()))


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
