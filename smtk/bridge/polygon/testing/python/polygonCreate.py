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

    #opnames = sess.operatorNames()
    #print opnames

  def checkModel(self, mod, origin, x_axis, y_axis, normal, feature_size, model_scale):

    self.assertEqual(mod.floatProperty('origin'), origin, 'Bad origin')
    [self.assertAlmostEqual(mod.floatProperty('x axis')[i], x_axis[i], 'Bad x axis') for i in range(3)]
    self.assertEqual(mod.floatProperty('y axis'), y_axis, 'Bad y axis')
    self.assertEqual(mod.floatProperty('normal'), normal, 'Bad normal')
    self.assertEqual(mod.floatProperty('feature size'), [feature_size,], 'Bad feature size')
    self.assertEqual(mod.integerProperty('model scale'), [int(model_scale / feature_size),], 'Bad model scale')

    #print smtk.io.ExportJSON.fromModelManager(self.mgr, smtk.io.JSON_DEFAULT)

    # Print a summary of the model:
    print 'Model ', mod.entity()
    print '  x axis  ', ('  {:.3g}'*3).format(*mod.floatProperty('x axis'))
    print '  y axis  ', ('  {:.3g}'*3).format(*mod.floatProperty('y axis'))
    print '  normal  ', ('  {:.3g}'*3).format(*mod.floatProperty('normal'))
    print '  feature size  {:14.3g}'.format(mod.floatProperty('feature size')[0])
    print '  model scale   {:14d}'.format(mod.integerProperty('model scale')[0])

    # Create vertices and test that they are correct
    # NB: 2.000000005 is chosen below since it is within 1e-8/231000 of 2.0
    #     and thus should result in two identical points for all of the models
    #     in testCreation().
    testVerts = [[1,1], [2,1], [2,2,0], [1,2], [2.00000000000001, 2, 0]]
    vlist = CreateVertices(testVerts, mod)
    print '  Created vertices\n   ', '\n    '.join([x.name() for x in vlist])

    self.assertEqual(len(vlist), 5, 'Expected 5 model vertices reported')
    for vi in range(len(testVerts)):
      vert = vlist[vi]
      vx = smtk.model.Vertex(vert).coordinates()
      print '  {name} {x:.5f} {y:.5f} {z:.5f}'.format(
          name=vert.name(), x=vx[0], y=vx[1], z=vx[2])
      [self.assertAlmostEqual(vx[i], testVerts[vi][i],
        msg='Bad vertex {vi} coordinate {i}'.format(vi=vi,i=i))
        for i in range(2)]
    self.assertEqual(vlist[2], vlist[4],
        'Expected vertices with nearly-identical coordinates to be equivalent')

    edgeTestVerts = [[0,0], [1,1], [0,1], [1,0],   [3,0], [3,3], [4,3], [2,0], [3,0]]
    edgeTestOffsets = [0, 4];
    elist = CreateEdge(edgeTestVerts, offsets=edgeTestOffsets, model=mod)
    print elist

  def testCreation(self):
    mod = CreateModel()
    self.checkModel(mod, [0, 0, 0], [1, 0, 0], [0, 1, 0], [0, 0, 1], 1e-8, 231000)

    mod = CreateModel(x_axis=[1,0,0], y_axis=[0,1,0], model_scale=231000)
    self.checkModel(mod, [0, 0, 0], [1, 0, 0], [0, 1, 0], [0, 0, 1], 1, 231000)

    mod = CreateModel(x_axis=[1,0,0], y_axis=[0,1,0], feature_size=1e-8)
    self.checkModel(mod, [0, 0, 0], [1, 0, 0], [0, 1, 0], [0, 0, 1], 1e-8, 231000)

    mod = CreateModel(x_axis=[1,0,0], normal=[0,0,1], feature_size=1e-8)
    self.checkModel(mod, [0, 0, 0], [1, 0, 0], [0, 1, 0], [0, 0, 1], 1e-8, 231000)

    mod = CreateModel(x_axis=[1,0,0], normal=[0,0,1], model_scale=1182720)
    self.checkModel(mod, [0, 0, 0], [1, 0, 0], [0, 1, 0], [0, 0, 1], 1, 1182720)

if __name__ == '__main__':
  smtk.testing.process_arguments()
  smtk.testing.main()
