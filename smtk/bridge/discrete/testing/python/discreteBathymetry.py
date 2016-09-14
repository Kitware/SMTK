#!/usr/bin/python
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
import smtk.testing
from smtk.simple import *

class TestDiscreteBathymetry(smtk.testing.TestCase):

  def resetTestFiles(self):
    self.filesToTest = []

  def addExternalFile(self, pathStr, bathymetryStr, validator = None):
    self.filesToTest += [{'filename':pathStr, 'bathymetryfile':bathymetryStr, 'validator':validator}]

  def addTestFile(self, pathList, bathymetryPathList, validator = None):
    self.addExternalFile(
      os.path.join(*([smtk.testing.DATA_DIR,] + pathList)),
      os.path.join(*([smtk.testing.DATA_DIR,] + bathymetryPathList)), validator)

  def validateBathymetry(self, model):

    if self.haveVTK() and self.haveVTKExtension():

      self.startRenderTest()

      mbs = self.addModelToScene(model)

      self.renderer.SetBackground(0.5,0.5,1)
      ac = self.renderer.GetActors()
      ac.InitTraversal()
      act = ac.GetNextActor()
      act.SetScale(100, 100, 1)
      cam = self.renderer.GetActiveCamera()
      self.renderer.ResetCamera()
      self.renderWindow.Render()
      self.assertImageMatch(['baseline', 'smtk', 'discrete', 'bathymetry-ChesapeakeBay.png'])
      self.interact()

  def setUp(self):
    import os, sys
    self.resetTestFiles()
    self.addTestFile(['model', '2d', 'cmb', 'ChesapeakeBayContour.cmb'], ['point_cloud', 'ChesapeakeBay100x100.vti'], self.validateBathymetry)

    self.mgr = smtk.model.Manager.create()
    sess = self.mgr.createSession('discrete')
    sess.assignDefaultName()
    SetActiveSession(sess)
    self.shouldSave = False

  def verifyEditBathymetry(self, filename, bathymetryfile, validator):
    """Read a single file and validate that the operator worked.
    This is done by checking number of cells greater than zero
    reported by the output model as well as running an optional
    function on the model to do further model-specific testing."""

    print '\n\nFile: {fname}'.format(fname=filename)

    mod = smtk.model.Model(Read(filename)[0])
    self.assertEqual(len(mod.cells()), 1, 'Expected {nc} free cells'.format(nc=1))

    btm = GetActiveSession().op('edit bathymetry')
    self.assertIsNotNone(btm, 'Missing edit bathymetry operator.')
    SetVectorValue(btm.findAsModelEntity('model'), [mod,])

    print '\n\nBathymetry File: {fname}\n\n'.format(fname=bathymetryfile)
    """
    btm.findAsString('operation').setValue('Apply Bathymetry')
    """
    btm.findAsFile('bathymetryfile').setValue(bathymetryfile)
    btm.findAsDouble('averaging elevation radius').setValue(0.05)

    res = btm.operate()
    
    sys.stdout.flush()

    self.assertEqual(res.findInt('outcome').value(0), smtk.model.OPERATION_SUCCEEDED,
        'Edit Bathymetry failed.')

    if validator:
      validator(mod)

    print '  Success'

  def testEditBathymetry(self):
    "Read each file named in setUp and validate the reader worked."
    for test in self.filesToTest:
      self.verifyEditBathymetry(test['filename'], test['bathymetryfile'], test['validator'])

    if self.shouldSave:
      out = file('testbathymetry.json', 'w')
      print >>out, smtk.io.ExportJSON.fromModelManager(self.mgr)
      out.close()

if __name__ == '__main__':
  smtk.testing.process_arguments()
  smtk.testing.main()
