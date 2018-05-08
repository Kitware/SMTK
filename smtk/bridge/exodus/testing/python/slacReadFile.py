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
import smtk
import smtk.bridge.exodus
import smtk.testing
from smtk.simple import *
import sys


class TestExodusSession(smtk.testing.TestCase):

    def setUp(self):
        import os
        self.filename = os.path.join(
            smtk.testing.DATA_DIR, 'model', '3d', 'netcdf', 'pillbox.ncdf')

        self.mgr = smtk.model.Manager.create()
        sess = self.mgr.createSession('exodus')
        SetActiveSession(sess)

    def testRead(self):
        ents = Read(self.filename, filetype='slac')
        self.model = smtk.model.Model(ents[0])

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
                SetEntityProperty(cell, 'color', as_float=color)
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
