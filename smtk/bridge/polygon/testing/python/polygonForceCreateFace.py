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
from smtk.simple import *
import smtk.testing

def computeOffsets(point_arrays):
    point_seq = []
    point_offsets = []
    point_offsets.append(len(point_seq))
    for edge in point_arrays:
        [point_seq.append(x) for x in edge]
        point_offsets.append(len(point_seq))
    return (point_seq, point_offsets)

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

  def renderTestModel(self, mod, baselinePath):

    if self.haveVTK() and self.haveVTKExtension():

      self.startRenderTest()

      mod = smtk.model.Model(mod)
      # Assign one color (black) to all edges and vertices (since this tests face creation):
      [v.setFloatProperty('color', [0,0,0,1]) for v in self.mgr.findEntitiesOfType(smtk.model.VERTEX, True)]
      [e.setFloatProperty('color', [0,0,0,1]) for e in self.mgr.findEntitiesOfType(smtk.model.EDGE, True)]
      #[mod.addCell(x) for x in self.mgr.findEntitiesOfType(smtk.model.CELL_ENTITY, False)]
      ms, vs, mp, ac = self.addModelToScene(mod)
      ac.GetProperty().SetLineWidth(2)
      ac.GetProperty().SetPointSize(6)

      cam = self.renderer.GetActiveCamera()
      cam.SetFocalPoint(5,5,0)
      cam.SetPosition(5,5,5)
      cam.SetViewUp(0,1,0)
      self.renderer.SetBackground(1,1,1)
      self.renderer.ResetCamera()
      self.renderWindow.Render()

      # Uncomment for debugging before the image test fails:
      #smtk.testing.INTERACTIVE = True
      #self.interact()

      # Skip the image match if we don't have a baseline.
      # This allows the test to succeed even on systems without the test
      # data but requires a match on systems with the test data.
      self.assertImageMatchIfFileExists(baselinePath)
      self.interact()

    else:
      self.assertFalse(
        self.haveVTKExtension(),
        'Could not import vtk. Python path is {pp}'.format(pp=sys.path))

  def testCreationFromPoints(self):
      print 'testCreationFromPoints'
      # Create a default model (in the x-y plane)
      mod = CreateModel()

      weirdPoints = [ \
          0.0,0.0,   5.0,0.0,   5.0,5.0,    0.0,5.0, \
          0.0,0.0, \
          \
          1.0,1.0,   2.0,1.0,   2.0,4.0,    1.0,4.0, \
          1.5,2.5,   1.0,1.0, \
          \
          3.0,2.0,   4.0,2.0,   3.7,2.5,    4.0,3.0, \
          3.0,3.0,   2.7,2.5,   3.0,2.0, \
          ]
      weirdCounts = [5, 2, 6, 7]

      fop = GetActiveSession().op('force create face')
      fop.associateEntity(mod)
      fop.findAsInt('construction method').setDiscreteIndex(0)
      SetVectorValue(fop.findAsInt('counts'), weirdCounts)
      SetVectorValue(fop.findAsDouble('points'), weirdPoints)
      fop.findAsInt('coordinates').setValue(0, 2)
      res = fop.operate()
      PrintResultLog(res)
      created = res.findModelEntity('created')
      faceColors = [[0.9, 1.0, 0.9, 1.0], [0.9, 0.9, 1.0, 1.0]]
      for i in range(created.numberOfValues()):
          print '  face: ', created.value(i).name()
          created.value(i).setFloatProperty('color', faceColors[i])

      #self.mgr.assignDefaultNames()
      #smtk.io.ExportJSON.fromModelManagerToFile(self.mgr, '/tmp/forcepoly.json')

      self.renderTestModel(mod, ['baseline', 'smtk', 'polygon', 'forceCreateFaces.png'])

  def createEdges(self, mod, point_arrays):
      (point_seq, point_offsets) = computeOffsets(point_arrays)
      return CreateEdge(point_seq, offsets=point_offsets, model=mod)

  def testCreationFromEdges(self):
      print 'testCreationFromEdges'
      # Create a default model (in the x-y plane)
      mod = CreateModel()

      # Create some edges
      elist = self.createEdges(mod, [ \
              # Face 1 Outer
          [[0.0,0.0],  [5.0,0.0]], \
          [[5.0,5.0],  [5.0,0.0]], \
          [[5.0,5.0],  [0.0,5.0],  [0.0,0.0]], \
          \
              # Face 1 Inner
          [[1.0,1.0],  [1.5,2.5],  [1.0,4.0],  [2.0,4.0],  [2.0,1.0],  [1.0,1.0]], \
          [[3.0,2.0],  [2.7,2.5],  [3.0,3.0],  [4.0,3.0],  [3.7,2.5],  [4.0,2.0],  [3.0,2.0]], \
          \
              # Face 2 Outer
          [[5.0,0.0],  [7.0,0.0],  [8.0,6.0],  [5.0,5.0]],
          ])

      # Create the face from edges we specify
      fop = GetActiveSession().op('force create face')
      fcount = [ 3,2,        1,  1,  2,0, ]
      floops = [ 0,  1,  2,  3,  4,  1,  5] # indices into elist of edges that form loops
      forint = [+1, -1, +1, +1, +1, +1, +1] # orientation of each edge in floops
      aelist = [elist[e] for e in floops]   # de-referenced version of floops
      fop.findAsInt('construction method').setDiscreteIndex(1)
      # NB: Cannot use fop.associateEntity(X) here when the same edge is repeated with different orientations
      #     because Attribute::associateEntity(X) checks that X is not already associated before appending X
      #     to the list of entities. Instead, we get the ModelEntityItem used to hold associations and add to
      #     it manually:
      SetVectorValue(fop.specification().associations(), aelist)
      SetVectorValue(fop.findAsInt('counts'), fcount)
      SetVectorValue(fop.findAsInt('orientations'), forint)
      res = fop.operate()
      PrintResultLog(res)
      print '  outcome: ', res.findInt('outcome').value(0)
      created = res.findModelEntity('created')
      faceColors = [[0.9, 1.0, 0.9, 1.0], [0.9, 0.9, 1.0, 1.0]]
      for i in range(created.numberOfValues()):
          print '  face: ', created.value(i).name()
          created.value(i).setFloatProperty('color', faceColors[i])

      #self.mgr.assignDefaultNames()
      #smtk.io.ExportJSON.fromModelManagerToFile(self.mgr, '/tmp/forcepoly2.json')

      self.renderTestModel(mod, ['baseline', 'smtk', 'polygon', 'forceCreateFacesFromEdges.png'])


if __name__ == '__main__':
  smtk.testing.process_arguments()
  smtk.testing.main()
