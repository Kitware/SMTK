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

class TestPolygonCreation(smtk.testing.TestCase):

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

    opnames = sess.operatorNames()
    print opnames

  def checkModel(self, mod, origin, x_axis, y_axis, normal, feature_size, model_scale):

    self.assertEqual(mod.floatProperty('origin'), origin, 'Bad origin')
    self.assertEqual(mod.floatProperty('x axis'), x_axis, 'Bad x axis')
    self.assertEqual(mod.floatProperty('y axis'), y_axis, 'Bad y axis')
    self.assertEqual(mod.floatProperty('normal'), normal, 'Bad normal')
    self.assertEqual(mod.floatProperty('feature size'), [feature_size,], 'Bad feature size')
    self.assertEqual(mod.integerProperty('model scale'), [model_scale,], 'Bad model scale')

  def testCreation(self):
    mod = CreateModel()
    self.checkModel(mod, [0, 0, 0], [1e8, 0, 0], [0, 1e8, 0], [0, 0, 1e8], 1e-8, 231000)

    mod = CreateModel(x_axis=[1,0,0], y_axis=[0,1,0], model_scale=231000)
    self.checkModel(mod, [0, 0, 0], [1, 0, 0], [0, 1, 0], [0, 0, 1], 1, 231000)

    mod = CreateModel(x_axis=[1,0,0], y_axis=[0,1,0], feature_size=1e-8)
    self.checkModel(mod, [0, 0, 0], [1e8, 0, 0], [0, 1e8, 0], [0, 0, 1e8], 1e-8, 231000)

    mod = CreateModel(x_axis=[1,0,0], normal=[0,0,1], feature_size=1e-8)
    self.checkModel(mod, [0, 0, 0], [1e8, 0, 0], [0, 1e8, 0], [0, 0, 1e8], 1e-8, 231000)

    mod = CreateModel(x_axis=[1,0,0], normal=[0,0,1], model_scale=1182720)
    self.checkModel(mod, [0, 0, 0], [1, 0, 0], [0, 1, 0], [0, 0, 1], 1, 1182720)

    # NB: 2.0000005 is chosen below since it is within 1/1182720 of 2.0 and thus
    #     should result in two identical points.
    vlist = CreateVertices([[1,1], [2,1], [2,2,0], [1,2], [2.0000005, 2, 0]], mod)
    #print smtk.io.ExportJSON.fromModelManager(self.mgr, smtk.io.JSON_DEFAULT)
    print [x.name() for x in vlist]
    self.assertEqual(len(vlist), 5, 'Expected 5 model vertices reported')
    self.assertEqual(vlist[2], vlist[4], 'Expected vertices with nearly-identical coordinates to be equivalent')

if __name__ == '__main__':
  smtk.testing.process_arguments()
  smtk.testing.main()
