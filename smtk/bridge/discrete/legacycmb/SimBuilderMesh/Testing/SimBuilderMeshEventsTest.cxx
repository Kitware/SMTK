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
#include <vtkCMBModelEdgeMesh.h>
#include <vtkCMBModelEntityMesh.h>
#include <vtkCMBModelFaceMesh.h>
#include <vtkCMBModelReadOperator.h>
#include <vtkDiscreteModel.h>
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
  mesh->SetGlobalLength(1.);
  vtkSmartPointer<vtkModelItemIterator> edges;
  edges.TakeReference(model->NewIterator(vtkModelEdgeType));
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkModelEdge* vmEdge = vtkModelEdge::SafeDownCast(edges->GetCurrentItem());
    vtkCMBModelEdgeMesh* edgeMesh = vtkCMBModelEdgeMesh::SafeDownCast(
      mesh->GetModelEntityMesh(vmEdge));
    edgeMesh->BuildModelEntityMesh(false);
    }

  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkModelEdge* vmEdge = vtkModelEdge::SafeDownCast(edges->GetCurrentItem());
    vtkCMBModelEdgeMesh* edgeMesh = vtkCMBModelEdgeMesh::SafeDownCast(
      mesh->GetModelEntityMesh(vmEdge));
    if(edgeMesh->GetLength() != 0.)
      {
      numberOfErrors++;
      vtkGenericWarningMacro("Model entity mesh size set when it shouldn't be.");
      }
    if(vtkPolyData::SafeDownCast(edgeMesh->GetModelEntityMesh()) == NULL ||
       vtkPolyData::SafeDownCast(edgeMesh->GetModelEntityMesh())->GetNumberOfCells() == 0)
      {
      numberOfErrors++;
      vtkGenericWarningMacro("Missing a valid mesh of a model entity.");
      }
    }

  // test 2D split by splitting an edge adjacent to 2 faces
  vtkSmartPointer<vtkEdgeSplitOperator> splitOperator =
    vtkSmartPointer<vtkEdgeSplitOperator>::New();
  vtkModelEdge* edge = vtkModelEdge::SafeDownCast(
    model->GetModelEntity(vtkModelEdgeType, 17));
  splitOperator->SetEdgeId(edge->GetUniquePersistentId());
  splitOperator->SetPointId(6);
  splitOperator->Operate(modelWrapper);
  if(splitOperator->GetOperateSucceeded() == false)
    {
    vtkGenericWarningMacro("Model edge split operator failed.");
    numberOfErrors++;
    }
  if(splitOperator->GetCreatedModelEdgeId() < 0)
    {
    vtkGenericWarningMacro("Split operator failed to split any model edges.");
    return ++numberOfErrors;
    }
  // checking all edges is overkill but easy
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkModelEdge* edge = vtkModelEdge::SafeDownCast(edges->GetCurrentItem());
    vtkCMBModelEdgeMesh* edgeMesh = vtkCMBModelEdgeMesh::SafeDownCast(
      mesh->GetModelEntityMesh(edge));
    if(edgeMesh->GetLength() != 0.)
      {
      numberOfErrors++;
      vtkGenericWarningMacro("Model entity mesh size not set.");
      }
    if(vtkPolyData::SafeDownCast(edgeMesh->GetModelEntityMesh()) == NULL ||
       vtkPolyData::SafeDownCast(edgeMesh->GetModelEntityMesh())->GetNumberOfCells() == 0)
      {
      numberOfErrors++;
      vtkGenericWarningMacro("Missing a valid mesh of a model entity.");
      }
    }

  // test 2D merge
  vtkModelGeometricEntity* createdEdge = vtkModelGeometricEntity::SafeDownCast(
    model->GetModelEntity(vtkModelEdgeType, splitOperator->GetCreatedModelEdgeId()));
  vtkCMBModelEdgeMesh* createdMesh = vtkCMBModelEdgeMesh::SafeDownCast(
    mesh->GetModelEntityMesh(createdEdge));
  // set the size to be larger to test that the smaller size is kept
  createdMesh->SetLength(5.);

  vtkSmartPointer<vtkMergeOperator> mergeOperator =
    vtkSmartPointer<vtkMergeOperator>::New();
  mergeOperator->SetSourceId(edge->GetUniquePersistentId());
  mergeOperator->SetTargetId(createdEdge->GetUniquePersistentId());
  mergeOperator->AddLowerDimensionalId(41);
  mergeOperator->Operate(modelWrapper);
  if(mergeOperator->GetOperateSucceeded() == false)
    {
    vtkGenericWarningMacro("Model edge merge operator failed.");
    return ++numberOfErrors;
    }
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkModelEdge* vmEdge = vtkModelEdge::SafeDownCast(edges->GetCurrentItem());
    vtkCMBModelEdgeMesh* edgeMesh = vtkCMBModelEdgeMesh::SafeDownCast(
      mesh->GetModelEntityMesh(vmEdge));
    if(vmEdge->GetUniquePersistentId() == 42)
      {
      if(edgeMesh->GetLength() != 5.)
        {
        numberOfErrors++;
        vtkGenericWarningMacro("Model entity mesh size not set properly.");
        }
      }
    else if(edgeMesh->GetLength() != 0.)
      {
      numberOfErrors++;
      vtkGenericWarningMacro("Model entity mesh size not set properly.");
      }
    if(vtkPolyData::SafeDownCast(edgeMesh->GetModelEntityMesh()) == NULL ||
       vtkPolyData::SafeDownCast(edgeMesh->GetModelEntityMesh())->GetNumberOfCells() == 0)
      {
      numberOfErrors++;
      vtkGenericWarningMacro("Missing a valid mesh of a model entity.");
      }
   }

  // test model face meshing
  mesh->SetGlobalMinimumAngle(10.);
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
    if(vtkPolyData::SafeDownCast(faceMesh->GetModelEntityMesh()) == NULL ||
       vtkPolyData::SafeDownCast(faceMesh->GetModelEntityMesh())->GetNumberOfCells() == 0)
      {
      numberOfErrors++;
      vtkGenericWarningMacro("Missing a valid mesh of a model entity.");
      }
    }

  //Test for bug #9452
  mesh->SetGlobalLength(0.1);
  mesh->SetGlobalMinimumAngle(20);
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkModelEdge* vmEdge = vtkModelEdge::SafeDownCast(edges->GetCurrentItem());
    vtkCMBModelEdgeMesh* edgeMesh = vtkCMBModelEdgeMesh::SafeDownCast(
      mesh->GetModelEntityMesh(vmEdge));
    edgeMesh->BuildModelEntityMesh(false);
    }
  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
    vtkCMBModelFaceMesh* faceMesh = vtkCMBModelFaceMesh::SafeDownCast(
      mesh->GetModelEntityMesh(face));
    faceMesh->BuildModelEntityMesh(false);
    if(vtkPolyData::SafeDownCast(faceMesh->GetModelEntityMesh()) == NULL ||
       vtkPolyData::SafeDownCast(faceMesh->GetModelEntityMesh())->GetNumberOfCells() == 0)
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
