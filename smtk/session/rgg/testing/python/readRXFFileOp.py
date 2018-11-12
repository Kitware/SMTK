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
import smtk.session.rgg
import smtk.testing


class ReadRXFFileOp(smtk.testing.TestCase):

    def setUp(self):
        self.modelFiles = [
            os.path.join(smtk.testing.DATA_DIR, 'model', '3d', 'rgg', 'sampleCore.rxf')]

    def testReadRXFFileOp(self):
        for modelFile in self.modelFiles:
            op = smtk.session.rgg.CreateModel.create()
            res = op.operate()
            if res.findInt('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
                raise ImportError
            self.model = res.find('created').objectValue(0)
            op = smtk.session.rgg.ReadRXFFile.create()
            fname = op.parameters().find('filename')
            fname.setValue(modelFile)
            op.parameters().associate(self.model)
            res = op.operate()
            if res.findInt('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
                raise RuntimeError


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
