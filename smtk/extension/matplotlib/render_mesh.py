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

""" render_mesh.py:

Render a 2-dmensional mesh using matplotlib.

"""
import matplotlib.pyplot as plt
import matplotlib.tri as tri
import smtk
import smtk.mesh
import smtk.model


@smtk.model.operator("render mesh", smtk.model.Session)
class RenderMesh(smtk.model.Operator):

    def __init__(self):
        smtk.model.Operator.__init__(self)

    def name(self):
        return "render mesh"

    def className(self):
        return self.__name__

    def operateInternal(self):
        # Access the mesh and filename from the specification
        mesh = self.specification().findMesh('mesh').value(0)
        filename = self.specification().findFile('filename').value(0)

        # Collect point coordinates
        coords = []

        class GetPoints(smtk.mesh.PointForEach):

            def __init__(self):
                smtk.mesh.PointForEach.__init__(self)

            def forPoints(self, pointIds, xyz, doModify):
                for pId in xrange(pointIds.size()):
                    coords.insert(pId, [xyz[3 * pId + i] for i in xrange(3)])

        getPoints = GetPoints()
        smtk.mesh.for_each(mesh.points(), getPoints)

        # Collect triangles
        tris = []

        class GetTriangles(smtk.mesh.CellForEach):

            def __init__(self):
                smtk.mesh.CellForEach.__init__(self, False)

            def forCell(self, cellId, cellType, numPoints):
                if numPoints == 3:
                    tris.append(
                        [mesh.points().find(self.pointId(i)) for i in xrange(numPoints)])

        getTriangles = GetTriangles()
        smtk.mesh.for_each(mesh.cells(), getTriangles)

        # Construct a pyplot, populate it with the triangles and color it by the
        # z-coordinate of the mesh
        plt.figure()
        plt.gca().set_aspect('equal')
        plt.tricontourf([c[0] for c in coords], [c[1]
                        for c in coords], tris, [c[2] for c in coords])
        plt.colorbar()
        plt.title('Mesh Elevation')
        plt.xlabel('X (units)')
        plt.ylabel('Y (units)')

        # Save the resulting image to the user-defined filename
        plt.savefig(filename, bbox_inches='tight')

        # Return with success
        result = self.createResult(smtk.model.OPERATION_SUCCEEDED)
        return result
