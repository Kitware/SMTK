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
import smtk.model
import smtk.bridge.rgg
import smtk.testing
from smtk.simple import *


class ReadRXFFileOp(smtk.testing.TestCase):

    def setUp(self):
        self.mgr = smtk.model.Manager.create()
        self.sess = self.mgr.createSession('rgg')
        SetActiveSession(self.sess)
        self.modelFiles = [
            os.path.join(smtk.testing.DATA_DIR, 'model', '3d', 'rgg', 'sampleCore.rxf')]

    def testReadRXFFileOp(self):
        for modelFile in self.modelFiles:
            op = self.sess.op('create model')
            res = op.operate()
            if res.findInt('outcome').value(0) != smtk.model.OPERATION_SUCCEEDED:
                raise ImportError
            self.model = res.findModelEntity('created').value(0)
            op = self.sess.op('read rxf file')
            fname = op.findFile('filename', int(smtk.attribute.ALL_CHILDREN))
            fname.setValue(modelFile)
            op.specification().associateEntity(self.model)
            res = op.operate()
            if res.findInt('outcome').value(0) != smtk.model.OPERATION_SUCCEEDED:
                raise RuntimeError


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
