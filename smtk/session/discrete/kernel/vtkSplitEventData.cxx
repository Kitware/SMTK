//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkSplitEventData.h"

#include "vtkIdList.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkSplitEventData);
vtkCxxSetObjectMacro(vtkSplitEventData, CreatedModelEntityIds, vtkIdList);

vtkSplitEventData::vtkSplitEventData()
{
  this->SourceEntity = nullptr;
  this->CreatedModelEntityIds = nullptr;
}

vtkSplitEventData::~vtkSplitEventData()
{
  // SourceEntity doesn't strictly need to be set to NULL
  // now but that may change in the future
  this->SetSourceEntity(nullptr);
  this->SetCreatedModelEntityIds(nullptr);
}

void vtkSplitEventData::PrintSelf(ostream& os, vtkIndent indent)
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
  os << indent << "CreatedModelEntityIds: ";
  if (this->CreatedModelEntityIds)
  {
    os << this->CreatedModelEntityIds << endl;
  }
  else
  {
    os << "(NULL)\n";
  }
}
