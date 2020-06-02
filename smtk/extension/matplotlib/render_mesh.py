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

""" render_mesh.py:

Render a 2-dmensional mesh using matplotlib.

"""
# use the 'sites' module to set up our path so we can find matplotlib
import site  # nopep8
site.main()  # nopep8

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.tri as tri
import matplotlib

import smtk.operation
import smtk.mesh
import smtk.io
import smtk.attribute
import smtk
from . import render_mesh_xml


class RenderMesh(smtk.operation.Operation):

    def __init__(self):
        smtk.operation.Operation.__init__(self)

    def name(self):
        return "render mesh"

    def operateInternal(self):
        # Access the mesh and filename from the parameters
        mesh = smtk.mesh.Component.CastTo(
            self.parameters().associations().objectValue(0)).mesh()
        filename = self.parameters().find('filename').value(0)

        # Collect point coordinates
        coords = []

        class GetPoints(smtk.mesh.PointForEach):

            def __init__(self):
                smtk.mesh.PointForEach.__init__(self)

            def forPoints(self, pointIds, xyz, doModify):
                for pId in range(pointIds.size()):
                    coords.insert(pId, [xyz[3 * pId + i] for i in range(3)])

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
                        [mesh.points().find(self.pointId(i)) for i in range(numPoints)])

        getTriangles = GetTriangles()
        smtk.mesh.for_each(mesh.cells(), getTriangles)

        # Construct a pyplot, populate it with the triangles and color it by the
        # z-coordinate of the mesh
        fig1, ax1 = plt.subplots()
        ax1.set_aspect('equal')
        tcf = ax1.tricontourf([c[0] for c in coords], [c[1]
                                                       for c in coords], tris, [c[2] for c in coords])
        fig1.colorbar(tcf)
        plt.title('Mesh Elevation')
        plt.xlabel('X (units)')
        plt.ylabel('Y (units)')

        # Save the resulting image to the user-defined filename
        plt.savefig(filename, bbox_inches='tight')

        # Return with success
        result = self.createResult(smtk.operation.Operation.Outcome.SUCCEEDED)
        return result

    def createSpecification(self):
        spec = smtk.attribute.Resource.create()
        reader = smtk.io.AttributeReader()
        reader.readContents(spec, render_mesh_xml.description, self.log())
        return spec
