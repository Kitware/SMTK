#!/usr/bin/python
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
from __future__ import print_function
import os
import sys
import smtk
import smtk.bridge.discrete
import smtk.testing


class TestDiscreteCreateAndSaveEdges(smtk.testing.TestCase):

    def read(self, filename):
        readOp = smtk.bridge.discrete.ReadOperation.create()
        readOp.parameters().find('filename').setValue(filename)
        result = readOp.operate()
        self.assertEqual(
            result.find('outcome').value(0),
            int(smtk.operation.Operation.SUCCEEDED),
            'discrete read operation failed')

        # store the resource so it doesn't go out of scope
        self.mgr = smtk.model.Manager.CastTo(result.find('resource').value(0))
        return self.mgr.findEntitiesOfType(int(smtk.model.MODEL_ENTITY))

    def resetTestFiles(self):
        self.filesToTest = []

    def addExternalFile(self, pathStr, validator=None):
        self.filesToTest += [{'filename': pathStr, 'validator': validator}]

    def addTestFile(self, pathList, validator=None):
        self.addExternalFile(
            os.path.join(*([smtk.testing.DATA_DIR, ] + pathList)),
            validator)

    def validateCreateAndSaveEdges(self, model):

        if self.haveVTK() and self.haveVTKExtension():

            self.startRenderTest()

            mbs = self.addModelToScene(model)

            self.renderer.SetBackground(0.5, 0.5, 1)
            ac = self.renderer.GetActors()
            ac.InitTraversal()
            act = ac.GetNextActor()
            act.GetProperty().SetLineWidth(2)
            act.GetProperty().SetPointSize(8)
            act.GetMapper().SetResolveCoincidentTopologyToPolygonOffset()
            cam = self.renderer.GetActiveCamera()
            cam.SetFocalPoint(0., 0., 0.)
            cam.SetPosition(15, -10, -20)
            cam.SetViewUp(0, 1, 0)
            self.renderer.ResetCamera()
            self.renderWindow.Render()
            self.assertImageMatch(
                ['baseline', 'smtk', 'discrete', 'createedges-hybridModelOneCube.png'])
            self.interact()

    def setUp(self):
        import os
        import sys
        self.resetTestFiles()
        self.addTestFile(
            ['model', '3d', 'cmb', 'hybridModelOneCube.cmb'], self.validateCreateAndSaveEdges)
        self.shouldSave = False

    def verifyCreateAndSaveEdges(self, filename, validator):
        """Read a single file and validate that the operator worked.
        This is done by checking number of cells greater than zero
        reported by the output model as well as running an optional
        function on the model to do further model-specific testing."""

        print('\n\nFile: {fname}'.format(fname=filename))

        mod = smtk.model.Model(self.read(filename)[0])
        self.assertEqual(len(mod.cells()), 2,
                         'Expected {nc} free cells'.format(nc=2))

        btm = smtk.bridge.discrete.CreateEdgesOperation.create()
        self.assertIsNotNone(btm, 'Missing create edges operator.')
        btm.parameters().associate(mod.component())

        res = btm.operate()
        sys.stdout.flush()
        self.assertEqual(
            res.find('outcome').value(0),
            int(smtk.operation.Operation.SUCCEEDED),
            'create edges operation failed')

        writeop = smtk.bridge.discrete.WriteOperation.create()
        self.assertIsNotNone(writeop, 'Missing discrete write operator.')
        writeop.parameters().associate(mod.component())
        tmpfile = ['testCreateAndSaveEdges.cmb', ]
        outfilename = os.path.join(*([smtk.testing.TEMP_DIR, ] + tmpfile))
        writeop.parameters().find('filename').setValue(outfilename)
        res = writeop.operate()
        sys.stdout.flush()
        self.assertEqual(
            res.find('outcome').value(0),
            int(smtk.operation.Operation.SUCCEEDED),
            'write SimpleBox model with edges failed')

        mod = smtk.model.Model(self.read(outfilename)[0])
        self.assertEqual(len(mod.cells()), 2,
                         'Expected {nc} free cells'.format(nc=2))

        if validator:
            validator(mod)

        print('  Success')
        os.remove(outfilename)

    def testCreateAndSaveEdges(self):
        "Read each file named in setUp and validate the reader worked."
        for test in self.filesToTest:
            self.verifyCreateAndSaveEdges(test['filename'], test['validator'])

        if self.shouldSave:
            out = file('testcreateandsaveedges.json', 'w')
            print(smtk.io.SaveJSON.fromModelManager(self.mgr), file=out)
            out.close()


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
