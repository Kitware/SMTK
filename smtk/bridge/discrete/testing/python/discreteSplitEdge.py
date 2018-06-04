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


def hex2rgb(hexstr):
    hh = hexstr[1:] if hexstr[0] == '#' else hexstr
    rr = int(hh[0:2], 16) / 255.
    gg = int(hh[2:4], 16) / 255.
    bb = int(hh[4:6], 16) / 255.
    return (rr, gg, bb)


class TestDiscreteSplitEdge(smtk.testing.TestCase):

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

    def setEntityProperty(self, ents, propName, **kwargs):
        """Set a property value (or vector of values) on an entity (or vector of entities).

        You may pass any combination of "as_int", "as_float", or "as_string" as named
        arguments specifying the property values. The values of these named arguments may
        be a single value or a list of values. Values will be coerced to the named type.

        Example:

          SetEntityProperty(face, 'color', as_float=(1., 0., 0.))
          SetEntityProperty(edge, 'name', as_string='edge 20')
          SetEntityProperty(body, 'visited', as_int='edge 20')
        """
        spr = smtk.model.SetProperty.create()
        if hasattr(ents, '__iter__'):
            [spr.parameters().associate(ent.component()) for ent in ents]
        else:
            spr.parameters().associate(ents.component())
        spr.parameters().find('name').setValue(propName)
        if 'as_int' in kwargs:
            vlist = kwargs['as_int']
            if not hasattr(vlist, '__iter__'):
                vlist = [vlist, ]
            intVal = spr.parameters().find('integer value')
            intVal.setNumberOfValues(len(vlist))
            for i in xrange(len(vlist)):
                intVal.setValue(i, vlist[i])
        if 'as_float' in kwargs:
            vlist = kwargs['as_float']
            if not hasattr(vlist, '__iter__'):
                vlist = [vlist, ]
            floatVal = spr.parameters().find('float value')
            floatVal.setNumberOfValues(len(vlist))
            for i in xrange(len(vlist)):
                floatVal.setValue(i, vlist[i])
        if 'as_string' in kwargs:
            vlist = kwargs['as_string']
            if not hasattr(vlist, '__iter__'):
                vlist = [vlist, ]
            stringVal = spr.parameters().find('string value')
            stringVal.setNumberOfValues(len(vlist))
            for i in xrange(len(vlist)):
                stringVal.setValue(i, vlist[i])
        res = spr.operate()
        self.assertEqual(
            res.find('outcome').value(0),
            int(smtk.operation.Operation.SUCCEEDED),
            'set property failed')
        return res.findInt('outcome').value(0)

    def resetTestFiles(self):
        self.filesToTest = []

    def addExternalFile(self, pathStr, splits, validator=None):
        self.filesToTest += [{'filename': pathStr,
                              'splits': splits, 'validator': validator}]

    def addTestFile(self, pathList, splits, validator=None):
        self.addExternalFile(
            os.path.join(*([smtk.testing.DATA_DIR, ] + pathList)),
            splits, validator)

    def findSplitsTest2D(self, model):
        "Find a repeatable edge to split in test2D.cmb"
        faces = [smtk.model.Face(x) for x in model.cells()]
        f4l = [f for f in faces if f.name() == 'Face4']
        self.assertEqual(len(f4l), 1, 'Could not find test2D "Face4"')
        face4 = f4l[0]
        outer = face4.positiveUse().loops()
        self.assertEqual(len(outer), 1, 'Face4 should have 1 outer loop')
        inner = outer[0].containedLoops()
        self.assertEqual(
            len(inner), 1, 'Face4\'s outer loop should have 1 inner loop')
        self.assertEqual(len(inner[0].edgeUses()), 1,
                         'Face4\'s inner loop should have 1 edge use')
        innerEdge = inner[0].edgeUses()[0].edge()
        self.assertEqual(innerEdge.name(), 'Edge10',
                         'Face4\'s inner loop should one edge named "Edge10"')
        # We will split Edge10 at an inner vertex of its Tessellation.
        etess = innerEdge.hasTessellation()
        # We should verify that etess.conn()
        self.assertEqual(
            etess.conn(), [
                2048, 2, 0, 1, 2048, 2, 1, 2, 2048, 2, 2, 3, 2048, 2, 3, 0],
                         'Unexpected connectivity for Edge10')
        splits = [(innerEdge.entity(), [2, ]), ]
        return splits

    def validateTest2D(self, model):
        "Verify that the test2D model is imported correctly."
        faces = [smtk.model.Face(x) for x in model.cells()]
        f4l = [f for f in faces if f.name() == 'Face4']
        self.assertEqual(len(f4l), 1, 'Could not find test2D "Face4"')
        face4 = f4l[0]
        outer = face4.positiveUse().loops()
        self.assertEqual(len(outer), 1, 'Face4 should have 1 outer loop')
        inner = outer[0].containedLoops()
        self.assertEqual(
            len(inner), 1, 'Face4\'s outer loop should have 1 inner loop')
        self.assertEqual(
            len(inner[0].edgeUses()), 2, 'Face4\'s inner loop should now have 2 edge uses')
        innerNames = [eu.edge().name() for eu in inner[0].edgeUses()]
        self.assertIn('Edge10', innerNames, 'Expected Edge10 to remain')
        self.assertIn('Edge11', innerNames, 'Expected Edge11 to appear')

        if self.haveVTK() and self.haveVTKExtension():

            self.startRenderTest()

            # Color some things on the model before we return.
            # This will help with debugging and verify that
            # color properties are preserved.
            entityColors = {
                'Face4':             '#875d4f',
                'Face1':             '#896f59',
                'Face2':             '#a99b86',
                'Face3':             '#5c3935',

                'Edge9':             '#093020',
                'Edge10':            '#0e433b',
                'Edge11':            '#104c57'
            }
            for (name, color) in entityColors.iteritems():
                self.setEntityProperty(
                    self.mgr.findEntitiesByProperty('name', name),
                    'color', as_float=hex2rgb(color))

            mbs = self.addModelToScene(model)

            self.renderer.SetBackground(0.5, 0.5, 1)
            ac = self.renderer.GetActors()
            ac.InitTraversal()
            act = ac.GetNextActor()
            act.GetProperty().SetLineWidth(2)
            act.GetProperty().SetPointSize(8)
            act.GetMapper().SetResolveCoincidentTopologyToPolygonOffset()

            cam = self.renderer.GetActiveCamera()
            self.renderer.ResetCamera()
            self.renderWindow.Render()
            self.assertImageMatch(
                ['baseline', 'smtk', 'discrete', 'edge-split-test2D.png'])
            self.interact()

    def setUp(self):
        import os
        import sys
        self.resetTestFiles()
        self.addTestFile(['model', '2d', 'cmb', 'test2D.cmb'],
                         self.findSplitsTest2D, self.validateTest2D)
        self.shouldSave = False

    def verifySplitEdge(self, filename, findSplits, validator):
        """Read a single file and validate that the operator worked.
        This is done by checking loops and shells around the split vertex
        reported by the output model as well as running an optional
        function on the model to do further model-specific testing."""

        print('\n\nFile: {fname}'.format(fname=filename))

        mod = smtk.model.Model(self.read(filename)[0])

        # Find the edges to split for this given model
        splits = findSplits(mod)

        spl = smtk.bridge.discrete.EdgeOperation.create()
        self.assertIsNotNone(spl, 'Missing modify edge operator.')
        spl.parameters().associate(mod.component())
        sel = spl.parameters().findMeshSelection('selection')
        sel.setModifyMode(smtk.attribute.ACCEPT)
        [sel.setValues(ent, set(tess)) for (ent, tess) in splits]
        res = spl.operate()
        self.assertEqual(
            res.findInt('outcome').value(0), int(
                smtk.operation.Operation.SUCCEEDED),
                         'Split failed.')

        if validator:
            validator(mod)

        print('  Success')

    def testSplitEdge(self):
        "Read each file named in setUp and validate the reader worked."
        for test in self.filesToTest:
            self.verifySplitEdge(
                test['filename'], test['splits'], test['validator'])

        if self.shouldSave:
            out = file('test.json', 'w')
            print(smtk.io.SaveJSON.fromModelManager(self.mgr), file=out)
            out.close()


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
