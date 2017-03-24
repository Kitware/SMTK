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
    import smtk.model
    import smtk.model.remus
    import smtk.bridge.discrete
import smtk.testing
from smtk.simple import *

class UnitMeshOperator(unittest.TestCase):

  def setUp(self):
    self.mgr = smtk.model.Manager.create()
    self.sess = self.mgr.createSession('discrete')
    SetActiveSession(self.sess)
    self.modelFile = os.path.join(smtk.testing.DATA_DIR, 'model', '2d', 'cmb', 'test2D.cmb')
    self.models = Read(self.modelFile)

  def testMeshing2D(self):
    mesher = self.sess.op('mesh')
    # Set mesher attributes here...
    print 'About to operate...'
    result = mesher.operate()
    #self.assertEqual(
    #    result.findInt('outcome').value(0),
    #    smtk.model.OPERATION_SUCCEEDED)

if __name__ == '__main__':
  smtk.testing.process_arguments()
  unittest.main()
