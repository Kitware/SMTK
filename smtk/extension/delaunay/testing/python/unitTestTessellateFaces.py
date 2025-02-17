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
import smtk.session.polygon
import smtk.testing


class UnitTessellateFaces(smtk.testing.TestCase):

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
        tessellateFace = smtk.extension.delaunay.TessellateFaces.create()
        tessellateFace.parameters().associateEntity(face)
        result = tessellateFace.operate()
        tessellatedFace = face.hasTessellation()
        assert (len(tessellatedFace.coords()) == 8 * 3)
        assert (len(tessellatedFace.conn()) == 8 * 4)

        if self.interactive() and self.haveVTK() and self.haveVTKExtension():
            self.startRenderTest()
            face.setFloatProperty('color', [0, 1, 0, 1])
            self.addModelToScene(face.model())
            cam = self.renderer.GetActiveCamera()
            self.renderer.ResetCamera()
            self.renderWindow.Render()
            self.interact()


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
