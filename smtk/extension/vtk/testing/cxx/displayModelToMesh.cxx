//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/ImportJSON.h"
#include "smtk/io/ModelToMesh.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
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
#include "vtkRegressionTestImage.h"

#include "smtk/extension/vtk/vtkMeshMultiBlockSource.h"

using namespace smtk::model;
using namespace smtk::io;

int main(int argc, char* argv[])
{
  int debug = argc > 2 ? (argv[2][0] == '-' ? 0 : 1) : 0;
  if (argc > 1 )
    {
    std::ifstream file;
    file.open(argv[1]);
    if(!file.good())
    {
    std::cout
      << "Could not open file \"" << argv[1] << "\".\n\n";
      return 1;
    }

    std::string data(
      (std::istreambuf_iterator<char>(file)),
      (std::istreambuf_iterator<char>()));

    ManagerPtr sm = Manager::create();

    int status = ImportJSON::intoModelManager(data.c_str(), sm);
    int numModels = sm->entitiesMatchingFlagsAs<Models>(smtk::model::MODEL_ENTITY).size();
    std::cout << "Imported models into manager: " << numModels << std::endl;
    if (numModels > 0)
      {
      smtk::mesh::ManagerPtr meshmgr = sm->meshes();
      smtk::io::ModelToMesh convert;
      smtk::mesh::CollectionPtr c = convert(meshmgr, sm);
      test( c->isValid(), "collection should be valid");

      std::size_t numMeshes = c->numberOfMeshes();
      std::cout << "number of meshes: " << numMeshes << std::endl;
      test( numMeshes!=0, "dataset once loaded should have more than zero meshes");

      smtk::common::UUID collectionID = c->entity();
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

      status = ! vtkRegressionTestImage(win.GetPointer());
      if (debug)
        {
        iac->Start();
        }
      }

    return status;
    }

  return 0;
}
