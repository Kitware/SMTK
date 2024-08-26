//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/ModelToMesh.h"

#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/model/Resource.h"
#include "smtk/model/json/jsonResource.h"

#include "vtkActor.h"
#include "vtkColorTransferFunction.h"
#include "vtkCommand.h"
#include "vtkCompositePolyDataMapper.h"
#include "vtkDataSetAttributes.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRegressionTestImage.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkXMLMultiBlockDataWriter.h"

#include "smtk/extension/vtk/mesh/RegisterVTKBackend.h"
#include "smtk/extension/vtk/source/vtkResourceMultiBlockSource.h"

using namespace smtk::model;
using namespace smtk::io;

int main(int argc, char* argv[])
{
  int debug = argc > 2 ? (argv[2][0] == '-' ? 0 : 1) : 0;
  if (argc > 1)
  {
    std::ifstream file;
    file.open(argv[1]);
    if (!file.good())
    {
      std::cout << "Could not open file \"" << argv[1] << "\".\n\n";
      return 1;
    }

    std::string data((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

    ResourcePtr resource = Resource::create();

    nlohmann::json json = nlohmann::json::parse(data);

    smtk::model::from_json(json, resource);
    for (auto& tessPair : json["tessellations"])
    {
      smtk::common::UUID id = tessPair[0];
      smtk::model::Tessellation tess = tessPair[1];
      resource->setTessellation(id, tess);
    }

    resource->assignDefaultNames();

    bool status = true;

    int numModels =
      static_cast<int>(resource->entitiesMatchingFlagsAs<Models>(smtk::model::MODEL_ENTITY).size());
    std::cout << "Imported models into resource: " << numModels << std::endl;
    if (numModels > 0)
    {
      smtk::io::ModelToMesh convert;
      smtk::mesh::ResourcePtr c = convert(resource);
      test(c->isValid(), "resource should be valid");

      std::size_t numMeshes = c->numberOfMeshes();
      std::cout << "number of meshes: " << numMeshes << std::endl;
      test(numMeshes != 0, "dataset once loaded should have more than zero meshes");

      smtk::extension::vtk::mesh::RegisterVTKBackend::registerClass();

      vtkNew<vtkActor> act;
      vtkNew<vtkResourceMultiBlockSource> src;
      vtkNew<vtkCompositePolyDataMapper> map;
      vtkNew<vtkRenderer> ren;
      vtkNew<vtkRenderWindow> win;
      src->SetResource(c);
      if (debug)
      {
        win->SetMultiSamples(16);
      }
      else
      {
        win->SetMultiSamples(0);
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
      vtkInteractorStyleSwitch::SafeDownCast(iac->GetInteractorStyle())
        ->SetCurrentStyleToTrackballCamera();
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

      status = !vtkRegressionTestImage(win.GetPointer());
      if (debug)
      {
        iac->Start();
      }
    }

    return status;
  }

  return 0;
}
