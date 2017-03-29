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


import math
import os
import smtk
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.io.vtk
    import smtk.mesh
import sys
import smtk.testing
import vtk


def readXMLFile(fileName):
    fn, fileExtension = os.path.splitext(fileName)
    if fileExtension == '.vtu':
        xmlReader = vtk.vtkXMLUnstructuredGridReader()
    elif fileExtension == '.vtp':
        xmlReader = vtk.vtkXMLPolyDataReader()
    xmlReader.SetFileName(fileName)
    xmlReader.Update()
    return xmlReader.GetOutputDataObject(0)


class CoolingPlateFilter(smtk.mesh.CellForEach):
    def __init__(self, yval, rval, o, less):
        smtk.mesh.CellForEach.__init__(self, True)
        self.validPoints = list()
        self.yvalue = yval
        self.rvalue = rval
        self.lessThan = less
        self.origin = o

    def forCell(self, meshHandle, cellType, numPoints):
        for i in xrange(0, numPoints):
            r = math.sqrt((self.coordinates()[i * 3] - self.origin[0]) *
                          (self.coordinates()[i * 3] - self.origin[0]) +
                          (self.coordinates()[i * 3 + 2] - self.origin[2]) *
                          (self.coordinates()[i * 3 + 2] - self.origin[2]))
            currValue = [self.coordinates()[(i * 3) + 1], r]
            if currValue[0] >= (self.yvalue - 0.002) and \
                    currValue[0] <= (self.yvalue + 0.002):
                if (self.lessThan and (currValue[1] < self.rvalue)) or \
                        ((not self.lessThan) and (currValue[1] >= self.rvalue)):
                    self.validPoints.append(self.pointId(i))


class OuterEdgeFilter(smtk.mesh.CellForEach):
    def __init__(self, o, rmin):
        smtk.mesh.CellForEach.__init__(self, True)
        self.validPoints = list()
        self.origin = o
        self.rmin = rmin

    def forCell(self, meshHandle, cellType, numPoints):
        if numPoints < 3:
            return

        v0 = [0., 0., 0.]  # unit vector from origin to first point in cell
        v1 = [0., 0., 0.]  # unit vector from first point to second point in cell
        v2 = [0., 0., 0.]  # unit vector from first point to third point in cell
        normal = [0., 0., 0.]  # unit normal of cell
        length = [0., 0., 0.]

        # compute v0,v1,v2
        for i in range(0, 3):
            v0[i] = self.coordinates()[i] - self.origin[i]
            length[0] += v0[i] * v0[i]
            v1[i] = self.coordinates()[3 + i] - self.coordinates()[i]
            length[1] += v1[i] * v1[i]
            v2[i] = self.coordinates()[6 + i] - self.coordinates()[i]
            length[2] += v2[i] * v2[i]

        length = map(math.sqrt, length)

        v0 = map(lambda x: x / length[0], v0)
        v1 = map(lambda x: x / length[1], v1)
        v2 = map(lambda x: x / length[2], v2)

        # compute normal
        for i in range(0, 3):
            i1 = (i + 1) % 3
            i2 = (i + 2) % 3
            normal[i] = v1[i1] * v2[i2] - v1[i2] * v2[i1]

        mag = math.sqrt(normal[0] * normal[0] +
                        normal[1] * normal[1] +
                        normal[2] * normal[2])

        normal = map(lambda x: x / mag, normal)

        # reject any cells whose normal is facing up or down
        if abs(normal[1]) > .995:
            return

        # reject any cells whose normal is not facing outwards
        if abs(sum(i[0] * i[1] for i in zip(v0, normal))) < .5:
            return

        # reject any cells whose first coordinate is less than a distance <rmin>
        # from the axis of rotation
        for i in xrange(0, numPoints):
            r = math.sqrt(self.coordinates()[3 * i] * self.coordinates()[3 * i] +
                          self.coordinates()[3 * i + 2] * self.coordinates()[3 * i + 2])
            if r > self.rmin:
                self.validPoints.append(self.pointId(i))


def labelIntersection(c, shell, filter_):
    shellCells = shell.cells()

    smtk.mesh.for_each(shellCells, filter_)
    filteredCells = smtk.mesh.CellSet(c, filter_.validPoints)

    domains = c.domains()

    for dom in domains:
        domainMeshes = c.meshes(dom)

        domainCells = domainMeshes.cells()
        contactCells = smtk.mesh.point_intersect(
            domainCells, filteredCells, smtk.mesh.FullyContained)

        if not contactCells.is_empty():
            contactD = c.createMesh(contactCells)
            c.setDirichletOnMeshes(contactD,
                                   smtk.mesh.Dirichlet(
                                       labelIntersection.nextDirId))
            labelIntersection.nextDirId += 1

    return True


labelIntersection.nextDirId = 0


def breakMaterialsByCellType(c):
    domains = c.domains()

    domainsMade = 0
    for dom in domains:
        domainMeshes = c.meshes(dom)

        for ct in xrange(smtk.mesh.Line, smtk.mesh.CellType_MAX):
            cells = domainMeshes.cells(smtk.mesh.CellType(ct))
            if not cells.is_empty():
                ms = c.createMesh(cells)
                v = (dom.value() * 100) + ct
                c.setDomainOnMeshes(ms, smtk.mesh.Domain(v))
                domainsMade += 1

        c.removeMeshes(domainMeshes)


def convert(inputFile, manager, material):
    if smtk.wrappingProtocol() == 'pybind11':
        cnvrt = smtk.io.vtk.ImportVTKData()
    else:
        cnvrt = smtk.extension.vtk.io.ImportVTKData()
    collection = cnvrt(inputFile, manager, material)
    return collection


def extractMaterials(c, radius, origin, outputFile, bounds):
    shell = c.meshes().extractShell()
    if shell.size() != 1:
        raise RuntimeError('Shell # mismatch (%d vs 1)' % shell.size())

    ymin = bounds[2]
    filter_ = CoolingPlateFilter(ymin, radius, origin, True)
    labelIntersection(c, shell, filter_)

    ymax = bounds[3]
    filter_ = CoolingPlateFilter(ymax, radius, origin, True)
    labelIntersection(c, shell, filter_)

    center = [origin[0], origin[1] + (ymax + ymin) * .5, origin[2]]
    filter_ = OuterEdgeFilter(origin, radius * 2)
    labelIntersection(c, shell, filter_)

    breakMaterialsByCellType(c)

    if len(c.domains()) != 5:
        raise RuntimeError('Domain # mismatch (%d vs 5)' % len(c.domains()))
    if len(c.dirichlets()) != 3:
        raise RuntimeError('Dirichlet # mismatch (%d vs 3)' %
                           len(c.dirichlets()))

#    smtk.io.writeEntireCollection(outputFile, c)


def test_multiscale_converter():
    manager = smtk.mesh.Manager.create()

    inputFileName = smtk.testing.DATA_DIR + '/mesh/3d/nickel_superalloy.vtu'
    outputFileName = 'mesh3D.exo'
    materialName = 'ZoneIds'
    radius = 1.2
    origin = [.02, 0., 0.]

    c = convert(inputFileName, manager, materialName)
    data = readXMLFile(inputFileName)
    bounds = data.GetBounds()

    extractMaterials(c, radius, origin, outputFileName, bounds)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    test_multiscale_converter()
