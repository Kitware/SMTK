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
import smtk.session.rgg.export_to_pyarc


def readFile(filename, mode="rt"):
    with open(filename, mode) as fin:
        return fin.read()


def deleteFile(filename):
    os.remove(filename)


class ExportToPyARC(smtk.testing.TestCase):

    def setUp(self):
        self.modelFile = os.path.join\
            (smtk.testing.DATA_DIR, 'model', '3d', 'rgg', 'sampleCore.rxf')
        self.targetFile = os.path.join\
            (smtk.testing.DATA_DIR, 'model',
             '3d', 'rgg', 'sampleCoreTest.son')
        self.validationFile = os.path.join \
            (smtk.testing.DATA_DIR, 'model', '3d', 'rgg', 'sampleCore.son')

    def testReadRXFFileOp(self):
        # create model
        op = smtk.session.rgg.CreateModel.create()
        res = op.operate()
        if res.findInt('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise ImportError
        self.model = res.find('created').objectValue(0)
        # read rxf file
        op = smtk.session.rgg.ReadRXFFile.create()
        fname = op.parameters().findFile(
            'filename', int(smtk.attribute.ALL_CHILDREN))
        fname.setValue(self.modelFile)
        op.parameters().associate(self.model)
        res = op.operate()
        if res.findInt('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise RuntimeError
        # export to PyARC
        op = smtk.session.rgg.export_to_pyarc.create()
        fname = op.findFile('filename', int(smtk.attribute.ALL_CHILDREN))
        fname.setValue(self.targetFile)
        op.parameters().associate(self.model)
        res = op.operate()
        if res.findInt('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise RuntimeError

        if readFile(self.validationFile) != readFile(self.targetFile):
            print "The generated file does not match simpleCore.son file!"
            deleteFile(self.targetFile)
            raise RuntimeError
        deleteFile(self.targetFile)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
