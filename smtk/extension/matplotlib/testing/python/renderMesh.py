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
import smtk.extension.matplotlib
import smtk.testing
from smtk.simple import *


class RenderMesh(smtk.testing.TestCase):

    def setUp(self):
        self.mgr = smtk.model.Manager.create()
        self.meshmgr = self.mgr.meshes()
        self.sess = self.mgr.createSession('native')
        SetActiveSession(self.sess)
        self.meshFile = os.path.join(
            smtk.testing.DATA_DIR, 'mesh', '2d', 'warpedMesh.h5m')
        self.collection = smtk.io.importMesh(self.meshFile, self.meshmgr)

    def testRenderMesh(self):
        renderMesh = self.sess.op('render mesh')
        renderMesh.specification().findMesh(
            'mesh').setValue(self.collection.meshes())
        self.outFile = os.path.join(
            smtk.testing.TEMP_DIR, str(smtk.common.UUID.random()) + '.png')
        renderMesh.specification().findFile('filename').setValue(self.outFile)
        result = renderMesh.operate()

        if self.interactive() and self.haveVTK() and self.haveVTKExtension():
            import vtk
            self.startRenderTest()
            reader = vtk.vtkPNGReader()
            reader.SetFileName(self.outFile)
            reader.Update()
            self.addImageToScene(reader)
            cam = self.renderer.GetActiveCamera()
            self.renderer.ResetCamera()
            self.renderWindow.Render()
            self.interact()


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
