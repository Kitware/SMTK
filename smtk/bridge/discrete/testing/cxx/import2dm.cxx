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
#include "smtk/model/Operator.h"

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/common/UUID.h"

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
#include "vtkRegressionTestImage.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkXMLMultiBlockDataWriter.h"

#include "smtk/bridge/discrete/Session.h"
#include "smtk/extension/vtk/source/vtkMeshMultiBlockSource.h"

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

    int status = 1;
    ManagerPtr mgr = Manager::create();
    smtk::bridge::discrete::Session::Ptr brg = smtk::bridge::discrete::Session::create();
//    Session::Ptr brg = mgr->createSessionOfType("discrete");
    mgr->registerSession(brg);
    Operator::Ptr op;
    OperatorResult result;

    op = brg->op("import");
    op->findFile("filename")->setValue(argv[1]);
    result = op->operate();
    if (result->findInt("outcome")->value() != OPERATION_SUCCEEDED)
      {
      std::cerr << "Import 2dm Failed: " << argv[1] << std::endl;
      return 1;
      }
    Model model2dm = result->findModelEntity("created")->value();

    if (model2dm.isValid())
      {
      smtk::mesh::ManagerPtr meshmgr = mgr->meshes();
        typedef std::vector< smtk::mesh::CollectionPtr > AssocCollections;
      AssocCollections assocCollections = meshmgr->collectionsWithAssociations();
      test(assocCollections.size() == 2, "expecting 2 mesh collections");

      smtk::mesh::CollectionPtr mc = assocCollections[0];
      if (mc->entity() == model2dm.entity())
	{
	// this collection has the same entity id as the model. It holds the
	// tessellation meshes for each model entity. We are looking for the
	// mesh that is affiliated with the model, not the one that represents
	// its tessellation.
	mc = assocCollections[1];
	}
      test((mc->meshes(smtk::mesh::Dims2)).size() == 4, "Expecting 4 face mesh");
      test((mc->meshes(smtk::mesh::Dims1)).size() == 10, "Expecting 10 edge mesh");
      test((mc->meshes(smtk::mesh::Dims0)).size() == 7, "Expecting 7 vertex mesh");

      smtk::common::UUID collectionID = mc->entity();
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
      else
	{
	win->SetMultiSamples(0);
	}
      map->SetInputConnection(src->GetOutputPort());

      act->SetMapper(map.GetPointer());
      act->GetProperty()->SetPointSize(5);
      act->GetProperty()->SetLineWidth(1);
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
