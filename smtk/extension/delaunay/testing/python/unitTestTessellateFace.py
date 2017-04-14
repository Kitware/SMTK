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
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.mesh
    import smtk.model
    import smtk.model.delaunay
    import smtk.bridge.polygon
import smtk.testing
from smtk.simple import *


class UnitTessellateFace(smtk.testing.TestCase):

    def setUp(self):
        self.mgr = smtk.model.Manager.create()
        self.meshmgr = self.mgr.meshes()
        self.sess = self.mgr.createSession('polygon')
        SetActiveSession(self.sess)
        self.modelFile = os.path.join(
            smtk.testing.DATA_DIR, 'mesh', '2d', 'boxWithHole.smtk')
        self.model = LoadSMTKModel(self.modelFile)[0]

    def testMeshing2D(self):
        if smtk.wrappingProtocol() == 'pybind11':
            face = self.mgr.findEntitiesOfType(int(smtk.model.FACE))[0]
        else:
            face = self.mgr.findEntitiesOfType(smtk.model.FACE, True)[0]
        tessellateFace = self.sess.op('tessellate face')
        tessellateFace.specification().associateEntity(self.model)
        tessellateFace.specification().findModelEntity("face").setValue(face)
        result = tessellateFace.operate()
        tessellatedFace = face.hasTessellation()
        assert(len(tessellatedFace.coords()) == 8*3)
        assert(len(tessellatedFace.conn()) == 8*4)

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
