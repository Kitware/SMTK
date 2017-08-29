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
import sys
import unittest
import smtk
import smtk.mesh
import smtk.model
import smtk.bridge.mesh
import smtk.testing
import smtk.io.vtk
from smtk.simple import *


class EulerRatio(smtk.testing.TestCase):

    def setUp(self):
        self.mgr = smtk.model.Manager.create()
        self.meshmgr = self.mgr.meshes()
        self.sess = self.mgr.createSession('mesh')
        SetActiveSession(self.sess)
        self.modelFiles = [
            os.path.join(
                smtk.testing.DATA_DIR, 'mesh', '3d', 'nickel_superalloy.vtu'),
            os.path.join(smtk.testing.DATA_DIR, 'mesh', '3d', 'cube.exo'),
            os.path.join(
                smtk.testing.DATA_DIR, 'model', '3d', 'genesis', 'gun-1fourth.gen'),
            os.path.join(
                smtk.testing.DATA_DIR, 'mesh', '3d', 'genesis', 'cube-hex.gen'),
            os.path.join(
                smtk.testing.DATA_DIR, 'mesh', '3d', 'genesis', 'cube-tet.gen'),
            os.path.join(
                smtk.testing.DATA_DIR, 'mesh', '3d', 'genesis', 'cube-tet10.gen')
        ]

    def testMeshing2D(self):
        for modelFile in self.modelFiles:
            op = self.sess.op('import')
            fname = op.findFile('filename', int(smtk.attribute.ALL_CHILDREN))
            fname.setValue(modelFile)
            op.findVoid('construct hierarchy', int(
                smtk.attribute.ALL_CHILDREN)).setIsEnabled(False)
            res = op.operate()
            if res.findInt('outcome').value(0) != smtk.model.OPERATION_SUCCEEDED:
                raise ImportError
            self.model = res.findModelEntity('created').value(0)

            op = self.sess.op('euler characteristic ratio')
            op.specification().associateEntity(self.model)
            res = op.operate()
            if res.findInt('outcome').value(0) != smtk.model.OPERATION_SUCCEEDED:
                raise RuntimeError
            value = res.findDouble('value').value(0)
            self.assertAlmostEqual(value, 2.)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
