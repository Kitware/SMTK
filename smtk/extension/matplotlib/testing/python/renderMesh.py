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
import smtk.io
import smtk.mesh
import smtk.model
import smtk.extension.matplotlib.render_mesh
import smtk.testing
from smtk.simple import *


class RenderMesh(smtk.testing.TestCase):

    def setUp(self):

        # Construct an operation manager
        self.operationManager = smtk.operation.Manager.create()

        # Register matplotlib operations to the operation manager
        smtk.extension.matplotlib.registerOperations(self.operationManager)

        # Set up the path to the test's input file
        meshFile = os.path.join(
            smtk.testing.DATA_DIR, 'mesh', '2d', 'warpedMesh.h5m')

        # Read in the mesh
        self.collection = smtk.mesh.Collection.create()
        smtk.io.readMesh(meshFile, self.collection)

    def testRenderMesh(self):

        # Create a "render mesh" operator
        renderMesh = self.operationManager.createOperator(
            'smtk.extension.matplotlib.render_mesh.RenderMesh')

        if not renderMesh:
            raise ImportError('Could not find operator \'render mesh\'')

        # Set the operator's parameters
        print renderMesh.parameters()
        renderMesh.parameters().find('mesh').setValue(self.collection.meshes())

        self.outFile = os.path.join(
            smtk.testing.TEMP_DIR, str(smtk.common.UUID.random()) + '.png')

        renderMesh.parameters().find('filename').setValue(self.outFile)

        # Execute the operator
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
