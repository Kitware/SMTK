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
  this->GridFileName = nullptr;
  this->ModelInfoFileName = nullptr;
}

vtkModelGridRepresentation::~vtkModelGridRepresentation()
{
  this->SetGridFileName(nullptr);
  this->SetModelInfoFileName(nullptr);
}

void vtkModelGridRepresentation::Reset()
{
  this->SetGridFileName(nullptr);
  this->SetModelInfoFileName(nullptr);
}
bool vtkModelGridRepresentation::IsSameModelInfoFile(const char* filename)
{
  if ((this->ModelInfoFileName == nullptr && filename == nullptr) ||
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
