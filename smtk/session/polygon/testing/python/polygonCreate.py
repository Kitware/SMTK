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
import smtk.model
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
        if yAxis is not None:
            self.setVectorValue(cm.parameters().find('y axis'), yAxis)
        if normal is not None and cm.parameters().find('z axis') is not None:
            self.setVectorValue(cm.parameters().find('z axis'), normal)
        if modelScale is not None:
            cm.parameters().find('model scale').setValue(modelScale)
        if featureSize is not None:
            cm.parameters().find('feature size').setValue(featureSize)
        res = cm.operate()
        self.resource = smtk.model.Resource.CastTo(
            res.find('resourcesCreated').value(0))
        return self.resource.findEntitiesOfType(int(smtk.model.MODEL_ENTITY))[0]

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
        res = spl.operate()
        edgeList = res.find('created')
        numEdges = edgeList.numberOfValues()
        return edgeList.value(0) if numEdges == 1 else [edgeList.value(i) for i in range(numEdges)]

    def setUp(self):
        self.writeJSON = False

        # opnames = sess.operatorNames()
        # print(opnames)

    def checkModel(self, mod, origin, x_axis, y_axis, normal, feature_size, model_scale):

        self.assertEqual(mod.floatProperty('origin'), origin, 'Bad origin')
        [self.assertAlmostEqual(mod.floatProperty(
            'x axis')[i], x_axis[i], 'Bad x axis') for i in range(3)]
        self.assertEqual(mod.floatProperty('y axis'), y_axis, 'Bad y axis')
        self.assertEqual(mod.floatProperty('normal'), normal, 'Bad normal')
        self.assertEqual(mod.floatProperty('feature size'), [feature_size, ],
                         'Bad feature size {:.3g}'.format(*mod.floatProperty('feature size')))
        print('Mod scale ', mod.floatProperty('model scale')[0], int(
            model_scale / feature_size), mod.floatProperty('model scale')[0] / int(model_scale / feature_size))
        self.assertEqual(
            int(mod.floatProperty('model scale')[0]), int(
                model_scale / feature_size),
            'Bad model scale {:1}'.format(*mod.floatProperty('model scale')))

        # print(smtk.io.SaveJSON.fromModelResource(self.resource,
        # smtk.io.JSON_DEFAULT))

        # Print a summary of the model:
        print('Model ', mod.entity())
        print('  x axis  ', ('  {:.3g}' * 3).format(
            *mod.floatProperty('x axis')))
        print('  y axis  ', ('  {:.3g}' * 3).format(
            *mod.floatProperty('y axis')))
        print('  normal  ', ('  {:.3g}' * 3).format(
            *mod.floatProperty('normal')))
        print('  feature size  {:14.3g}'.format(
            mod.floatProperty('feature size')[0]))
        print('  model scale   {:14f}'.format(
            mod.floatProperty('model scale')[0]))

        # Create vertices and test that they are correct
        # NB: 2.000000005 is chosen below since it is within 1e-6/231000 of 2.0
        #     and thus should result in two identical points for all of the models
        #     in testCreation().
#        testVerts = [[1, 1], [2, 1], [2, 2, 0],
#                     [1, 2], [2.00000000000001, 2, 0]]

        # The above values result in 4 unique vertices
        testVerts = [[1, 1], [2, 1], [2, 2, 0],
                     [1, 2], [3, 2, 0]]
        crv = smtk.session.polygon.CreateVertices.create()
        crv.parameters().associate(mod.component())
        pgi = crv.parameters().find('point dimension')
        numPts = len(testVerts)
        numCoordsPerPoint = max([len(testVerts[i]) for i in range(numPts)])
        pgi.setDiscreteIndex(0 if numCoordsPerPoint == 2 else 1)
        pgr = crv.parameters().find(
            '2d points' if numCoordsPerPoint == 2 else '3d points')
        pgr.setNumberOfGroups(numPts)
        for ix in range(numPts):
            #            xx = smtk.attribute.to_concrete(pgr.item(ix, 0))
            xx = pgr.item(ix, 0)
            v = testVerts[ix][0:numCoordsPerPoint] + [0, ] * \
                (numCoordsPerPoint - len(testVerts[ix]))
            xx.setNumberOfValues(len(v))
            for i in range(len(v)):
                xx.setValue(i, v[i])
        res = crv.operate()
        clist = res.find('created')
        vlist = [smtk.model.EntityRef(clist.value(i))
                 for i in range(clist.numberOfValues())]

        print('  Created vertices\n   ',
              '\n    '.join([x.name() for x in vlist]))

        self.assertEqual(len(vlist), 5, 'Expected 5 model vertices reported.')
        for vi in range(len(testVerts)):
            vert = vlist[vi]
            vx = smtk.model.Vertex(vert).coordinates()
            print('  {name} {x:.5f} {y:.5f} {z:.5f}'.format(
                name=vert.name(), x=vx[0], y=vx[1], z=vx[2]))
            [self.assertAlmostEqual(
                vx[i], testVerts[vi][i],
                msg='Bad vertex {vi} coordinate {i}'.format(vi=vi, i=i)) for i in range(2)]
#        self.assertEqual(vlist[2], vlist[4],
#                         'Expected vertices with nearly-identical coordinates to be equivalent.')

        # Test a simple case: a non-periodic edge of one segment
        # whose ends must be promoted to model vertices. Note that
        # the edge goes from right to left, so we check that the
        # endpoints are ordered properly.
        openEdgeTestVerts = [[4, 3.5], [3, 3.5]]
        elist = self.createEdge(openEdgeTestVerts, model=mod)
        edge = smtk.model.Edge(elist)
        self.assertIsNotNone(edge, 'Expected a single edge.')
        self.assertEqual(len(edge.vertices()), 2,
                         'Expected two vertices bounding edge.')
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
        openEdgeTestVerts = [[3, 4], [3, 5], [
            4, 5], [4, 4],  [0, 1.5], [1, 2.5]]
        openEdgeTestOffsets = [0, 4]
        elist = self.createEdge(openEdgeTestVerts,
                                offsets=openEdgeTestOffsets, model=mod)

        # Test multiple edge insertion.
        # Test invalid edge connectivity.
        # Test self-intersecting edges.
        # Test periodic edges with non-model-vertex at first point.
        edgeTestVerts = [[0, 0], [1, 1], [0, 1], [1, 0],   [
            3, 0], [3, 3], [4, 3], [2, 0], [3, 0], [10, 10]]
        edgeTestOffsets = [0, 4, 9, 9, 12]  # Only first 2 edges are valid
        elist = self.createEdge(
            edgeTestVerts, offsets=edgeTestOffsets, model=mod)
        # Make sure that warnings are generated for invalid edge offsets.
        logStr = self.res.findString('log').value(0)
        import ast
        log = ast.literal_eval(logStr)
        self.assertEqual(
            len(log), 4,
            'Expected 4 messages due to 3 invalid offsets, got\n' + logStr)

        # Test creation of periodic edge with no model vertices.
        # Verify that no model vertices are created.
        periodicEdgeVerts = [[0, 4], [1, 4], [1, 5], [0, 5], [0, 4]]
        elist = self.createEdge(periodicEdgeVerts, model=mod)

        # Test creation of a second periodic edge with no model vertices
        # but which shares a point with the previous edge.
        # Verify that no model vertices are created.
        # However, if the two edges are used as holes for a containing face
        # or unioned, then the shared point should become a model vertex.
        periodicEdgeVerts = [[1, 3], [2, 3], [2, 4], [1, 4], [1, 3]]
        elist = self.createEdge(periodicEdgeVerts, model=mod)
        edge = smtk.model.Edge(elist)
        self.assertIsNotNone(edge, 'Expected a single edge.')
        self.assertEqual(edge.vertices(), [],
                         'Expected no model vertices bounding edge.')

        arf = self.splitEdge(elist, [2, 4])

    def testCreation(self):
        cm = smtk.session.polygon.CreateModel.create()
        res = cm.operate()
        self.assertEqual(res.find('outcome').value(0),
                         int(smtk.operation.Operation.SUCCEEDED),
                         'create model failed')

        # store the resource so it doesn't go out of scope
        self.resource = smtk.model.Resource.CastTo(
            res.find('resourcesCreated').value(0))
        mod = self.resource.findEntitiesOfType(int(smtk.model.MODEL_ENTITY))[0]

        self.checkModel(mod, [0, 0, 0], [1, 0, 0], [
                        0, 1, 0], [0, 0, 1], 1e-6, 231)

        mod = self.createModel(x_axis=[1, 0, 0], y_axis=[
            0, 1, 0], model_scale=231000)
        self.checkModel(mod, [0, 0, 0], [1, 0, 0], [
                        0, 1, 0], [0, 0, 1], 1, 231000)

        mod = self.createModel(x_axis=[1, 0, 0], y_axis=[
            0, 1, 0], feature_size=1e-8)
        self.checkModel(mod, [0, 0, 0], [1, 0, 0], [
                        0, 1, 0], [0, 0, 1], 1e-8, 231)

        mod = self.createModel(x_axis=[1, 0, 0], normal=[
            0, 0, 1], feature_size=1e-6)
        self.checkModel(mod, [0, 0, 0], [1, 0, 0], [
                        0, 1, 0], [0, 0, 1], 1e-6, 231)

        mod = self.createModel(x_axis=[1, 0, 0], normal=[
            0, 0, 1], model_scale=1182720)
        self.checkModel(mod, [0, 0, 0], [1, 0, 0], [
                        0, 1, 0], [0, 0, 1], 1, 1182720)

        if self.haveVTK() and self.haveVTKExtension():

            self.startRenderTest()

            mod = smtk.model.Model(mod)
            [mod.addCell(x) for x in self.resource.findEntitiesOfType(
                int(smtk.model.CELL_ENTITY), False)]
            ms, vs, mp, ac = self.addModelToScene(mod)
            ac.GetProperty().SetLineWidth(2)
            ac.GetProperty().SetPointSize(6)

            cam = self.renderer.GetActiveCamera()
            cam.SetFocalPoint(5, 5, 0)
            cam.SetPosition(5, 5, 5)
            cam.SetViewUp(0, 1, 0)
            self.renderer.ResetCamera()
            self.renderWindow.Render()
            # Skip the image match if we don't have a baseline.
            # This allows the test to succeed even on systems without the test
            # data but requires a match on systems with the test data.
            self.assertImageMatchIfFileExists(
                ['baseline', 'smtk', 'polygon', 'creation.png'])
            self.interact()

        else:
            self.assertFalse(
                self.haveVTKExtension(),
                'Could not import vtk. Python path is {pp}'.format(pp=sys.path))


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
