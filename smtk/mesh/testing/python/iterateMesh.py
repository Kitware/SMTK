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


import os
import smtk
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.io
    import smtk.mesh
import smtk.testing
import sys


class MeshVisitor(smtk.mesh.MeshForEach):
    def __init__(self):
        smtk.mesh.MeshForEach.__init__(self)
        self.count = 0

    def forMesh(self, singleMesh):
        self.count += 1


class CellVisitor(smtk.mesh.CellForEach):
    def __init__(self):
        smtk.mesh.CellForEach.__init__(self, True)
        self.count = 0

    def forCell(self, cellId, cellType, numPoints):
        self.count += 1

        # show how to access the point ids of the cell
        pts = []
        for i in xrange(0, numPoints):
            pts.append(self.pointId(i))

        # verify we have the correct number
        # of coordinates
        numCoords = len(self.coordinates())
        if not (numCoords == numPoints * 3):
            # mark the cell invalid if numCoords
            # is wrong
            self.count -= 1


class PointVisitor(smtk.mesh.PointForEach):
    def __init__(self):
        smtk.mesh.PointForEach.__init__(self)
        self.count = 0

    def forPoints(self, pointIds, xyz, doModify):
        self.count += pointIds.size()
        # x,y,z is the physical location of the point
        pass


def test_file_load():
    m = smtk.mesh.Manager.create()

    # Load the mesh file
    print 'data_dir', smtk.testing.DATA_DIR
    mesh_path = os.path.join(smtk.testing.DATA_DIR,
                             'mesh', '3d/sixth_hexflatcore.h5m')
    c = smtk.io.importMesh(mesh_path, m)
    if not c.isValid():
        raise RuntimeError("Failed to read valid mesh")

    # 1. iterate meshes
    meshVisitor = MeshVisitor()
    smtk.mesh.for_each(c.meshes(), meshVisitor)
    if not (meshVisitor.count == c.meshes().size()):
        raise RuntimeError("Python MeshForEach didn't visit each mesh")

    # 2. iterate cells
    cellVisitor = CellVisitor()
    smtk.mesh.for_each(c.cells(), cellVisitor)
    if not (cellVisitor.count == c.cells().size()):
        raise RuntimeError("Python CellVisitor didn't visit each cell")

    # 3. iterate points
    pointVisitor = PointVisitor()
    smtk.mesh.for_each(c.points(), pointVisitor)
    if not (pointVisitor.count == c.points().size()):
        raise RuntimeError("Python PointForEach didn't visit each point")


if __name__ == '__main__':
    smtk.testing.process_arguments()
    test_file_load()
