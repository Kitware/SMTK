//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelFaceMeshOperator.h"

#include "vtkCMBMeshServer.h"
#include "vtkCMBMeshWrapper.h"
#include "vtkCMBModelFaceMeshServer.h"
#include <vtkModel.h>
#include <vtkModelFace.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkCMBModelFaceMeshOperator);

vtkCMBModelFaceMeshOperator::vtkCMBModelFaceMeshOperator()
{
  this->OperateSucceeded = 0;
  this->FaceMesherFailed = 0;
  this->Id = 0;
  this->Length = 0;
  this->MinimumAngle = 0;
  this->BuildModelEntityMesh = 0;
  this->MeshHigherDimensionalEntities = 0;
}

vtkCMBModelFaceMeshOperator::~vtkCMBModelFaceMeshOperator()
{
}

void vtkCMBModelFaceMeshOperator::Operate(vtkCMBMeshWrapper* meshWrapper)
{
  //reset these to the defaults each time we call operate
  this->OperateSucceeded = 0;
  this->FaceMesherFailed = 0;

  if (this->Id == 0)
  { // id not set
    return;
  }
  vtkCMBMeshServer* mesh = meshWrapper->GetMesh();
  vtkModel* model = mesh->GetModel();
  vtkModelFace* modelFace =
    vtkModelFace::SafeDownCast(model->GetModelEntity(vtkModelFaceType, this->Id));
  vtkCMBModelFaceMeshServer* faceMesh =
    vtkCMBModelFaceMeshServer::SafeDownCast(mesh->GetModelEntityMesh(modelFace));
  if (!faceMesh)
  {
    vtkWarningMacro("There is no face mesh on the server for changing local parameters");
    return;
  }
  faceMesh->SetLength(this->Length);
  faceMesh->SetMinimumAngle(this->MinimumAngle);

  this->OperateSucceeded = 1;
  if (this->BuildModelEntityMesh)
  {
    this->OperateSucceeded =
      faceMesh->BuildModelEntityMesh(this->MeshHigherDimensionalEntities != 0);
    this->FaceMesherFailed = faceMesh->GetFaceMesherFailed();
  }
  return;
}

void vtkCMBModelFaceMeshOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
  os << indent << "Id: " << this->Id << endl;
  os << indent << "Length: " << this->Length << endl;
  os << indent << "MinimumAngle: " << this->MinimumAngle << endl;
  os << indent << "BuildModelEntityMesh: " << this->BuildModelEntityMesh << endl;
  os << indent << "MeshHigherDimensionalEntities: " << this->MeshHigherDimensionalEntities << endl;
}
