//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "cmbFaceMeshHelper.h"
#include <vtkCMBMeshServer.h>
#include <vtkCMBModelEdgeMesh.h>
#include <vtkCMBModelEntityMesh.h>
#include <vtkCMBModelFaceMesh.h>
#include <vtkCMBModelReadOperator.h>
#include <vtkCellData.h>
#include <vtkDiscreteModel.h>
#include <vtkDiscreteModelWrapper.h>
#include <vtkEdgeSplitOperator.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkMergeOperator.h>
#include <vtkModelEdge.h>
#include <vtkModelFace.h>
#include <vtkModelItemIterator.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

using namespace discreteFaceMesherClasses;

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

  vtkSmartPointer<vtkModelItemIterator> edges;
  edges.TakeReference(model->NewIterator(vtkModelEdgeType));

  vtkSmartPointer<vtkModelItemIterator> faces;
  faces.TakeReference(model->NewIterator(vtkModelFaceType));

  mesh->SetGlobalLength(1);
  mesh->SetGlobalMinimumAngle(20);
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkModelEdge* edge = vtkModelEdge::SafeDownCast(edges->GetCurrentItem());
    vtkCMBModelEdgeMesh* edgeMesh = vtkCMBModelEdgeMesh::SafeDownCast(
      mesh->GetModelEntityMesh(edge));
    edgeMesh->BuildModelEntityMesh(false);
    }
  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
    vtkCMBModelFaceMesh* faceMesh = vtkCMBModelFaceMesh::SafeDownCast(
      mesh->GetModelEntityMesh(face));
    faceMesh->BuildModelEntityMesh(false);
    }

  //lets now confirm each faces model mapping on the mesh
  //We are going to confirm the mapping by verifying the number of
  //each model types per face on the points.

  //face ids are 26,27,28,29. so we are going to get the correct index
  //by subtracting 26 from the id. so 26 == 0, ... 29 == 3
  //index 1 == verts, 3 == edge, 6 == face

  //face 26 == 2 verts, 12 edges, 10 face points
  //face 27 == 5 verts, 9 edges, 7 face points
  //face 28 == 3 verts, 7 edges, 3 face points
  //face 29 == 4 verts, 14 edges, 8 face points
  const vtkIdType correctPointIdSums[4][7]={
    {0,2,0,12,0,0,10},
    {0,5,0,9,0,0,7},
    {0,3,0,7,0,0,3},
    {0,4,0,14,0,0,8}
    };


  vtkIdType foundPointIdSums[4][7] = {
    {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}
    };

  //verify points
  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
    vtkCMBModelFaceMesh* faceMesh = vtkCMBModelFaceMesh::SafeDownCast(mesh->GetModelEntityMesh(face));
    vtkPolyData *meshTmp = faceMesh->GetModelEntityMesh();

    vtkIdType faceId = face->GetUniquePersistentId() - 26;
    vtkIntArray *types = vtkIntArray::SafeDownCast(
      meshTmp->GetPointData()->GetArray(
      ModelFaceRep::Get2DAnalysisPointModelTypesString()));
    for (vtkIdType i=0; i < types->GetNumberOfTuples(); ++i)
      {
      foundPointIdSums[faceId][types->GetValue(i)]++;
      }

    //verify this face is correct
    bool valid = false;
    for (vtkIdType i=0; i < 7; ++i)
      {
      valid = foundPointIdSums[faceId][i] == correctPointIdSums[faceId][i];
      if ( valid == false )
        {
        cerr << "ERROR: Point Mapping failed on FaceId: " << 26 + faceId << endl;
        cerr << "i is :" << i << endl;
        cerr << "foundCellIdSums: " << foundPointIdSums[faceId][i] << endl;
        cerr << "correctCellIdSums: " << correctPointIdSums[faceId][i] << endl;
        cerr << endl;
        numberOfErrors++;
        break;
        }
      }
    }

  //now that the point mapping has been checked we are going to verify the cell
  //mapping. This is going to be done by looking at the edges. Each edge of the triangle
  //is assigned to an edge or face. so we can do a sum check on these too. The summed
  //lookup table is:
  // 0 edge model Id's on the triangle = index 18
  // 1 edge model Id's on the triangle = index 15
  // 2 edge model Id's on the triangle = inde 12
  // 3 edge model Id's on the triangle = inde 9

  //face 26 == 1 2 edge cell, 12 1 edge cells, 19 face cells
  //face 27 == 2 2 edge cell, 10 1 edge cells, 14 face cells
  //face 28 == 2 2 edge cell, 6 1 edge cells, 6 face cells
  //face 29 == 1 2 edge cell, 16 1 edge cells, 17 face cells


  const int correctCellIdSums[4][19]={
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,12,0,0,19},
    {0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,0,0,14},
    {0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,6,0,0,6},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,16,0,0,17}
    };

  int foundCellIdSums[4][19]={
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    };

  //verify cells
  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
    vtkCMBModelFaceMesh* faceMesh = vtkCMBModelFaceMesh::SafeDownCast(mesh->GetModelEntityMesh(face));
    vtkPolyData *meshTmp = faceMesh->GetModelEntityMesh();

    vtkIdType faceId = face->GetUniquePersistentId() - 26;
    vtkIntArray *types = vtkIntArray::SafeDownCast(
      meshTmp->GetCellData()->GetArray(
      ModelFaceRep::Get2DAnalysisCellModelTypesString()));
    for (vtkIdType i=0; i < types->GetNumberOfTuples(); ++i)
      {
      int sum=0;
      for(vtkIdType j=0; j < types->GetNumberOfComponents(); ++j)
        {
        sum += types->GetComponent(i,j);
        }
      foundCellIdSums[faceId][sum]++;
      }

    //verify this face is correct
    bool valid = false;
    for (vtkIdType i=0; i < 19; ++i)
      {
      valid = foundCellIdSums[faceId][i] == correctCellIdSums[faceId][i];
      if ( valid == false )
        {
        cerr << "ERROR: Cell Mapping failed on FaceId: " << 26 + faceId << endl;
        cerr << "i is :" << i << endl;
        cerr << "foundCellIdSums: " << foundCellIdSums[faceId][i] << endl;
        cerr << "correctCellIdSums: " << correctCellIdSums[faceId][i] << endl;
        cerr << endl;
        numberOfErrors++;
        break;
        }
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
