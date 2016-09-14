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

class TestDiscreteCreateEdges(smtk.testing.TestCase):

  def resetTestFiles(self):
    self.filesToTest = []

  def addExternalFile(self, pathStr, validator = None):
    self.filesToTest += [{'filename':pathStr, 'validator':validator}]

  def addTestFile(self, pathList, validator = None):
    self.addExternalFile(
      os.path.join(*([smtk.testing.DATA_DIR,] + pathList)),
      validator)

  def validateCreateEdges(self, model):

    if self.haveVTK() and self.haveVTKExtension():

      self.startRenderTest()

      mbs = self.addModelToScene(model)

      self.renderer.SetBackground(0.5,0.5,1)
      ac = self.renderer.GetActors()
      ac.InitTraversal()
      act = ac.GetNextActor()
      act.GetProperty().SetLineWidth(2)
      act.GetProperty().SetPointSize(8)
      act.GetMapper().SetResolveCoincidentTopologyToPolygonOffset()
      cam = self.renderer.GetActiveCamera()
      cam.SetPosition(10,15,20)
      cam.SetViewUp(0,1,0)
      self.renderer.ResetCamera()
      self.renderWindow.Render()
      self.assertImageMatch(['baseline', 'smtk', 'discrete', 'createedges-SimpleBox.png'])
      self.interact()

  def setUp(self):
    import os, sys
    self.resetTestFiles()
    self.addTestFile(['cmb', 'SimpleBox.cmb'], self.validateCreateEdges)

    self.mgr = smtk.model.Manager.create()
    sess = self.mgr.createSession('discrete')
    sess.assignDefaultName()
    SetActiveSession(sess)
    self.shouldSave = False

  def verifyCreateEdges(self, filename, validator):
    """Read a single file and validate that the operator worked.
    This is done by checking number of cells greater than zero
    reported by the output model as well as running an optional
    function on the model to do further model-specific testing."""

    print '\n\nFile: {fname}'.format(fname=filename)

    mod = smtk.model.Model(Read(filename)[0])
    self.assertEqual(len(mod.cells()), 1, 'Expected {nc} free cells'.format(nc=1))

    btm = GetActiveSession().op('create edges')
    self.assertIsNotNone(btm, 'Missing create edges operator.')
    SetVectorValue(btm.findAsModelEntity('model'), [mod,])

    res = btm.operate()
    
    sys.stdout.flush()

    self.assertEqual(res.findInt('outcome').value(0), smtk.model.OPERATION_SUCCEEDED,
        'create edges failed.')

    if validator:
      validator(mod)

    print '  Success'

  def testCreateEdges(self):
    "Read each file named in setUp and validate the reader worked."
    for test in self.filesToTest:
      self.verifyCreateEdges(test['filename'], test['validator'])

    if self.shouldSave:
      out = file('testcreateedges.json', 'w')
      print >>out, smtk.io.ExportJSON.fromModelManager(self.mgr)
      out.close()

if __name__ == '__main__':
  smtk.testing.process_arguments()
  smtk.testing.main()
