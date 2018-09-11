//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBMeshWrapper.h"

#include "vtkCMBMeshServer.h"
#include <vtkDiscreteModel.h>
#include <vtkDiscreteModelWrapper.h>
#include <vtkObjectFactory.h>

#include <sstream>

vtkStandardNewMacro(vtkCMBMeshWrapper);

vtkCMBMeshWrapper::vtkCMBMeshWrapper()
{
  this->Mesh = NULL;
}

vtkCMBMeshWrapper::~vtkCMBMeshWrapper()
{
  if (this->Mesh)
  {
    this->Mesh->Delete();
    this->Mesh = NULL;
  }
}

vtkCMBMeshServer* vtkCMBMeshWrapper::GetMesh()
{
  if (!this->Mesh)
  {
    this->Mesh = vtkCMBMeshServer::New();
  }
  return this->Mesh;
}

void vtkCMBMeshWrapper::SetModelWrapper(vtkDiscreteModelWrapper* modelWrapper)
{
  if (modelWrapper == NULL)
  {
    vtkErrorMacro("ModelWrapper is NULL");
    return;
  }
  this->GetMesh()->Initialize(modelWrapper->GetModel());
}

void vtkCMBMeshWrapper::SetGlobalLength(double length)
{
  this->GetMesh()->SetGlobalLength(length);
  this->Modified();
}

void vtkCMBMeshWrapper::SetGlobalMinimumAngle(double angle)
{
  this->GetMesh()->SetGlobalMinimumAngle(angle);
  this->Modified();
}

void vtkCMBMeshWrapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  if (this->Mesh)
  {
    os << indent << "Mesh: " << this->Mesh << endl;
  }
  else
  {
    os << indent << "Mesh: (NULL)\n";
  }
}
