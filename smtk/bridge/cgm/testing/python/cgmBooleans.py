#!/usr/bin/python
import sys
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
import smtk
from smtk.simple import *
import smtk.testing

class TestCGMBooleans(smtk.testing.TestCase):

  def setUp(self):
    self.writeJSON = False
    self.mgr = smtk.model.Manager.create()
    sess = self.mgr.createSession('cgm')
    brg = sess.session()
    print sess
    print brg
    sess.assignDefaultName()
    SetActiveSession(sess)
    print '\n\n%s: type "%s" %s %s' % \
      (sess.name(), brg.name(), sess.flagSummary(0), brg.sessionId())
    print '  Site: %s' % (sess.site() or 'local')

    # We could evaluate the session tag as JSON, but most of
    # the information is available through methods above that
    # we needed to test:
    sessiontag = sess.tag()
    print '\n'

    opnames = sess.operatorNames()

  def testBooleans(self):
    b0 = CreateBrick(center=[0,0,0])
    Translate(b0, [0, 1.5, 0])
    s0 = CreateSphere(center=[0.5,2.0,0.5], radius=0.5)
    bsuni = Union(bodies=[b0, s0])

    b0 = CreateBrick(center=[0,0,0])
    s0 = CreateSphere(center=[0.5,0.5,0.5], radius=0.5)
    bsint = Intersect(bodies=[b0, s0])

    b0 = CreateBrick(center=[0,0,0])
    s0 = CreateSphere(center=[0.5,0.5,0.5], radius=0.5)
    bssub = Subtract(workpiece=b0, tool=s0)

    Translate(bsint, [0.1, 0.1, 0.1])

    if self.haveVTK() and self.haveVTKExtension():

      self.startRenderTest()

      self.addModelToScene(bsuni)
      self.addModelToScene(bsint)
      self.addModelToScene(bssub)

      cam = self.renderer.GetActiveCamera()
      cam.SetFocalPoint(0.125, 0.7, -0.1)
      cam.SetPosition(2,-1,1)
      cam.SetViewUp(-1,1,1)
      self.renderer.ResetCamera()
      self.renderWindow.Render()
      # Skip the image match if we don't have a baseline.
      # This allows the test to succeed even on systems without the test
      # data but requires a match on systems with the test data.
      self.assertImageMatchIfFileExists(['baseline', 'smtk', 'cgm', 'booleans.png'])
      self.interact()

    else:
      self.assertFalse(
        self.haveVTKExtension(),
        'Could not import vtk. Python path is {pp}'.format(pp=sys.path))

    mod = smtk.model.Model(bsuni)
    self.assertEqual(
        mod.geometryStyle(), smtk.model.PARAMETRIC,
        'Expected a parametric solid model, got {gs}'.format(gs=mod.geometryStyle()))
    if self.writeJSON:
      json = smtk.io.ExportJSON.fromModelManager(self.mgr)
      sphFile = open('boolean.json', 'w')
      print >> sphFile, json
      sphFile.close()

if __name__ == '__main__':
  smtk.testing.process_arguments()
  smtk.testing.main()
