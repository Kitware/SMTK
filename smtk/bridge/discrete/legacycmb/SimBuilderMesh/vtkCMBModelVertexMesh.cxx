//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelVertexMesh.h"

#include <vtkModelVertex.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkCMBModelVertexMesh);

vtkCMBModelVertexMesh::vtkCMBModelVertexMesh()
{
  this->ModelVertex = NULL;
}

vtkCMBModelVertexMesh::~vtkCMBModelVertexMesh()
{
}

vtkModelGeometricEntity* vtkCMBModelVertexMesh::GetModelGeometricEntity()
{
  return this->ModelVertex;
}

void vtkCMBModelVertexMesh::Initialize(vtkCMBMesh* masterMesh, vtkModelVertex* vertex)
{
  if (this->GetMasterMesh() != masterMesh)
  {
    this->SetMasterMesh(masterMesh);
    this->Modified();
  }
  if (this->ModelVertex != vertex)
  {
    this->ModelVertex = vertex;
    this->Modified();
  }
}

bool vtkCMBModelVertexMesh::BuildModelEntityMesh(bool vtkNotUsed(meshHigherDimensionalEntities))
{
  return false;
}

void vtkCMBModelVertexMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  if (this->ModelVertex)
  {
    os << indent << "ModelVertex: " << this->ModelVertex << "\n";
  }
  else
  {
    os << indent << "ModelVertex: (NULL)\n";
  }
}
