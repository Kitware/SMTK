#!/usr/bin/python
import sys
import os
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

class TestPolygonImport(smtk.testing.TestCase):

  def setUp(self):
    self.writeJSON = False
    self.mgr = smtk.model.Manager.create()
    sess = self.mgr.createSession('polygon')
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

    #opnames = sess.operatorNames()
    #print opnames

  def color(self, i):
      # Brewer pastel1:
      colors = [0xFBB4AE, 0xB3CDE3, 0xCCEBC5, 0xDECBE4, 0xFED9A6, 0xFFFFCC, 0xE5D8BD, 0xFDDAEC, 0xF2F2F2]
      # Brewer qualitative set1:
      colors = [0xE41A1C, 0x377EB8, 0x4DAF4A, 0x984EA3, 0xFF7F00, 0xFFFF33, 0xA65628, 0xF781BF, 0x999999]
      im = i % len(colors)
      col = colors[im]
      return ((col/256/256)/255.0, (col/256 % 256)/255.0, (col % 256)/255.0, 1.0)

  def testImportShapefile(self):
    iop = GetActiveSession().op('import')
    iop.findAsFile('filename').setValue( \
        os.path.join(smtk.testing.DATA_DIR, 'gis', 'ne_110m_land', 'ne_110m_land.shp'))
    res = iop.operate()
    print '----'
    PrintResultLog(res, always=True)

    cre = res.findModelEntity('created')
    print cre
    print '----'
    self.assertEqual(cre.numberOfValues(), 1, 'Import failed to create a single model')
    mod = smtk.model.Model(cre.value(0))
    if self.writeJSON:
        smtk.io.ExportJSON.fromModelManagerToFile(self.mgr, '/tmp/shapefile.json')
        print 'Wrote /tmp/shapefile.json'

    if self.haveVTK() and self.haveVTKExtension():

      self.startRenderTest()

      mod = smtk.model.Model(mod)
      freeCells = mod.cells()
      # Color faces:
      for i in range(len(freeCells)):
          if freeCells[i].isFace():
              colorIdx = freeCells[i].integerProperty('pedigree')
              freeCells[i].setColor(self.color(colorIdx[0]))
          else:
              freeCells[i].setColor((0,0,0,1))
      ms, vs, mp, ac = self.addModelToScene(mod)
      ac.GetProperty().SetLineWidth(2)
      ac.GetProperty().SetPointSize(6)

      self.renderer.SetBackground(255, 255, 255)
      self.renderWindow.SetSize(800,400)

      cam = self.renderer.GetActiveCamera()
      cam.SetViewUp(0,1,0)
      self.renderer.ResetCamera()
      cam.Zoom(2.19)
      self.renderWindow.Render()
      # Skip the image match if we don't have a baseline.
      # This allows the test to succeed even on systems without the test
      # data but requires a match on systems with the test data.
      self.assertImageMatch(['baseline', 'smtk', 'polygon', 'import-ne_110m_land.png'], 50.0)
      self.interact()

    else:
      self.assertFalse(
        self.haveVTKExtension(),
        'Could not import vtk. Python path is {pp}'.format(pp=sys.path))


if __name__ == '__main__':
  smtk.testing.process_arguments()
  smtk.testing.main()
