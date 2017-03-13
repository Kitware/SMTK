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
import os, sys
import unittest
import smtk
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.mesh
    import smtk.model
    import smtk.model.delaunay
    import smtk.bridge.polygon
import smtk.testing
from smtk.simple import *

class UnitTriangulateFace(unittest.TestCase):

  def setUp(self):
    self.mgr = smtk.model.Manager.create()
    self.sess = self.mgr.createSession('polygon')
    SetActiveSession(self.sess)
    self.modelFile = os.path.join(smtk.testing.DATA_DIR, 'mesh', '2d', 'boxWithHole.smtk')
    self.model = ImportSMTKModel(self.modelFile)[0]

  def testMeshing2D(self):
    if smtk.wrappingProtocol() == 'pybind11':
      face = self.mgr.findEntitiesOfType(int(smtk.model.FACE))[0]
    else:
      face = self.mgr.findEntitiesOfType(smtk.model.FACE, True)[0]
    triangulateFace = self.sess.op('triangulate face')
    triangulateFace.specification().associateEntity(self.model)
    triangulateFace.specification().findModelEntity("face").setValue(face)
    result = triangulateFace.operate()
    triangulatedFace = self.mgr.meshes().associatedCollections(face)[0]
    assert(triangulatedFace.points().size() == 8)
    assert(triangulatedFace.cells().size() == 8)

if __name__ == '__main__':
  smtk.testing.process_arguments()
  unittest.main()
