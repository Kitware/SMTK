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
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.bridge.exodus
import smtk.testing
from smtk.simple import *
import sys


def floatColorToHex(fc):
    return '#' + ''.join(['{:02x}'.format(int(x * 255.0)) for x in fc])


class TestExodusSession(smtk.testing.TestCase):

    def setUp(self):
        import os
        self.filename = os.path.join(
            smtk.testing.DATA_DIR, 'model', '3d', 'exodus', 'disk_out_ref.ex2')

        self.mgr = smtk.model.Manager.create()
        sess = self.mgr.createSession('exodus')
        SetActiveSession(sess)

    def testRead(self):
        ents = Read(self.filename)
        self.model = smtk.model.Model(ents[0])

        # Verify that the model is marked as discrete
        self.assertEqual(
            self.model.geometryStyle(), smtk.model.DISCRETE,
            'Expected discrete model, got {gs}'.format(gs=self.model.geometryStyle()))

        # Verify that the file contains the proper number of groups.
        subgroups = self.model.groups()
        numGroups = len(subgroups)
        self.assertEqual(
            numGroups, 3, 'Expected 3 groups, found %d' % numGroups)

        numGroupMembersExpected = [1, 7, 3]
        allCells = []
        for i in range(len(numGroupMembersExpected)):
            numMembers = len(subgroups[i].members())
            self.assertEqual(
                numGroupMembersExpected[i], numMembers,
                'Expected {e} groups, found {a}'.format(e=numGroupMembersExpected[i], a=numMembers))
            allCells += subgroups[i].members()

        # Verify that the cell names match those from the Exodus file.
        nameset = {
            'Unnamed block ID: 1 Type: HEX8': '#5a5255',
            'Unnamed set ID: 1':              '#ae5a41',
            'Unnamed set ID: 2':              '#559e83',
            'Unnamed set ID: 3':              '#c3cb71',
            'Unnamed set ID: 4':              '#1b85b8',
            'Unnamed set ID: 5':              '#cb2c31',
            'Unnamed set ID: 6':              '#8b1ec4',
            'Unnamed set ID: 7':              '#ff6700'
        }
        self.assertTrue(all([x.name() in nameset for x in allCells]),
                        'Not all cell names recognized.')

        someCell = allCells[0]
        self.assertEqual(someCell.attributes(), set([]),
                         'Cell should not have any attribute associations.')
        asys = smtk.attribute.System.create()
        adef = asys.createDefinition('testDef')
        adef.setAssociationMask(int(smtk.model.CELL_ENTITY))
        adef.associationRule().setNumberOfRequiredValues(1)
        self.assertEqual(adef.associationRule().numberOfRequiredValues(), 1)
        attr = asys.createAttribute(adef.type())

        self.assertTrue(attr.associateEntity(someCell),
                        'Could not associate cell to attribute')
        self.assertEqual(someCell.attributes(), set([attr.id()]),
                         'Cell should have the assigned attribute.')

        # Verify that no cells which are not in the list above are present.
        cellNames = [x.name() for x in allCells]
        self.assertTrue(all([x in cellNames for x in nameset]),
                        'Some expected cell names not present.')

        # Count the number of each *type* of cell (node, face, volume)
        # print '\n'.join([str((x.name(), x.flagSummary())) for x in
        # allCells])
        cellTypes = [x.flagSummary() for x in allCells]
        gtc = {x: cellTypes.count(x) for x in cellTypes}
        expectedCellTypeCounts = {
            'vertex': 3,
            'face': 7,
            'volume': 1
        }
        for entry in gtc.items():
            print '%40s: %d' % entry
        self.assertEqual(gtc, expectedCellTypeCounts,
                         'At least one cell was of the wrong type.')

        if self.haveVTK() and self.haveVTKExtension():

            # Render cells with colors:
            for cell in allCells:
                color = self.hex2rgb(nameset[cell.name()])
                SetEntityProperty(cell, 'color', as_float=color)
                # The element block should not be shown as it is coincident with some
                # of the side sets and throws off baseline images. Remove its
                # tessellation.
                if cell.name() == 'Unnamed block ID: 1 Type: HEX8':
                    cell.setTessellation(smtk.model.Tessellation())

            self.startRenderTest()
            mbs, filt, mapper, actor = self.addModelToScene(self.model)

            self.renderer.SetBackground(1, 1, 1)
            cam = self.renderer.GetActiveCamera()
            cam.SetFocalPoint(0., 0., 0.)
            cam.SetPosition(19, 17, -43)
            cam.SetViewUp(-0.891963, -0.122107, -0.435306)
            self.renderer.ResetCamera()
            self.renderWindow.Render()
            self.interact()
            try:
                self.assertImageMatch(
                    ['baseline', 'smtk', 'exodus', 'disk_out_ref.png'])
            finally:
                self.interact()

        else:
            self.assertFalse(
                self.haveVTKExtension(),
                'Could not import vtk. Python path is {pp}'.format(pp=sys.path))


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
