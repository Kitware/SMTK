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
import smtk
from smtk.simple import *
import smtk.testing
import unittest
try:
    from vtkSMTKSourceExtPython import *
    from vtk import *
except:
    pass


class TestDisplayAnalysisMesh(smtk.testing.TestCase):
    """
    Create a face with both an analysis and a display tessellation; verify that
    the multiblock source can display both and that switching the source between
    the two tessellations works as expected.
    """

    def setUp(self):
        self.mgr = smtk.model.Manager.create()
        sref = self.mgr.createSession('native')
        SetActiveSession(sref)
        self.face = self.mgr.addFace()
        self.model = self.mgr.addModel(3, 3, 'Simple face')
        self.model.setSession(sref)
        self.model.addCell(self.face)

    def createTessellations(self, dgen, agen):
        dtess = smtk.model.Tessellation()
        amesh = smtk.model.Tessellation()
        [dtess.addCoords(x[0], x[1], x[2])
         for x in [(0, 0, 0), (2, 0, 0), (2, 2, 0), (0, 2, 0)]]
        [dtess.addTriangle(ii[0], ii[1], ii[2])
         for ii in [(0, 1, 2), (0, 2, 3)]]
        [amesh.addCoords(x[0], x[1], x[2]) for x in [
            (0, 0, 0), (1, 0.5, 0),  (2, 0, 0), (2, 2, 0), (1, 1.5, 0), (0, 2, 0)]]
        [amesh.addTriangle(ii[0], ii[1], ii[2])
         for ii in [(0, 1, 4), (0, 4, 5), (1, 2, 3), (1, 3, 4)]]
        dtessGen = self.face.setTessellationAndBoundingBox(dtess, 0)
        ameshGen = self.face.setTessellationAndBoundingBox(amesh, 1)
        self.assertEqual(
            dtessGen, dgen, 'Unexpected display tessellation generation {dg}'.format(dg=dtessGen))
        self.assertEqual(
            ameshGen, agen, 'Unexpected analyis mesh generation {ag}'.format(ag=ameshGen))

    def testSetTessellations(self):
        """Verify that tessellation generation numbers are updated."""
        self.createTessellations(0, 0)
        self.createTessellations(1, 1)

    def testDisplayBoth(self):
        "Draw both tessellations."
        self.createTessellations(0, 0)

        mbs1 = vtkModelMultiBlockSource()
        mbs1.SetModelManager(self.mgr.pointerAsString())
        mbs1.SetModelEntityID(str(self.model.entity()))
        mbs1.ShowAnalysisTessellationOff()

        mbs2 = vtkModelMultiBlockSource()
        mbs2.SetModelManager(self.mgr.pointerAsString())
        mbs2.SetModelEntityID(str(self.model.entity()))
        mbs2.ShowAnalysisTessellationOn()

        self.startRenderTest()
        self.addToScene(mbs1, translate=(-1.5, 0, 0))
        self.addToScene(mbs2, translate=(+1.5, 0, 0))

        self.renderer.ResetCamera()
        self.renderWindow.Render()
        self.assertImageMatch(['baseline', 'smtk', 'vtk', 'both-tess.png'])
        self.interact()

    def testSwitchingTessellations(self):
        """Verify that switching tessellations regenerates the VTK source's output."""
        self.createTessellations(0, 0)

        mbs = vtkModelMultiBlockSource()
        mbs.SetModelManager(self.mgr.pointerAsString())
        mbs.SetModelEntityID(str(self.model.entity()))
        mbs.ShowAnalysisTessellationOff()

        self.startRenderTest()
        self.addToScene(mbs)

        self.renderer.ResetCamera()
        self.renderWindow.Render()

        mbs.ShowAnalysisTessellationOn()
        self.renderWindow.Render()
        self.assertImageMatch(['baseline', 'smtk', 'vtk', 'analysis-tess.png'])
        self.interact()

        mbs.ShowAnalysisTessellationOff()
        self.renderWindow.Render()
        self.assertImageMatch(['baseline', 'smtk', 'vtk', 'display-tess.png'])
        self.interact()


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
