//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtkToMesh/VTKDataConverter.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include "vtkAppendFilter.h"
#include "vtkNew.h"
#include "vtkParametricBoy.h"
#include "vtkParametricFunctionSource.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"
#include "vtkTetra.h"
#include "vtkUnstructuredGrid.h"
#include "vtkWedge.h"

namespace
{

//----------------------------------------------------------------------------
vtkSmartPointer< vtkPolyData > make_EmptyPolyData()
{
  return vtkSmartPointer< vtkPolyData >::New();
}

//----------------------------------------------------------------------------
vtkSmartPointer< vtkPolyData > make_TrianglePolyData()
{
  //use a vtk parametric source to construct polydata on the fly
  vtkNew<vtkParametricBoy> PB;
  vtkNew<vtkParametricFunctionSource> PFS;
  PFS->SetParametricFunction(PB.Get());

  PFS->SetUResolution( 400 );
  PFS->SetVResolution( 500 );
  PFS->Update();

  vtkSmartPointer< vtkPolyData > result = vtkSmartPointer< vtkPolyData >::New();

  result->ShallowCopy( PFS->GetOutput() );
  return result;
}

//----------------------------------------------------------------------------
vtkSmartPointer< vtkUnstructuredGrid > make_TriangleUGrid()
{
  //use a vtk parametric source to construct polydata on the fly
  vtkNew<vtkParametricBoy> PB;
  vtkNew<vtkParametricFunctionSource> PFS;
  PFS->SetParametricFunction(PB.Get());

  PFS->SetUResolution( 400 );
  PFS->SetVResolution( 500 );
  PFS->Update();

  vtkNew<vtkAppendFilter> appendFilter;
  appendFilter->AddInputData(PFS->GetOutput());
  appendFilter->Update();

  vtkSmartPointer<vtkUnstructuredGrid> result =
     vtkSmartPointer<vtkUnstructuredGrid>::New();
  result->ShallowCopy(appendFilter->GetOutput());
  return result;
}

//----------------------------------------------------------------------------
vtkSmartPointer< vtkUnstructuredGrid > make_MixedVolUGrid()
{
  //manually create a mixed wedge and tet volume
  vtkNew<vtkPoints> points;
  points->SetNumberOfPoints(8);
  points->InsertPoint(0, 0, 1, 0);
  points->InsertPoint(1, 0, 0, 0);
  points->InsertPoint(2, 0, .5, .5);
  points->InsertPoint(3, 1, 1, 0);
  points->InsertPoint(4, 1, 0, 0);
  points->InsertPoint(5, 1, .5, .5);
  points->InsertPoint(6, .5, 1, 0);
  points->InsertPoint(7, .5, .5, 1);

  vtkNew<vtkWedge> aWedge;
  aWedge->GetPointIds()->SetId(0, 0);
  aWedge->GetPointIds()->SetId(1, 1);
  aWedge->GetPointIds()->SetId(2, 2);
  aWedge->GetPointIds()->SetId(3, 3);
  aWedge->GetPointIds()->SetId(4, 4);
  aWedge->GetPointIds()->SetId(5, 5);

  vtkNew<vtkTetra> aTetra;
  aTetra->GetPointIds()->SetId(0, 0);
  aTetra->GetPointIds()->SetId(1, 4);
  aTetra->GetPointIds()->SetId(2, 6);
  aTetra->GetPointIds()->SetId(3, 3);

  vtkSmartPointer<vtkUnstructuredGrid> result =
     vtkSmartPointer<vtkUnstructuredGrid>::New();
  result->SetPoints( points.GetPointer() );
  result->Allocate(1);
  result->InsertNextCell(aWedge->GetCellType(), aWedge->GetPointIds());
  //result->InsertNextCell(aTetra->GetCellType(), aTetra->GetPointIds());

  return result;
}

//----------------------------------------------------------------------------
void verify_null_polydata()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::extension::vtkToMesh::VTKDataConverter convert(manager);

  vtkPolyData* pd = NULL;
  smtk::mesh::CollectionPtr c = convert(pd);
  test( !c, "collection should be invalid for a NULL poly data");
}

//----------------------------------------------------------------------------
void verify_empty_polydata()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::extension::vtkToMesh::VTKDataConverter convert(manager);

  smtk::mesh::CollectionPtr c = convert( make_EmptyPolyData() );
  test( !c, "collection should invalid for empty poly data");
}

//----------------------------------------------------------------------------
void verify_tri_polydata()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::extension::vtkToMesh::VTKDataConverter convert(manager);

  vtkSmartPointer< vtkPolyData > pd = make_TrianglePolyData();
  smtk::mesh::CollectionPtr c = convert( pd );
  test( c->isValid(), "collection should valid");
  test( c->numberOfMeshes() == 1, "collection should only have a single mesh");
  test( c->cells().size() == pd->GetNumberOfCells());

  //this is a triangle pd so it is 2d only
  smtk::mesh::MeshSet meshes = c->meshes( smtk::mesh::Dims2 );
  test( meshes.size() == 1);
  test( meshes.cells() == c->cells());

  smtk::mesh::MeshSet meshes1d = c->meshes( smtk::mesh::Dims1 );
  test( meshes1d.size() == 0);
}


//----------------------------------------------------------------------------
void verify_tri_ugrid()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::extension::vtkToMesh::VTKDataConverter convert(manager);

  vtkSmartPointer< vtkUnstructuredGrid > ug = make_TriangleUGrid();
  smtk::mesh::CollectionPtr c = convert( ug );
  test( c->isValid(), "collection should valid");
  test( c->numberOfMeshes() == 1, "collection should only have a single mesh");
  test( c->cells().size() == ug->GetNumberOfCells());

  //this is a triangle grid so it is 2d only
  smtk::mesh::MeshSet meshes = c->meshes( smtk::mesh::Dims2 );
  test( meshes.size() == 1);
  test( meshes.cells() == c->cells());

  smtk::mesh::MeshSet meshes1d = c->meshes( smtk::mesh::Dims1 );
  test( meshes1d.size() == 0);
}

//----------------------------------------------------------------------------
void verify_mixed_cell_ugrid()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::extension::vtkToMesh::VTKDataConverter convert(manager);

  vtkSmartPointer< vtkUnstructuredGrid > ug = make_MixedVolUGrid();
  smtk::mesh::CollectionPtr c = convert( ug );

  std::cout << "number of cells: " << c->cells().size() << std::endl;
  std::cout << "number of cells ug: " <<ug->GetNumberOfCells() << std::endl;

  test( c->isValid(), "collection should valid");
  test( c->numberOfMeshes() == 1, "collection should only have a single mesh");
  test( c->cells().size() == ug->GetNumberOfCells(), "number of cells in mesh don't match");

  //this is a volume only grid
  smtk::mesh::MeshSet meshes = c->meshes( smtk::mesh::Dims3 );
  test( meshes.size() == 1);
  test( meshes.cells() == c->cells());
}

}

//----------------------------------------------------------------------------
int UnitTestVTKDataConverter(int argc, char** argv)
{

  verify_null_polydata();

  verify_empty_polydata();

  verify_tri_polydata();

  verify_tri_ugrid();

  verify_mixed_cell_ugrid();

  return 0;
}
