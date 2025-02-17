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
import sys
import unittest
import smtk
import smtk.mesh
import smtk.model
import smtk.extension.delaunay
import smtk.operation
import smtk.session.polygon
import smtk.testing


class UnitTriangulateFaces(smtk.testing.TestCase):

    def setUp(self):
        # Set up the path to the test's input file
        modelFile = os.path.join(
            smtk.testing.DATA_DIR, 'mesh', '2d', 'boxWithHole.smtk')

        # Load the input file
        loadOp = smtk.session.polygon.LegacyRead.create()
        loadOp.parameters().find('filename').setValue(modelFile)
        loadRes = loadOp.operate()

        # Access the resource
        self.resource = smtk.model.Resource.CastTo(
            loadRes.find('resourcesCreated').value())

    def testMeshing2D(self):
        face = self.resource.findEntitiesOfType(int(smtk.model.FACE))[0]
        triangulateFace = smtk.extension.delaunay.TriangulateFaces.create()
        triangulateFace.parameters().associateEntity(face)
        result = triangulateFace.operate()
        triangulatedFace = result.find("meshresource").value()
        assert (triangulatedFace.points().size() == 8)
        assert (triangulatedFace.cells().size() == 8)

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
