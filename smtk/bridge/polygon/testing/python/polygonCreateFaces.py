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
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.bridge.polygon
    import smtk.io
from smtk.simple import *
import smtk.testing

modelParams = [ \
   [ [0, 0, 0], [1, 0, 0], [0, 1, 0], [0, 0, 1], 1e-8, 231000,  True, ], \
   [ [0, 0, 0], [1, 0, 0], [0, 1, 0], [0, 0, 1], 1,    231000, False, ], \
   [ [0, 0, 0], [1, 0, 0], [0, 1, 0], [0, 0, 1], 1e-8, 231000, False, ], \
   [ [0, 0, 0], [1, 0, 0], [0, 1, 0], [0, 0, 1], 1e-8, 231000, False, ], \
   [ [0, 0, 0], [1, 0, 0], [0, 1, 0], [0, 0, 1], 1,   1182720, False, ], \
]

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

  def createModelA(self, mod, origin, x_axis, y_axis, normal, feature_size, model_scale):

    self.assertEqual(mod.floatProperty('origin'), origin, 'Bad origin')
    [self.assertAlmostEqual(mod.floatProperty('x axis')[i], x_axis[i], 'Bad x axis') for i in range(3)]
    self.assertEqual(mod.floatProperty('y axis'), y_axis, 'Bad y axis')
    self.assertEqual(mod.floatProperty('normal'), normal, 'Bad normal')
    self.assertEqual(mod.floatProperty('feature size'), [feature_size,], 'Bad feature size {:1} vs {:2}'.format(mod.floatProperty('feature size')[0], feature_size))

    #print smtk.io.ExportJSON.fromModelManager(self.mgr, smtk.io.JSON_DEFAULT)

    # Print a summary of the model:
    print 'Model ', mod.entity()
    print '  x axis  ', ('  {:.3g}'*3).format(*mod.floatProperty('x axis'))
    print '  y axis  ', ('  {:.3g}'*3).format(*mod.floatProperty('y axis'))
    print '  normal  ', ('  {:.3g}'*3).format(*mod.floatProperty('normal'))
    print '  feature size  {:14.3g}'.format(mod.floatProperty('feature size')[0])
    print '  model scale   {:14.3g}'.format(mod.floatProperty('model scale')[0])

    # Create vertices and test that they are correct
    # NB: 2.000000005 is chosen below since it is within 1e-8/231000 of 2.0
    #     and thus should result in two identical points for all of the models
    #     in testCreation().
    testVerts = [[1,1], [2,1], [2,2,0], [1,2], [2.00000000000001, 2, 0]]
    vlist = CreateVertices(testVerts, mod)
    print '  Created vertices\n   ', '\n    '.join([x.name() for x in vlist])

    self.assertEqual(len(vlist), 5, 'Expected 5 model vertices reported.')
    for vi in range(len(testVerts)):
      vert = vlist[vi]
      vx = smtk.model.Vertex(vert).coordinates()
      print '  {name} {x:.5f} {y:.5f} {z:.5f}'.format(
          name=vert.name(), x=vx[0], y=vx[1], z=vx[2])
      [self.assertAlmostEqual(vx[i], testVerts[vi][i],
        msg='Bad vertex {vi} coordinate {i}'.format(vi=vi,i=i))
        for i in range(2)]
    self.assertEqual(vlist[2], vlist[4],
        'Expected vertices with nearly-identical coordinates to be equivalent.')

    # Test a simple case: a non-periodic edge of one segment
    # whose ends must be promoted to model vertices. Note that
    # the edge goes from right to left, so we check that the
    # endpoints are ordered properly.
    openEdgeTestVerts = [[4,3.5], [3,3.5]]
    elist = CreateEdge(openEdgeTestVerts, model=mod)
    edge = smtk.model.Edge(elist)
    self.assertIsNotNone(edge, 'Expected a single edge.')
    self.assertEqual(len(edge.vertices()), 2, 'Expected two vertices bounding edge.')
    # NB. We cannot test direction of edge using order of edge.vertices() because
    #     they will be ordered by UUID, not parameter value.
    #     Should we start creating edge and vertex uses, plus "vertex chains",
    #     we can then use the SMTK API to properly fetch edge direction. But
    #     arguably, these records should not exist until edges are actually used
    #     by a higher-dimensional entity.

    # Test non-periodic edge of multiple segments whose
    # ends must be promoted to model vertices. Note that
    # the final segment has reversed order relative to
    # boost.polygon's left-right, bottom-top order, so we
    # are verifying that endpoints are computed correctly.
    # This prevents an observed regression.
    openEdgeTestVerts = [[3,4], [3,5], [4,5], [4,4],  [0, 1.5], [1, 2.5]]
    openEdgeTestOffsets = [0, 4]
    elist = CreateEdge(openEdgeTestVerts, offsets=openEdgeTestOffsets, model=mod)

    # Test multiple edge insertion.
    # Test invalid edge connectivity.
    # Test self-intersecting edges.
    # Test periodic edges with non-model-vertex at first point.
    edgeTestVerts = [[0,0], [1,1], [0,1], [1,0],   [3,0], [3,3], [4,3], [2,0], [3,0], [10,10]]
    edgeTestOffsets = [0, 4, 9, 9, 12]; # Only first 3 edges are valid
    elist = CreateEdge(edgeTestVerts, offsets=edgeTestOffsets, model=mod)
    # Make sure that warnings are generated for invalid edge offsets.
    res = GetLastResult()
    logStr = res.findString('log').value(0)
    log = smtk.io.Logger()
    smtk.io.ImportJSON.ofLog(logStr, log)
    print log.convertToString()
    self.assertEqual(
        log.numberOfRecords(), 4,
        'Expected 4 messages due to 3 invalid offsets, got\n' + log.convertToString())
    #print elist

    # Test creation of periodic edge with no model vertices.
    # Verify that no model vertices are created.
    periodicEdgeVerts = [[0, 4], [1, 4], [1, 5], [0, 5], [0, 4]]
    elist = CreateEdge(periodicEdgeVerts, model=mod)

    # Test creation of a second periodic edge with no model vertices
    # but which shares a point with the previous edge.
    # Verify that no model vertices are created.
    # However, if the two edges are used as holes for a containing face
    # or unioned, then the shared point should become a model vertex.
    periodicEdgeVerts = [[1, 3], [2, 3], [2, 4], [1, 4], [1, 3]]
    elist = CreateEdge(periodicEdgeVerts, model=mod)
    edge = smtk.model.Edge(elist)
    self.assertIsNotNone(edge, 'Expected a single edge.')
    self.assertEqual(edge.vertices(), [], 'Expected no model vertices bounding edge.')

    arf = SplitEdge(elist, [2, 4])

    innerLoopVerts = [ \
        [1.25,3.35], [1.65,3.75], [1.25,3.75], [1.25,3.35], \
        [1.35,3.25], [1.75,3.25], [1.75,3.65], [1.35,3.25], \
        [0.25,4.25], [0.75,4.75], \
        [0.25,4.25], [0.75,4.25], [0.75,4.75], \
        [0.25,4.25], [0.25,4.75], [0.75,4.75], \
        ]
    #innerLoopOffsets = [0, 4]
    innerLoopOffsets = [0, 4, 8, 10, 13];
    elist = CreateEdge(innerLoopVerts, offsets=innerLoopOffsets, model=mod)

    # Square pierced by edges
    weirdVerts = [ \
        [4.0,0.5], [4.0,0.0], [5.0,0.0], \
        [4.0,0.5], [4.0,1.0], [5.0,1.0], \
        [5.0,0.0], [5.0,1.0], \
        [3.5,0.5], [4.0,0.5], \
        [4.0,0.5], [4.5,0.5], \
        \
        [4.0,1.5], [4.0,2.5], [4.5,2.5], \
        [5.0,1.5], [5.0,2.5], [4.5,2.5], \
        [4.0,1.5], [5.0,1.5], \
        [4.5,2.0], [4.5,2.5], \
        [4.5,2.5], [4.5,3.0], \
        ]
    weirdOffsets = [0, 3, 6, 8, 10, 12, 15, 18, 20, 22, 24]
    elist = CreateEdge(weirdVerts, offsets=weirdOffsets, model=mod)

    # Some edges completely internal to faces
    weirdVerts = [ \
        [2.56250, 0.1875], [2.9375, 1.3125], \
        [3.06250, 1.6875], [3.4375, 2.8125], \
        ]
    weirdOffsets = [0, 2, 4]
    elist = CreateEdge(weirdVerts, offsets=weirdOffsets, model=mod)

    smtk.io.ExportJSON.fromModelManagerToFile(self.mgr, '/tmp/poly.json')

    flist = CreateFaces(mod)

  def createModelB(self, mod, origin, x_axis, y_axis, normal, feature_size, model_scale):

    # Test geometry
    weirdVerts = [ \
        [  0,    0], [160,    0], [160,    0], [240,    0], \
        [  0,   30], [ 60,   90], [ 60,   90], [ 90,  120], \
        [  0,  120], [ 70,  120], [ 70,  120], [ 90,  120], \
        [ 50,  100], [ 70,  120], [ 70,  120], [120,  170], \
        [ 50,  100], [ 60,   90], [ 60,   90], [120,   30], \
        [190,  100], [175,  115], [175,  115], [120,  170], \

        [110,  110], [110,   90], [130,   90], [110,   90], \
        [130,  110], [110,  110], [130,   90], [130,  110], \

        [160,    0], [160,   70], [160,   70], [160,  100], \
        [160,  100], [175,  115], [175,  115], [220,  160], \
        [120,   30], [160,   70], [160,   70], [190,  100], \
        ]
    weirdOffsets = [0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44]
    elist = CreateEdge(weirdVerts, offsets=weirdOffsets, model=mod)

    # smtk.io.ExportJSON.fromModelManagerToFile(self.mgr, '/tmp/poly.json')

    flist = CreateFaces(mod)

  def testModelACreation(self):
    for p in modelParams:
      if p[4] == 1:
        mod = CreateModel(origin=p[0], x_axis=p[1], y_axis=p[2], z_axis=p[3], model_scale=p[5])
      else:
        mod = CreateModel(origin=p[0], x_axis=p[1], y_axis=p[2], z_axis=p[3], feature_size=p[4])
      self.createModelA(mod, p[0], p[1], p[2], p[3], p[4], p[5])
      res = GetLastResult()
      created = res.findModelEntity('created')
      print 'created ', created.numberOfValues(), ' faces'
      self.imageComparison(mod, created, ['baseline', 'smtk', 'polygon', 'createFacesA.png'], False)

  def testModelBCreation(self):
    t = 0
    for p in modelParams:
      t += 1
      if p[4] == 1:
        mod = CreateModel(origin=p[0], x_axis=p[1], y_axis=p[2], z_axis=p[3], model_scale=p[5])
      else:
        mod = CreateModel(origin=p[0], x_axis=p[1], y_axis=p[2], z_axis=p[3], feature_size=p[4])
      self.createModelB(mod, p[0], p[1], p[2], p[3], p[4], p[5])
      res = GetLastResult()
      created = res.findModelEntity('created')
      print 'created ', created.numberOfValues(), ' faces'
      self.imageComparison(mod, created, ['baseline', 'smtk', 'polygon', 'createFacesB{:1}.png'.format(t)], False) # or p[6])

  def imageComparison(self, mod, created, imagePath, doInteract):
    if self.haveVTK() and self.haveVTKExtension():
      from vtk import vtkColorSeries

      self.startRenderTest()

      #mod = smtk.model.Model(mod)
      #[mod.addCell(x) for x in self.mgr.findEntitiesOfType(smtk.model.CELL_ENTITY, False)]

      # Color faces but not edges or vertices
      flist = [created.value(i) for i in range(created.numberOfValues())]
      cs = vtkColorSeries()
      cs.SetColorScheme(vtkColorSeries.BREWER_QUALITATIVE_SET1)
      clist = [cs.GetColor(i) for i in range(cs.GetNumberOfColors())]
      faceColors = [(c.GetRed()/255., c.GetGreen()/255., c.GetBlue()/255., 1.0) for c in clist]
      for fi in range(len(flist)):
          flist[fi].setFloatProperty('color', faceColors[fi % len(faceColors)])
      [v.setFloatProperty('color', [0,0,0,1]) for v in self.mgr.findEntitiesOfType(smtk.model.VERTEX, True)]
      [e.setFloatProperty('color', [0,0,0,1]) for e in self.mgr.findEntitiesOfType(smtk.model.EDGE, True)]

      ms, vs, mp, ac = self.addModelToScene(mod)
      ac.GetProperty().SetLineWidth(2)
      ac.GetProperty().SetPointSize(6)

      self.renderer.SetBackground(1.0,1.0,1.0)
      cam = self.renderer.GetActiveCamera()
      cam.SetFocalPoint(5,5,0)
      cam.SetPosition(5,5,5)
      cam.SetViewUp(0,1,0)
      self.renderer.ResetCamera()
      self.renderWindow.Render()
      smtk.testing.INTERACTIVE = doInteract
      # Skip the image match if we don't have a baseline.
      # This allows the test to succeed even on systems without the test
      # data but requires a match on systems with the test data.
      self.assertImageMatchIfFileExists(imagePath, 70)
      #self.assertImageMatch(imagePath)
      self.interact()

    else:
      self.assertFalse(
        self.haveVTKExtension(),
        'Could not import vtk. Python path is {pp}'.format(pp=sys.path))


if __name__ == '__main__':
  smtk.testing.process_arguments()
  smtk.testing.main()
