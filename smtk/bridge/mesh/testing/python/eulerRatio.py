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
            op = smtk.bridge.mesh.ImportOperator.create()
            fname = op.parameters().findFile('filename')
            fname.setValue(modelFile)
            res = op.operate()
            if res.findInt('outcome').value(0) != int(smtk.operation.NewOp.SUCCEEDED):
                raise ImportError
            modelEntity = res.find('created').value(0)
            self.model = smtk.model.Model(
                modelEntity.modelResource(), modelEntity.id())

            op = smtk.bridge.mesh.EulerCharacteristicRatio.create()
            op.parameters().associateEntity(self.model)
            res = op.operate()
            if res.findInt('outcome').value(0) != int(smtk.operation.NewOp.SUCCEEDED):
                raise RuntimeError
            value = res.findDouble('value').value(0)
            self.assertAlmostEqual(value, 2.)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
