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

    def setUp(self):
        self.filename = os.path.join(
            smtk.testing.DATA_DIR, 'model', '3d', 'netcdf', 'pillbox.ncdf')

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
            for i in range(len(vlist)):
                intVal.setValue(i, vlist[i])
        if 'as_float' in kwargs:
            vlist = kwargs['as_float']
            if not hasattr(vlist, '__iter__'):
                vlist = [vlist, ]
            floatVal = spr.parameters().find('float value')
            floatVal.setNumberOfValues(len(vlist))
            for i in range(len(vlist)):
                floatVal.setValue(i, vlist[i])
        if 'as_string' in kwargs:
            vlist = kwargs['as_string']
            if not hasattr(vlist, '__iter__'):
                vlist = [vlist, ]
            stringVal = spr.parameters().find('string value')
            stringVal.setNumberOfValues(len(vlist))
            for i in range(len(vlist)):
                stringVal.setValue(i, vlist[i])
        res = spr.operate()
        self.assertEqual(
            res.find('outcome').value(0),
            int(smtk.operation.Operation.SUCCEEDED),
            'set property failed')
        return res.findInt('outcome').value(0)

    def testRead(self):
        # Import the input file
        importOp = smtk.session.vtk.Import.create()
        importOp.parameters().find('filename').setValue(self.filename)
        importOp.parameters().find('filetype').setValue('slac')
        importRes = importOp.operate()
        self.assertEqual(importRes.find('outcome').value(0),
                         int(smtk.operation.Operation.SUCCEEDED),
                         'import model failed')

        # Access the resource
        resource = smtk.model.Resource.CastTo(
            importRes.find('resourcesCreated').value())
        self.model = resource.findEntitiesOfType(
            int(smtk.model.MODEL_ENTITY))[0]

        # Verify that the model is marked as discrete
        self.assertEqual(
            self.model.geometryStyle(), smtk.model.DISCRETE,
            'Expected discrete model, got {gs}'.format(gs=self.model.geometryStyle()))

        # Verify that the file contains the proper number of groups.
        subgroups = self.model.groups()
        numGroups = len(subgroups)
        self.assertEqual(
            numGroups, 0, 'Expected 0 groups, found %d' % numGroups)

        expectedCellsOfDim = [0, 0, 5, 1]
        allCells = self.model.cells()
        numCellsOfDim = [0, 0, 0, 0]
        for cell in allCells:
            numCellsOfDim[cell.dimension()] += 1
        for dim in range(len(expectedCellsOfDim)):
            self.assertEqual(expectedCellsOfDim, numCellsOfDim,
                             'Expected {e} cells of dimension {d}, found {f}'.format(d=dim, e=expectedCellsOfDim[dim], f=numCellsOfDim[dim]))

        print('\n'.join([x.name() for x in allCells]))
        # Verify that the cell names match those from the Exodus file.
        nameset = {
            'surface 1':                     '#5a5255',
            'surface 2':                     '#ae5a41',
            'surface 3':                     '#559e83',
            'surface 4':                     '#c3cb71',
            'surface 6':                     '#1b85b8',
            'volume 1':                      '#cb2c31'
        }
        self.assertTrue(all([x.name() in nameset for x in allCells]),
                        'Not all cell names recognized.')
        # Verify that no cells which are not in the list above are present.
        cellNames = [x.name() for x in allCells]
        self.assertTrue(all([x in cellNames for x in nameset]),
                        'Some expected cell names not present.')

        # Count the number of each *type* of cell (node, face, volume)
        # print('\n'.join([str((x.name(), x.flagSummary())) for x in
        # allCells]))
        cellTypes = [x.flagSummary() for x in allCells]
        gtc = {x: cellTypes.count(x) for x in cellTypes}
        expectedCellTypeCounts = {
            'face': 5,
            'volume': 1
        }
        for entry in gtc.items():
            print('%40s: %d' % entry)
        self.assertEqual(gtc, expectedCellTypeCounts,
                         'At least one cell was of the wrong type.')

        if self.haveVTK() and self.haveVTKExtension():

            # Render cells with colors:
            for cell in allCells:
                color = self.hex2rgb(nameset[cell.name()])
                self.setEntityProperty(cell, 'color', as_float=color)
                # Prevent z-fighting by removing the volume's tessellation:
                if cell.name() == 'volume 1':
                    cell.setTessellation(smtk.model.Tessellation())

            self.startRenderTest()
            mbs = self.addModelToScene(self.model)

            self.renderer.SetBackground(1, 1, 1)
            cam = self.renderer.GetActiveCamera()
            cam.SetFocalPoint(0., 0., 0.)
            cam.SetPosition(1, 0, 1)
            cam.SetViewUp(0, 1, 0)
            self.renderer.ResetCamera()
            self.renderWindow.Render()
            self.assertImageMatch(
                ['baseline', 'smtk', 'exodus', 'pillbox.png'])
            self.interact()

        else:
            self.assertFalse(
                self.haveVTKExtension(),
                'Could not import vtk. Python path is {pp}'.format(pp=sys.path))


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
