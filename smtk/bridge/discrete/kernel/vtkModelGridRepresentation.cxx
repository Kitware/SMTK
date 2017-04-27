//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelGridRepresentation.h"

vtkModelGridRepresentation::vtkModelGridRepresentation()
{
  this->GridFileName = NULL;
  this->ModelInfoFileName = NULL;
}

vtkModelGridRepresentation::~vtkModelGridRepresentation()
{
  this->SetGridFileName(NULL);
  this->SetModelInfoFileName(NULL);
}

void vtkModelGridRepresentation::Reset()
{
  this->SetGridFileName(NULL);
  this->SetModelInfoFileName(NULL);
}
bool vtkModelGridRepresentation::IsSameModelInfoFile(const char* filename)
{
  if ((this->ModelInfoFileName == NULL && filename == NULL) ||
    (this->ModelInfoFileName && filename && !strcmp(this->ModelInfoFileName, filename)))
  {
    return true;
  }
  return false;
}

void vtkModelGridRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "GridFileName: " << this->GridFileName << "\n";
  os << indent << "ModelInfoFileName: " << this->ModelInfoFileName << "\n";
}
