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

#include "vtkCmbModelFaceMeshOperator.h"

#include "vtkCmbModelFaceMeshServer.h"
#include "vtkCmbMeshServer.h"
#include "vtkCmbMeshWrapper.h"
#include <vtkModel.h>
#include <vtkModelFace.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkCmbModelFaceMeshOperator);

vtkCmbModelFaceMeshOperator::vtkCmbModelFaceMeshOperator()
{
  this->OperateSucceeded = 0;
  this->Id = 0;
  this->Length = 0;
  this->MinimumAngle = 0;
  this->BuildModelEntityMesh = 0;
  this->MeshHigherDimensionalEntities = 0;
}

vtkCmbModelFaceMeshOperator::~vtkCmbModelFaceMeshOperator()
{
}

void vtkCmbModelFaceMeshOperator::Operate(vtkCmbMeshWrapper* meshWrapper)
{
  if(this->Id == 0)
    {  // id not set
    this->OperateSucceeded = 0;
    return;
    }
  vtkCmbMeshServer* mesh = meshWrapper->GetMesh();
  vtkModel* model = mesh->GetModel();
  vtkModelFace* modelFace =
    vtkModelFace::SafeDownCast(model->GetModelEntity(vtkModelFaceType, this->Id));
  vtkCmbModelFaceMeshServer* faceMesh = vtkCmbModelFaceMeshServer::SafeDownCast(
    mesh->GetModelEntityMesh(modelFace));
  if(!faceMesh)
    {
    vtkWarningMacro("There is no face mesh on the server for changing local parameters");
    this->OperateSucceeded = 0;
    return;
    }
  faceMesh->SetLength(this->Length);
  faceMesh->SetMinimumAngle(this->MinimumAngle);
  this->OperateSucceeded = 1;
  if(this->BuildModelEntityMesh)
    {
    this->OperateSucceeded =
      faceMesh->BuildModelEntityMesh(this->MeshHigherDimensionalEntities);
    }

  return;
}

void vtkCmbModelFaceMeshOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
  os << indent << "Id: " << this->Id << endl;
  os << indent << "Length: " << this->Length << endl;
  os << indent << "MinimumAngle: " << this->MinimumAngle << endl;
  os << indent << "BuildModelEntityMesh: "
     << this->BuildModelEntityMesh << endl;
  os << indent << "MeshHigherDimensionalEntities: "
     << this->MeshHigherDimensionalEntities << endl;
}
