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
import smtk.extension.delaunay
import smtk.operation
import smtk.bridge.polygon
import smtk.testing
from smtk.simple import *


class UnitTriangulateFaces(smtk.testing.TestCase):

    def setUp(self):

        # Construct a resource manager
        self.resourceManager = smtk.resource.Manager.create()

        # Register polygon resource to the resource manager (this allows the
        # resource manager to track polygon resources)
        smtk.bridge.polygon.registerResources(self.resourceManager)

        # Construct an operation manager
        self.operationManager = smtk.operation.Manager.create()

        # Register operation and delaunay operations to the operation manager
        # (the former provides us with resource I/O, and the latter provides
        # us with the TriangulateFaces operator we wish to test.
        smtk.operation.registerOperations(self.operationManager)
        smtk.extension.delaunay.registerOperations(self.operationManager)

        # Register resource manager to the operation manager
        self.operationManager.registerResourceManager(self.resourceManager)

        # Set up the path to the test's input file
        modelFile = os.path.join(
            smtk.testing.DATA_DIR, 'mesh', '2d', 'boxWithHole.smtk')

        # Load the input file
        loadOp = self.operationManager.createOperation(
            'smtk::operation::LoadResource')
        loadOp.parameters().find('filename').setValue(modelFile)
        loadRes = loadOp.operate()

        # Access the resource
        self.resource = smtk.model.Manager.CastTo(
            loadRes.find('resource').value())

    def testMeshing2D(self):
        face = self.resource.findEntitiesOfType(int(smtk.model.FACE))[0]
        triangulateFace = self.operationManager.createOperation(
            'smtk::extension::delaunay::TriangulateFaces')
        triangulateFace.parameters().associateEntity(face)
        result = triangulateFace.operate()
        triangulatedFace = self.resource.meshes().associatedCollections(
            face)[0]
        assert(triangulatedFace.points().size() == 8)
        assert(triangulatedFace.cells().size() == 8)

        if self.interactive() and self.haveVTK() and self.haveVTKExtension():
            self.startRenderTest()
            self.addMeshToScene(triangulatedFace.meshes())
            cam = self.renderer.GetActiveCamera()
            self.renderer.ResetCamera()
            self.renderWindow.Render()
            self.interact()


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
