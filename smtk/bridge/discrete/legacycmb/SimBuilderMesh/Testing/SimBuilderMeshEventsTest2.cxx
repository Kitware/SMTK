//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include <vtkCMBMeshServer.h>
#include <vtkDiscreteModel.h>
#include <vtkCMBModelEdgeMeshServer.h>
#include <vtkCMBModelEntityMesh.h>
#include <vtkCMBModelFaceMeshServer.h>
#include <vtkCMBModelReadOperator.h>
#include <vtkDiscreteModelWrapper.h>
#include <vtkEdgeSplitOperator.h>
#include <vtkMergeOperator.h>
#include <vtkModelEdge.h>
#include <vtkModelFace.h>
#include <vtkModelItemIterator.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

// This tests the events for the SimBuilder Mesh.

int Check2DModel(const char* fileName)
{
  int numberOfErrors = 0;
  vtkDiscreteModelWrapper* modelWrapper = vtkDiscreteModelWrapper::New();

  vtkDiscreteModel* model = modelWrapper->GetModel();

  vtkSmartPointer<vtkCMBModelReadOperator> reader =
    vtkSmartPointer<vtkCMBModelReadOperator>::New();
  reader->SetFileName(fileName);
  reader->Operate(modelWrapper);
  if(reader->GetOperateSucceeded() == false)
    {
    vtkGenericWarningMacro("Could not load file " << fileName);
    return 1;
    }

  vtkSmartPointer<vtkCMBMeshServer> mesh =
    vtkSmartPointer<vtkCMBMeshServer>::New();
  mesh->Initialize(model);
  mesh->SetGlobalLength(100.);

  vtkSmartPointer<vtkModelItemIterator> edges;
  edges.TakeReference(model->NewIterator(vtkModelEdgeType));
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkModelEdge* edge = vtkModelEdge::SafeDownCast(edges->GetCurrentItem());
    vtkCMBModelEdgeMesh* edgeMesh = vtkCMBModelEdgeMesh::SafeDownCast(
      mesh->GetModelEntityMesh(edge));
    edgeMesh->BuildModelEntityMesh(false);
    }

  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkModelEdge* edge = vtkModelEdge::SafeDownCast(edges->GetCurrentItem());
    vtkCMBModelEdgeMesh* edgeMesh = vtkCMBModelEdgeMesh::SafeDownCast(
      mesh->GetModelEntityMesh(edge));
    if(edgeMesh->GetLength() != 0.)
      {
      numberOfErrors++;
      vtkGenericWarningMacro("Model entity mesh size set when it shouldn't be.");
      }
    if(edgeMesh->GetModelEntityMesh() == NULL ||
       edgeMesh->GetModelEntityMesh()->GetNumberOfCells() == 0)
      {
      numberOfErrors++;
      vtkGenericWarningMacro("Missing a valid mesh of a model entity.");
      }
    }

  // test model face meshing
  mesh->SetGlobalMinimumAngle(30);
  vtkSmartPointer<vtkModelItemIterator> faces;
  faces.TakeReference(model->NewIterator(vtkModelFaceType));
  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
    vtkCMBModelFaceMesh* faceMesh = vtkCMBModelFaceMesh::SafeDownCast(
      mesh->GetModelEntityMesh(face));
    faceMesh->BuildModelEntityMesh(false);
    }

  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
    vtkCMBModelFaceMesh* faceMesh = vtkCMBModelFaceMesh::SafeDownCast(
      mesh->GetModelEntityMesh(face));
    if(faceMesh->GetLength() != 0. || faceMesh->GetMinimumAngle() != 0.)
      {
      numberOfErrors++;
      vtkGenericWarningMacro("Model face mesh parameter set when it shouldn't be.");
      }
    if(faceMesh->GetModelEntityMesh() == NULL ||
       faceMesh->GetModelEntityMesh()->GetNumberOfCells() == 0)
      {
      numberOfErrors++;
      vtkGenericWarningMacro("Missing a valid mesh of a model entity.");
      }
    }

  model->Reset();
  modelWrapper->Delete();
  return numberOfErrors;
}

int main(int argc, char ** argv)
{
  if(argc != 2)
    {
    vtkGenericWarningMacro("Not enough arguments -- need to specify a 2D CMB model file.");
    return 1;
    }
  int errors = Check2DModel(argv[1]);
  //errors += Check3DModel(argv[2]);
  std::cout << "Finished with " << errors << " errors.\n";
  return errors;
}
