//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelEntityMesh.h"

#include <vtkPolyData.h>

vtkCxxSetObjectMacro(vtkCMBModelEntityMesh, ModelEntityMesh, vtkPolyData);

vtkCMBModelEntityMesh::vtkCMBModelEntityMesh()
{
  this->Visible = true;
  this->MasterMesh = NULL;
  this->ModelEntityMesh = NULL;
  this->Length = 0;
  this->MeshedLength = 0;
}

vtkCMBModelEntityMesh::~vtkCMBModelEntityMesh()
{
  this->SetModelEntityMesh(NULL);
}

void vtkCMBModelEntityMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Visible: " << this->Visible << "\n";
  os << indent << "Length: " << this->Length << "\n";
  os << indent << "MeshedLength: " << this->MeshedLength << "\n";
  if(this->MasterMesh)
    {
    os << indent << "MasterMesh: " << this->MasterMesh << "\n";
    }
  else
    {
    os << indent << "MasterMesh: (NULL)\n";
    }
  if(this->ModelEntityMesh)
    {
    os << indent << "ModelEntityMesh: " << this->ModelEntityMesh << "\n";
    }
  else
    {
    os << indent << "ModelEntityMesh: (NULL)\n";
    }
}
