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
import sys
import smtk
import smtk.session.polygon
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

    def renderTestModel(self, mod, baselinePath):

        if self.haveVTK() and self.haveVTKExtension():

            self.startRenderTest()

            mod = smtk.model.Model(mod)
            # Assign one color (black) to all edges and vertices (since this
            # tests face creation):
            [v.setFloatProperty('color', [0, 0, 0, 1])
             for v in self.resource.findEntitiesOfType(smtk.model.VERTEX, True)]
            [e.setFloatProperty('color', [0, 0, 0, 1])
             for e in self.resource.findEntitiesOfType(smtk.model.EDGE, True)]
            # [mod.addCell(x) for x in self.resource.findEntitiesOfType(smtk.model.CELL_ENTITY, False)]
            ms, vs, mp, ac = self.addModelToScene(mod)
            ac.GetProperty().SetLineWidth(2)
            ac.GetProperty().SetPointSize(6)

            cam = self.renderer.GetActiveCamera()
            cam.SetFocalPoint(5, 5, 0)
            cam.SetPosition(5, 5, 5)
            cam.SetViewUp(0, 1, 0)
            self.renderer.SetBackground(1, 1, 1)
            self.renderer.ResetCamera()
            self.renderWindow.Render()

            # Uncomment for debugging before the image test fails:
            # smtk.testing.INTERACTIVE = True
            # self.interact()

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
        print('testCreationFromPoints')
        # Create a default model (in the x-y plane)
        mod = self.createModel()

        weirdPoints = [
            0.0, 0.0,   5.0, 0.0,   5.0, 5.0,    0.0, 5.0,
            0.0, 0.0,
            \
            1.0, 1.0,   2.0, 1.0,   2.0, 4.0,    1.0, 4.0,
            1.5, 2.5,   1.0, 1.0,
            \
            3.0, 2.0,   4.0, 2.0,   3.7, 2.5,    4.0, 3.0,
            3.0, 3.0,   2.7, 2.5,   3.0, 2.0,
        ]
        weirdCounts = [5, 2, 6, 7]

        fop = smtk.session.polygon.ForceCreateFace.create()
        fop.parameters().associateEntity(mod)
        fop.parameters().find('construction method').setDiscreteIndex(0)
        self.setVectorValue(fop.parameters().find('counts'), weirdCounts)
        self.setVectorValue(fop.parameters().find('points'), weirdPoints)
        fop.parameters().find('coordinates').setValue(0, 2)
        res = fop.operate()
        created = res.find('created')
        faceColors = [[0.9, 1.0, 0.9, 1.0], [0.9, 0.9, 1.0, 1.0]]
        for i in range(created.numberOfValues()):
            print('  face: ', smtk.model.EntityRef(created.value(i)).name())
            smtk.model.EntityRef(created.value(i)).setFloatProperty(
                'color', faceColors[i])

        self.renderTestModel(
            mod, ['baseline', 'smtk', 'polygon', 'forceCreateFaces.png'])

    def createEdges(self, mod, point_arrays):
        (point_seq, point_offsets) = computeOffsets(point_arrays)
        return self.createEdge(point_seq, offsets=point_offsets, model=mod)

    def testCreationFromEdges(self):
        print('testCreationFromEdges')
        # Create a default model (in the x-y plane)
        mod = self.createModel()

        # Create some edges
        elist = self.createEdges(mod, [
            # Face 1 Outer
            [[0.0, 0.0],  [5.0, 0.0]],
            [[5.0, 5.0],  [5.0, 0.0]],
            [[5.0, 5.0],  [0.0, 5.0],  [0.0, 0.0]],
            \
            # Face 1 Inner
            [[1.0, 1.0],  [1.5, 2.5],  [1.0, 4.0],
                [2.0, 4.0],  [2.0, 1.0],  [1.0, 1.0]],
            [[3.0, 2.0],  [2.7, 2.5],  [3.0, 3.0],  [4.0, 3.0],
                [3.7, 2.5],  [4.0, 2.0],  [3.0, 2.0]],
            \
            # Face 2 Outer
            [[5.0, 0.0],  [7.0, 0.0],  [8.0, 6.0],  [5.0, 5.0]],
        ])

        # Create the face from edges we specify
        fop = smtk.session.polygon.ForceCreateFace.create()
        fcount = [3, 2,        1,  1,  2, 0, ]
        # indices into elist of edges that form loops
        floops = [0,  1,  2,  3,  4,  1,  5]
        # orientation of each edge in floops
        forint = [+1, -1, +1, +1, +1, +1, +1]
        aelist = [elist[e] for e in floops]   # de-referenced version of floops
        fop.parameters().find('construction method').setDiscreteIndex(1)
        for i in aelist:
            fop.parameters().associate(i.component())
        self.setVectorValue(fop.parameters().find('counts'), fcount)
        self.setVectorValue(fop.parameters().find('orientations'), forint)
        res = fop.operate()
        print('  outcome: ', res.find('outcome').value(0))
        created = res.find('created')
        faceColors = [[0.9, 1.0, 0.9, 1.0], [0.9, 0.9, 1.0, 1.0]]
        for i in range(created.numberOfValues()):
            print('  face: ', created.value(i).name())
            created.value(i).setFloatProperty('color', faceColors[i])

        self.renderTestModel(
            mod, ['baseline', 'smtk', 'polygon', 'forceCreateFacesFromEdges.png'])


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
