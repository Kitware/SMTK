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
import smtk.io.vtk
import smtk.mesh
import sys
import smtk.testing
import vtk

import inspect


def make_EmptyPolyData():
    return vtk.vtkPolyData()


def make_TrianglePolyData():
    PB = vtk.vtkParametricBoy()
    PFS = vtk.vtkParametricFunctionSource()
    PFS.SetParametricFunction(PB)

    PFS.SetUResolution(400)
    PFS.SetVResolution(500)
    PFS.Update()

    result = vtk.vtkPolyData()

    result.ShallowCopy(PFS.GetOutput())
    return result


def make_TriangleUGrid():
    PB = vtk.vtkParametricBoy()
    PFS = vtk.vtkParametricFunctionSource()
    PFS.SetParametricFunction(PB)

    PFS.SetUResolution(400)
    PFS.SetVResolution(500)
    PFS.Update()

    appendFilter = vtk.vtkAppendFilter()
    appendFilter.AddInputData(PFS.GetOutput())
    appendFilter.Update()

    result = vtk.vtkUnstructuredGrid()

    result.ShallowCopy(appendFilter.GetOutput())
    return result


def make_MixedVolUGrid():
    points = vtk.vtkPoints()
    points.SetNumberOfPoints(7)
    points.InsertPoint(0, 0, 1, 0)
    points.InsertPoint(1, 0, 0, 0)
    points.InsertPoint(2, 0, .5, .5)
    points.InsertPoint(3, 1, 1, 0)
    points.InsertPoint(4, 1, 0, 0)
    points.InsertPoint(5, 1, .5, .5)
    points.InsertPoint(6, .5, 1, 0)

    aWedge = vtk.vtkWedge()
    aWedge.GetPointIds().SetId(0, 0)
    aWedge.GetPointIds().SetId(1, 1)
    aWedge.GetPointIds().SetId(2, 2)
    aWedge.GetPointIds().SetId(3, 3)
    aWedge.GetPointIds().SetId(4, 4)
    aWedge.GetPointIds().SetId(5, 5)

    aTetra = vtk.vtkTetra()
    aTetra.GetPointIds().SetId(0, 0)
    aTetra.GetPointIds().SetId(1, 4)
    aTetra.GetPointIds().SetId(2, 6)
    aTetra.GetPointIds().SetId(3, 3)

    result = vtk.vtkUnstructuredGrid()
    result.SetPoints(points)
    result.Allocate(2)
    result.InsertNextCell(aTetra.GetCellType(), aTetra.GetPointIds())
    result.InsertNextCell(aWedge.GetCellType(), aWedge.GetPointIds())

    return result


def test_same_datasets(ds, ds2):
    EPSILON = 1.e-6
    assert (ds.GetNumberOfPoints() == ds2.GetNumberOfPoints())
    assert (ds.GetNumberOfCells() == ds2.GetNumberOfCells())

    it = ds.NewCellIterator()
    it2 = ds2.NewCellIterator()
    it.InitTraversal()
    it2.InitTraversal()
    while True:
        if it.IsDoneWithTraversal() or it2.IsDoneWithTraversal():
            break
        assert (it.GetCellType() == it2.GetCellType())
        assert (it.GetNumberOfPoints() == it2.GetNumberOfPoints())
        points = it.GetPoints()
        points2 = it2.GetPoints()
        xyz = [0., 0., 0.]
        xyz2 = [0., 0., 0.]
        for i in range(points.GetNumberOfPoints()):
            points.GetPoint(i, xyz)
            points2.GetPoint(i, xyz2)
            for j in range(3):
                assert (abs(xyz[j] - xyz2[j]) < EPSILON)
        it.GoToNextCell()
        it2.GoToNextCell()


def verify_tri_polydata():
    imprt = smtk.io.vtk.ImportVTKData()

    pd = make_TrianglePolyData()
    meshResource = smtk.mesh.Resource.create()
    imprt(pd, meshResource)

    if meshResource == None or not meshResource.isValid():
        raise RuntimeError("resource should exist")
    if meshResource.numberOfMeshes() != 1:
        raise RuntimeError("resource should only have a single mesh")
    if meshResource.cells().size() != pd.GetNumberOfCells():
        raise RuntimeError(
            "resource and polydata should have the same number of cells")

    meshes = meshResource.meshes(smtk.mesh.Dims2)
    if meshes.size() != 1:
        raise RuntimeError("wrong number of meshes")
    if meshes.cells() != meshResource.cells():
        raise RuntimeError("cell sets should match")

    meshes1d = meshResource.meshes(smtk.mesh.Dims1)
    if meshes1d.size() != 0:
        raise RuntimeError("number of 1d cells should be 0")

    exprt = smtk.io.vtk.ExportVTKData()
    pd2 = vtk.vtkPolyData()
    exprt(meshes, pd2)
    test_same_datasets(pd, pd2)


def verify_tri_ugrid():
    imprt = smtk.io.vtk.ImportVTKData()

    ug = make_TriangleUGrid()
    meshResource = smtk.mesh.Resource.create()
    imprt(ug, meshResource)

    if meshResource == None or not meshResource.isValid():
        raise RuntimeError("resource should be valid")
    if meshResource.numberOfMeshes() != 1:
        raise RuntimeError("resource should only have a single mesh")

    meshes = meshResource.meshes(smtk.mesh.Dims2)
    if meshes.size() != 1:
        raise RuntimeError("wrong number of meshes")
    if meshes.cells() != meshResource.cells():
        raise RuntimeError("cell sets should match")

    meshes1d = meshResource.meshes(smtk.mesh.Dims1)
    if meshes1d.size() != 0:
        raise RuntimeError("number of 1d cells should be 0")

    exprt = smtk.io.vtk.ExportVTKData()
    ug2 = vtk.vtkUnstructuredGrid()
    exprt(meshes, ug2)
    test_same_datasets(ug, ug2)


def verify_mixed_cell_ugrid():
    imprt = smtk.io.vtk.ImportVTKData()

    ug = make_MixedVolUGrid()
    meshResource = smtk.mesh.Resource.create()
    imprt(ug, meshResource)

    if meshResource == None or not meshResource.isValid():
        raise RuntimeError("resource should be valid")
    if meshResource.numberOfMeshes() != 1:
        raise RuntimeError("resource should only have a single mesh")
    if meshResource.cells().size() != ug.GetNumberOfCells():
        raise RuntimeError(
            "resource and unsigned grid should have the same number of cells")

    meshes = meshResource.meshes(smtk.mesh.Dims3)
    if meshes.size() != 1:
        raise RuntimeError("wrong number of meshes")
    if meshes.cells() != meshResource.cells():
        raise RuntimeError("cell sets should match")

    exprt = smtk.io.vtk.ExportVTKData()
    ug2 = vtk.vtkUnstructuredGrid()
    exprt(meshes, ug2)
    test_same_datasets(ug, ug2)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    verify_tri_polydata()
    verify_tri_ugrid()
    verify_mixed_cell_ugrid()
