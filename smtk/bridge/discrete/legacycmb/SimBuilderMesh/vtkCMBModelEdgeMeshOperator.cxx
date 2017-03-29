//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCMBModelEdgeMeshOperator.h"

#include "vtkCMBMeshServer.h"
#include "vtkCMBMeshWrapper.h"
#include "vtkCMBModelEdgeMeshServer.h"
#include <vtkModel.h>
#include <vtkModelEdge.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkCMBModelEdgeMeshOperator);

vtkCMBModelEdgeMeshOperator::vtkCMBModelEdgeMeshOperator()
{
  this->OperateSucceeded = 0;
  this->Id = 0;
  this->Length = 0;
  this->BuildModelEntityMesh = 0;
  this->MeshHigherDimensionalEntities = 0;
}

vtkCMBModelEdgeMeshOperator::~vtkCMBModelEdgeMeshOperator()
{
}

void vtkCMBModelEdgeMeshOperator::Operate(vtkCMBMeshWrapper* meshWrapper)
{
  if(this->Id == 0)
    {  // id not set
    this->OperateSucceeded = 0;
    return;
    }
  this->OperateSucceeded = 1;
  vtkCMBMeshServer* mesh = meshWrapper->GetMesh();
  vtkModel* model = mesh->GetModel();
  vtkModelEdge* modelEdge =
    vtkModelEdge::SafeDownCast(model->GetModelEntity(vtkModelEdgeType, this->Id));
  vtkCMBModelEdgeMeshServer* edgeMesh = vtkCMBModelEdgeMeshServer::SafeDownCast(
    mesh->GetModelEntityMesh(modelEdge));
  edgeMesh->SetLength(this->Length);
  if(this->BuildModelEntityMesh)
    {
    this->OperateSucceeded =
      edgeMesh->BuildModelEntityMesh(this->MeshHigherDimensionalEntities != 0);
    }
  return;
}

void vtkCMBModelEdgeMeshOperator::PrintSelf(ostream& os, vtkIndent indent)
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
