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

  def createTestEdges(self, mod):

    #print smtk.io.ExportJSON.fromModelManager(self.mgr, smtk.io.JSON_DEFAULT)

    openEdgeTestVerts = [[4,3.5], [3,3.5]]
    elist = CreateEdge(openEdgeTestVerts, model=mod)
    edges = [smtk.model.Edge(elist)]
    self.assertIsNotNone(edges[0], 'Expected a single edge.')
    self.assertEqual(len(edges[0].vertices()), 2, 'Expected two vertices bounding edge.')
    edges[0].setName('Jinky')

    # Test non-periodic edge with self-intersection.
    # Test periodic edge with self-intersection.
    #edgeTestVerts = [[0,1], [1,2], [0,2], [1,1],   [4,0], [4,3], [5,3], [3,0], [4,0], [11,10]]
    edgeTestVerts = [[0,1], [1,2], [0,2], [0.5,1.5],   [4,0], [4,3], [5,3], [3,0], [4,0], [11,10]]
    edgeTestOffsets = [0, 4, 9, 9, 12]; # Only first 3 edges are valid
    elist = CreateEdge(edgeTestVerts, offsets=edgeTestOffsets, model=mod)
    edges += [smtk.model.Edge(e) for e in elist]
    edges[1].setName('Appendix')
    edges[2].setName('Tango')
    edges[3].setName('BowTieA')
    edges[4].setName('BowTieB')

    # Test creation of periodic edge with no model vertices.
    periodicEdgeVerts = [[0, 4], [1, 4], [1, 5], [0, 5], [0, 4]]
    edge = CreateEdge(periodicEdgeVerts, model=mod)
    edges += [smtk.model.Edge(edge)]
    edges[5].setName('Square')

    print 'Created a total of {:1} edges'.format(len(edges))

    #smtk.io.ExportJSON.fromModelManagerToFile(self.mgr, '/tmp/poly.json')
    return edges

  def testTweakEdge(self):
      mod = CreateModel()
      tinkered = []
      edges = self.createTestEdges(mod)
      flist = CreateFaces(mod)
      print '{:1} faces'.format(len(flist))
      for ff in range(len(flist)):
        print 'Face {:1} edges {:2}'.format(ff, ';'.join([x.name() for x in smtk.model.Face(flist[ff]).edges()]))
      smtk.io.ExportJSON.fromModelManagerToFile(self.mgr, '/tmp/polya.json')
      # Test the easy case: an isolated, non-periodic edge is reshaped:
      print 'Tweaking {:1} {:2}'.format(edges[0].name(), edges[0].entity())
      mods = TweakEdge(edges[0], [[0, 0], [1, 0], [2, 3], [3,3]])
      tinkered += mods
      # Test that when an edge is tweaked whose endpoint is connected to a second edge,
      # the second edge's point-sequence and tessellation are also updated:
      print 'Tweaking {:1} {:2}'.format(edges[1].name(), edges[1].entity())
      mods = TweakEdge(edges[1], [[0,1], [1,1]])
      tinkered += mods

      print 'Tweaking {:1} {:2}'.format(edges[4].name(), edges[4].entity())
      mods = TweakEdge(edges[4], [[4,1.5], [5, 3], [4.5, 3.25], [4,3], [4,1.5]])
      tinkered += mods

      print 'Tweaking {:1} {:2}'.format(edges[3].name(), edges[3].entity())
      mods = TweakEdge(edges[3], [[4,1.5], [3, 0], [3.5, -0.25], [4,0], [4,1.5]])
      tinkered += mods
      smtk.io.ExportJSON.fromModelManagerToFile(self.mgr, '/tmp/polyb.json')
      print 'Tinkered with ', tinkered
      self.imageComparison(mod, tinkered, ['baseline', 'smtk', 'polygon', 'tweakEdge-caseA.png'], False)

  def imageComparison(self, mod, edges, imagePath, doInteract):
    if self.haveVTK() and self.haveVTKExtension():
      from vtk import vtkColorSeries

      self.startRenderTest()

      #mod = smtk.model.Model(mod)
      #[mod.addCell(x) for x in self.mgr.findEntitiesOfType(smtk.model.CELL_ENTITY, False)]

      # Color faces but not edges or vertices
      cs = vtkColorSeries()
      cs.SetColorScheme(vtkColorSeries.BREWER_QUALITATIVE_SET1)
      clist = [cs.GetColor(i) for i in range(cs.GetNumberOfColors())]
      edgeColors = [(c.GetRed()/255., c.GetGreen()/255., c.GetBlue()/255., 1.0) for c in clist]
      ents = self.mgr.findEntitiesOfType(smtk.model.CELL_ENTITY, False)
      for ei in range(len(ents)):
          ents[ei].setFloatProperty('color', edgeColors[ei % len(edgeColors)])
          print ents[ei].name(), ' color ', edgeColors[ei % len(edgeColors)]
      #[v.setFloatProperty('color', [0,0,0,1]) for v in self.mgr.findEntitiesOfType(smtk.model.VERTEX, True)]
      #[e.setFloatProperty('color', [0,0,0,1]) for e in self.mgr.findEntitiesOfType(smtk.model.EDGE, True)]

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
      #smtk.testing.INTERACTIVE = doInteract
      # Skip the image match if we don't have a baseline.
      # This allows the test to succeed even on systems without the test
      # data but requires a match on systems with the test data.
      #self.assertImageMatchIfFileExists(imagePath, 70)
      #self.assertImageMatch(imagePath)
      self.interact()

    else:
      self.assertFalse(
        self.haveVTKExtension(),
        'Could not import vtk. Python path is {pp}'.format(pp=sys.path))


if __name__ == '__main__':
  smtk.testing.process_arguments()
  smtk.testing.main()
