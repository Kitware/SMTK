import sys
# =============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
# =============================================================================
import smtk
import smtk.session.polygon
import smtk.io
import smtk.testing


class TestPolygonCreation(smtk.testing.TestCase):

    def setVectorValue(self, item, vals):
        item.setNumberOfValues(len(vals))
        for i in range(len(vals)):
            item.setValue(i, vals[i])

    def createModel(self, **args):
        """Create an empty geometric model.
        """
        cm = smtk.session.polygon.CreateModel.create()
        if cm is None:
            return
        xAxis = args['x_axis'] if 'x_axis' in args else None
        yAxis = args['y_axis'] if 'y_axis' in args else None
        normal = args['normal'] if 'normal' in args else (
            args['z_axis'] if 'z_axis' in args else None)
        origin = args['origin'] if 'origin' in args else None
        modelScale = args['model_scale'] if 'model_scale' in args else None
        featureSize = args['feature_size'] if 'feature_size' in args else None
        if modelScale is not None and featureSize is not None:
            print('Specify either model_scale or feature_size but not both')
            return
        method = -1
        if modelScale is not None:
            if normal is not None:
                print(
                    'When specifying model_scale, you must specify x and y axes. Normal is ignored.')
            method = 2
        if featureSize is not None:
            if normal is not None:
                method = 1
            else:
                method = 0
        cm.parameters().find('construction method').setDiscreteIndex(method)
        if origin is not None:
            self.setVectorValue(cm.parameters().find('origin'), origin)
        if xAxis is not None:
            self.setVectorValue(cm.parameters().find('x axis'), xAxis)
        if yAxis is not None and cm.parameters().find('y axis') is not None:
            self.setVectorValue(cm.parameters().find('y axis'), yAxis)
        if normal is not None and cm.parameters().find('z axis') is not None:
            self.setVectorValue(cm.parameters().find('z axis'), normal)
        if modelScale is not None:
            cm.parameters().find('model scale').setValue(modelScale)
        if featureSize is not None:
            cm.parameters().find('feature size').setValue(featureSize)
        self.res = cm.operate()
        self.resource = smtk.model.Resource.CastTo(
            self.res.find('resourcesCreated').value(0))
        return self.resource.findEntitiesOfType(int(smtk.model.MODEL_ENTITY))[0]

    def createVertices(self, pt, model, **kwargs):
        """Create one or more vertices given point coordinates.
        Point coordinates should be specified as a list of 3-tuples.
        The vertices are inserted into the given model
        """
        crv = smtk.session.polygon.CreateVertices.create()
        crv.parameters().associate(model.component())
        numPts = len(pt)
        numCoordsPerPoint = max([len(pt[i]) for i in range(numPts)])
        pgi = crv.parameters().find('point dimension')
        pgi.setDiscreteIndex(0 if numCoordsPerPoint == 2 else 1)
        pgr = crv.parameters().find(
            '2d points' if numCoordsPerPoint == 2 else '3d points')
        pgr.setNumberOfGroups(numPts)
        for ix in range(numPts):
            xx = pgr.item(ix, 0)
            self.setVectorValue(xx, pt[ix][0:numCoordsPerPoint] +
                                [0, ] * (numCoordsPerPoint - len(pt[ix])))
        self.res = crv.operate()
        created = self.res.find('created')
        return [smtk.model.EntityRef(created.value(i)) for i in range(created.numberOfValues())]

    class CurveType:
        ARC = 1
        LINE = 6

    def createEdge(self, verts, curve_type=CurveType.LINE, **kwargs):
        """Create an edge from a pair of vertices.
        """
        import itertools
        cre = smtk.session.polygon.CreateEdge.create()
        cre.parameters().find('construction method').setValue(0)
        if hasattr(verts[0], '__iter__'):
            # Verts is actually a list of tuples specifying point coordinates.
            # Look for a model to associate with the operator.
            if 'model' not in kwargs:
                print('Error: No model specified.')
                return None
            cre.parameters().associate(kwargs['model'].component())
            # Pad and flatten point data
            numCoordsPerPoint = max([len(verts[i]) for i in range(len(verts))])
            tmp = min([len(verts[i]) for i in range(len(verts))])
            x = cre.parameters().find('points')
            c = cre.parameters().find('coordinates')
            if c:
                c.setValue(0, numCoordsPerPoint)
            if tmp != numCoordsPerPoint:
                ptflat = []
                for p in verts:
                    ptflat.append(p + [0, ] * (numCoordsPerPoint - len(p)))
                ptflat = list(itertools.chain(*ptflat))
            else:
                ptflat = list(itertools.chain(*verts))
            if x:
                x.setNumberOfValues(len(ptflat))
                for i in range(len(ptflat)):
                    x.setValue(i, ptflat[i])
        else:
            [cre.parameters().associate(x.component()) for x in verts]
        t = cre.parameters().find('curve type')
        if t:
            t.setValue(0, curve_type)
        if 'offsets' in kwargs:
            o = cre.parameters().find('offsets')
            if o:
                o.setNumberOfValues(len(kwargs['offsets']))
                for i in range(len(kwargs['offsets'])):
                    o.setValue(i, kwargs['offsets'][i])
        if 'midpoint' in kwargs:
            x = cre.parameters().find('point')
            if x:
                x.setNumberOfValues(len(kwargs['midpoint']))
                for i in range(len(kwargs['midpoint'])):
                    x.setValue(i, kwargs['midpoint'][i])
        if 'color' in kwargs:
            c = cre.parameters().find('color')
            if c:
                c.setNumberOfValues(len(kwargs['color']))
                for i in range(len(kwargs['color'])):
                    c.setValue(i, kwargs['color'][i])
                c.setValue(0, kwargs['color'])
        self.res = cre.operate()
        entList = self.res.find('created')
        numNewEnts = entList.numberOfValues()

        eList = [smtk.model.EntityRef(entList.value(i))
                 for i in range(entList.numberOfValues())]

        edgeList = []
        for i in range(numNewEnts):
            if eList[i].isEdge():
                edgeList.append(eList[i])
        return edgeList[0] if len(edgeList) == 1 else edgeList

    def tweakEdge(self, edge, newPts, **kwargs):
        """Tweak an edge by providing a new set of points along it.
        """
        import itertools
        twk = smtk.session.polygon.TweakEdge.create()
        twk.parameters().associateEntity(edge)
        numCoordsPerPoint = max([len(newPts[i]) for i in range(len(newPts))])
        tmp = min([len(newPts[i]) for i in range(len(newPts))])
        x = twk.parameters().find('points')
        c = twk.parameters().find('coordinates')
        if c:
            c.setValue(0, numCoordsPerPoint)
        if tmp != numCoordsPerPoint:
            ptflat = []
            for p in newPts:
                ptflat.append(p + [0, ] * (numCoordsPerPoint - len(p)))
            ptflat = list(itertools.chain(*ptflat))
        else:
            ptflat = list(itertools.chain(*newPts))
        if x:
            self.setVectorValue(x, ptflat)
        if 'promote' in kwargs:
            o = twk.parameters().find('promote')
            if o:
                SetVectorValue(o, kwargs['promote'])
        self.res = twk.operate()
        modlist = self.res.find('modified')
        result = [smtk.model.EntityRef(modlist.value(i))
                  for i in range(modlist.numberOfValues())]
        crelist = self.res.find('created')
        result += [smtk.model.EntityRef(crelist.value(i))
                   for i in range(crelist.numberOfValues())]
        return result

    def splitEdge(self, edge, point, **kwargs):
        """Split an edge at a point along the edge.
        """
        import itertools
        spl = smtk.session.polygon.SplitEdge.create()
        spl.parameters().associateEntity(edge)
        x = spl.parameters().find('point')
        x.setNumberOfValues(len(point))
        for i in range(len(point)):
            x.setValue(i, point[i])
        self.res = spl.operate()
        edgeList = self.res.find('created')
        numEdges = edgeList.numberOfValues()
        return smtk.model.EntityRef(edgeList.value(0)) if numEdges == 1 else [smtk.model.EntityRef(edgeList.value(i)) for i in range(numEdges)]

    def createFaces(self, modelOrEdges, **kwargs):
        """Create all possible planar faces from a set of edges.
        """
        crf = smtk.session.polygon.CreateFaces.create()

        # Associate model or edges to operator:
        if hasattr(modelOrEdges, '__iter__'):
            [crf.parameters().associateEntity(ent) for ent in modelOrEdges]
        else:
            crf.parameters().associateEntity(modelOrEdges)

        self.res = crf.operate()

        faceList = self.res.find('created')
        numFaces = faceList.numberOfValues()
        return smtk.model.EntityRef(faceList.value(0)) if numFaces == 1 else [smtk.model.EntityRef(faceList.value(i)) for i in range(numFaces)]

    def setUp(self):
        self.writeJSON = False

    def createTestEdges(self, mod):

        openEdgeTestVerts = [[4, 3.5], [3, 3.5]]
        elist = self.createEdge(openEdgeTestVerts, model=mod)
        edges = [smtk.model.Edge(elist)]
        self.assertIsNotNone(edges[0], 'Expected a single edge.')
        self.assertEqual(len(edges[0].vertices()), 2,
                         'Expected two vertices bounding edge.')
        edges[0].setName('Jinky')

        # Test non-periodic edge with self-intersection.
        # Test periodic edge with self-intersection.
        # edgeTestVerts = [[0,1], [1,2], [0,2], [1,1],   [4,0], [4,3], [5,3],
        # [3,0], [4,0], [11,10]]
        edgeTestVerts = [[0, 1], [1, 2], [0, 2], [0.5, 1.5],
                         [4, 0], [4, 3], [5, 3], [3, 0], [4, 0], [11, 10]]
        edgeTestOffsets = [0, 4, 9, 9, 12]  # Only first 3 edges are valid
        elist = self.createEdge(
            edgeTestVerts, offsets=edgeTestOffsets, model=mod)
        edges += [smtk.model.Edge(e) for e in elist]
        edges[1].setName('Appendix')
        edges[2].setName('Tango')
        edges[3].setName('BowTieA')
        edges[4].setName('BowTieB')

        # Test creation of periodic edge with no model vertices.
        periodicEdgeVerts = [[0, 4], [1, 4], [1, 5], [0, 5], [0, 4]]
        edge = self.createEdge(periodicEdgeVerts, model=mod)
        edges += [smtk.model.Edge(edge)]
        edges[5].setName('Square')

        print('Created a total of {:1} edges'.format(len(edges)))

        return edges

    def testTweakEdge(self):
        mod = self.createModel()
        tinkered = []
        edges = self.createTestEdges(mod)
        flist = self.createFaces(mod)
        print('{:1} faces'.format(len(flist)))
        for ff in range(len(flist)):
            print('Face {:1} edges {:2}'.format(
                ff, ';'.join([x.name() for x in smtk.model.Face(flist[ff]).edges()])))
        # Test the easy case: an isolated, non-periodic edge is reshaped:
        print('Tweaking {:1} {:2}'.format(
            edges[0].name(), str(edges[0].entity())))
        mods = self.tweakEdge(edges[0], [[0, 0], [1, 0], [2, 3], [3, 3]])
        tinkered += mods
        # Test that when an edge is tweaked whose endpoint is connected to a second edge,
        # the second edge's point-sequence and tessellation are also updated:
        print('Tweaking {:1} {:2}'.format(
            edges[1].name(), str(edges[1].entity())))
        mods = self.tweakEdge(edges[1], [[0, 1], [1, 1]])
        tinkered += mods

        print('Tweaking {:1} {:2}'.format(
            edges[4].name(), str(edges[4].entity())))
        mods = self.tweakEdge(edges[4], [[4, 1.5], [5, 3], [
            4.5, 3.25], [4, 3], [4, 1.5]])
        tinkered += mods

        print('Tweaking {:1} {:2}'.format(
            edges[3].name(), str(edges[3].entity())))
        mods = self.tweakEdge(edges[3], [[4, 1.5], [3, 0], [
            3.5, -0.25], [4, 0], [4, 1.5]])
        tinkered += mods
        print('Tinkered with ', tinkered)
        self.imageComparison(
            mod, tinkered, ['baseline', 'smtk', 'polygon', 'tweakEdge-caseA.png'], False)

    def imageComparison(self, mod, edges, imagePath, doInteract):
        if self.haveVTK() and self.haveVTKExtension():
            from vtk import vtkColorSeries

            self.startRenderTest()

            # mod = smtk.model.Model(mod)
            # [mod.addCell(x) for x in self.resource.findEntitiesOfType(smtk.model.CELL_ENTITY, False)]

            # Color faces but not edges or vertices
            cs = vtkColorSeries()
            cs.SetColorScheme(vtkColorSeries.BREWER_QUALITATIVE_SET1)
            clist = [cs.GetColor(i) for i in range(cs.GetNumberOfColors())]
            edgeColors = [(c.GetRed() / 255., c.GetGreen() / 255.,
                           c.GetBlue() / 255., 1.0) for c in clist]
            ents = self.resource.findEntitiesOfType(
                smtk.model.CELL_ENTITY, False)
            for ei in range(len(ents)):
                ents[ei].setFloatProperty(
                    'color', edgeColors[ei % len(edgeColors)])
                print(ents[ei].name(), ' color ',
                      edgeColors[ei % len(edgeColors)])
            # [v.setFloatProperty('color', [0,0,0,1]) for v in self.resource.findEntitiesOfType(smtk.model.VERTEX, True)]
            # [e.setFloatProperty('color', [0,0,0,1]) for e in self.resource.findEntitiesOfType(smtk.model.EDGE, True)]

            ms, vs, mp, ac = self.addModelToScene(mod)
            ac.GetProperty().SetLineWidth(2)
            ac.GetProperty().SetPointSize(6)

            self.renderer.SetBackground(1.0, 1.0, 1.0)
            cam = self.renderer.GetActiveCamera()
            cam.SetFocalPoint(5, 5, 0)
            cam.SetPosition(5, 5, 5)
            cam.SetViewUp(0, 1, 0)
            self.renderer.ResetCamera()
            self.renderWindow.Render()
            # smtk.testing.INTERACTIVE = doInteract
            # Skip the image match if we don't have a baseline.
            # This allows the test to succeed even on systems without the test
            # data but requires a match on systems with the test data.
            # self.assertImageMatchIfFileExists(imagePath, 70)
            # self.assertImageMatch(imagePath)
            self.interact()

        else:
            self.assertFalse(
                self.haveVTKExtension(),
                'Could not import vtk. Python path is {pp}'.format(pp=sys.path))


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
