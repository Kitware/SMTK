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

#include "vtkCmbModelEdgeMeshOperator.h"

#include "vtkCmbModelEdgeMeshServer.h"
#include "vtkCmbMeshServer.h"
#include "vtkCmbMeshWrapper.h"
#include <vtkModel.h>
#include <vtkModelEdge.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkCmbModelEdgeMeshOperator);

vtkCmbModelEdgeMeshOperator::vtkCmbModelEdgeMeshOperator()
{
  this->OperateSucceeded = 0;
  this->Id = 0;
  this->Length = 0;
  this->BuildModelEntityMesh = 0;
  this->MeshHigherDimensionalEntities = 0;
}

vtkCmbModelEdgeMeshOperator::~vtkCmbModelEdgeMeshOperator()
{
}

void vtkCmbModelEdgeMeshOperator::Operate(vtkCmbMeshWrapper* meshWrapper)
{
  if(this->Id == 0)
    {  // id not set
    this->OperateSucceeded = 0;
    return;
    }
  this->OperateSucceeded = 1;
  vtkCmbMeshServer* mesh = meshWrapper->GetMesh();
  vtkModel* model = mesh->GetModel();
  vtkModelEdge* modelEdge =
    vtkModelEdge::SafeDownCast(model->GetModelEntity(vtkModelEdgeType, this->Id));
  vtkCmbModelEdgeMeshServer* edgeMesh = vtkCmbModelEdgeMeshServer::SafeDownCast(
    mesh->GetModelEntityMesh(modelEdge));
  edgeMesh->SetLength(this->Length);
  if(this->BuildModelEntityMesh)
    {
    this->OperateSucceeded =
      edgeMesh->BuildModelEntityMesh(this->MeshHigherDimensionalEntities);
    }
  return;
}

void vtkCmbModelEdgeMeshOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
  os << indent << "Id: " << this->Id << endl;
  os << indent << "Length: " << this->Length << endl;
  os << indent << "BuildModelEntityMesh: "
     << this->BuildModelEntityMesh << endl;
  os << indent << "MeshHigherDimensionalEntities: "
     << this->MeshHigherDimensionalEntities << endl;
}
