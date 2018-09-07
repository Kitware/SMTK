//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkMergeEventData.h"

#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkMergeEventData);
vtkCxxSetObjectMacro(vtkMergeEventData, LowerDimensionalIds, vtkIdTypeArray);

vtkMergeEventData::vtkMergeEventData()
{
  this->SourceEntity = NULL;
  this->TargetEntity = NULL;
  this->LowerDimensionalIds = NULL;
}

vtkMergeEventData::~vtkMergeEventData()
{
  // SourceEntity and TargetEntity don't strictly need to be set to NULL
  // now but that may change in the future
  this->SetSourceEntity(0);
  this->SetTargetEntity(0);
  this->SetLowerDimensionalIds(0);
}

void vtkMergeEventData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "SourceEntity: ";
  if (this->SourceEntity)
  {
    os << this->SourceEntity << endl;
  }
  else
  {
    os << "(NULL)\n";
  }
  os << indent << "TargetEntity: ";
  if (this->TargetEntity)
  {
    os << this->TargetEntity << endl;
  }
  else
  {
    os << "(NULL)\n";
  }
  os << indent << "LowerDimensionalIds: ";
  if (this->LowerDimensionalIds)
  {
    os << this->LowerDimensionalIds << endl;
  }
  else
  {
    os << "(NULL)\n";
  }
}
