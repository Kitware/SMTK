/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include <vtkCmbMesh.h>
#include <vtkCMBModel.h>
#include <vtkCmbModelEdgeMesh.h>
#include <vtkCmbModelEntityMesh.h>
#include <vtkCmbModelFaceMesh.h>
#include <vtkCMBModelReadOperator.h>
#include <vtkCMBModelWrapper.h>
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
  vtkCMBModelWrapper* modelWrapper = vtkCMBModelWrapper::New();

  vtkCMBModel* model = modelWrapper->GetModel();

  vtkSmartPointer<vtkCMBModelReadOperator> reader =
    vtkSmartPointer<vtkCMBModelReadOperator>::New();
  reader->SetFileName(fileName);
  reader->Operate(modelWrapper);
  if(reader->GetOperateSucceeded() == false)
    {
    vtkGenericWarningMacro("Could not load file " << fileName);
    return 1;
    }

  vtkSmartPointer<vtkCmbMesh> mesh =
    vtkSmartPointer<vtkCmbMesh>::New();
  mesh->Initialize(model);
  mesh->SetGlobalLength(100.);

  vtkSmartPointer<vtkModelItemIterator> edges;
  edges.TakeReference(model->NewIterator(vtkModelEdgeType));
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkModelEdge* edge = vtkModelEdge::SafeDownCast(edges->GetCurrentItem());
    vtkCmbModelEdgeMesh* edgeMesh = vtkCmbModelEdgeMesh::SafeDownCast(
      mesh->GetModelEntityMesh(edge));
    edgeMesh->BuildModelEntityMesh(false);
    }

  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkModelEdge* edge = vtkModelEdge::SafeDownCast(edges->GetCurrentItem());
    vtkCmbModelEdgeMesh* edgeMesh = vtkCmbModelEdgeMesh::SafeDownCast(
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
  mesh->SetGlobalMaximumArea(50);
  mesh->SetGlobalMinimumAngle(30);
  vtkSmartPointer<vtkModelItemIterator> faces;
  faces.TakeReference(model->NewIterator(vtkModelFaceType));
  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
    vtkCmbModelFaceMesh* faceMesh = vtkCmbModelFaceMesh::SafeDownCast(
      mesh->GetModelEntityMesh(face));
    faceMesh->BuildModelEntityMesh(false);
    }

  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
    vtkCmbModelFaceMesh* faceMesh = vtkCmbModelFaceMesh::SafeDownCast(
      mesh->GetModelEntityMesh(face));
    if(faceMesh->GetMaximumArea() != 0. || faceMesh->GetMinimumAngle() != 0.)
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
