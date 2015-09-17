//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/io/ImportMesh.h"

#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/model/Manager.h"

#include "vtkActor.h"
#include "vtkColorTransferFunction.h"
#include "vtkCommand.h"
#include "vtkCompositePolyDataMapper2.h"
#include "vtkDataSetAttributes.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkXMLMultiBlockDataWriter.h"

#include "smtk/extension/vtk/vtkMeshMultiBlockSource.h"

namespace
{

// SMTK_DATA_DIR is a define setup by cmake
// std::string data_root = SMTK_DATA_DIR;

int render_meshes(const smtk::mesh::ManagerPtr& meshmgr,
  const smtk::common::UUID& collectionID )
{
  bool debug = true;

  vtkNew<vtkActor> act;
  vtkNew<vtkMeshMultiBlockSource> src;
  vtkNew<vtkCompositePolyDataMapper2> map;
  vtkNew<vtkRenderer> ren;
  vtkNew<vtkRenderWindow> win;
  src->SetMeshManager(meshmgr);
  src->SetMeshCollectionID(collectionID.toString().c_str());
  if(debug)
    {
    win->SetMultiSamples(16);
    src->AllowNormalGenerationOn();
    }
  map->SetInputConnection(src->GetOutputPort());
  /*
  map->SetColorModeToMapScalars();
  map->SetScalarModeToUseCellData();
  map->SelectColorArray("vtkCompositeIndex");
  vtkSmartPointer< vtkColorTransferFunction > lut =
      vtkSmartPointer< vtkColorTransferFunction >::New();
  // This creates a blue to red lut.
  //double range[2];
  lut->AddHSVPoint(0,0.667,1,1);
  lut->AddHSVPoint(55,0,1,1);
  lut->SetColorSpaceToDiverging();
  lut->SetVectorModeToMagnitude();
  map->SetLookupTable(lut);
  map->SetInterpolateScalarsBeforeMapping(1);
  */
  act->SetMapper(map.GetPointer());
  act->GetProperty()->SetPointSize(5);
  act->GetProperty()->SetLineWidth(2);
  act->GetProperty()->SetEdgeVisibility(1);
  act->GetProperty()->SetEdgeColor(0, 0, 0.5);
  win->AddRenderer(ren.GetPointer());
  ren->AddActor(act.GetPointer());

  vtkRenderWindowInteractor* iac = win->MakeRenderWindowInteractor();
  vtkInteractorStyleSwitch::SafeDownCast(iac->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
  win->SetInteractor(iac);
/*
  if (debug && argc > 3)
    {
    vtkNew<vtkXMLMultiBlockDataWriter> wri;
    wri->SetInputConnection(src->GetOutputPort());
    wri->SetFileName(argv[3]);
    wri->Write();
    }
*/
  win->Render();
  ren->ResetCamera();

//  status = ! vtkRegressionTestImage(win.GetPointer());

  if (debug)
    {
    iac->Start();
    }

  return 0;
}

//----------------------------------------------------------------------------
int verify_load_valid_mesh(const std::string& file_path)
{
//  std::string file_path(data_root);
//  file_path += "/mesh/twoassm_out.h5m";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = smtk::io::ImportMesh::entireFile(file_path, manager);
  test( c->isValid(), "collection should be valid");

  std::size_t numMeshes = c->numberOfMeshes();
  std::cout << "number of meshes in twoassm_out is: " << numMeshes << std::endl;
  test( numMeshes!=0, "dataset once loaded should have more than zero meshes");
  // test( numMeshes == 53, "dataset once loaded should have 53 meshes");

  return render_meshes(manager, c->entity());
}

}

int main(int argc, char* argv[])
{
  if (argc > 1)
    return verify_load_valid_mesh(argv[1]);

  return 0;
}
