//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelStateOperationBase.h"

#include "vtkDiscreteModel.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"

vtkStandardNewMacro(vtkCMBModelStateOperationBase);

vtkCMBModelStateOperationBase::vtkCMBModelStateOperationBase()
{
  this->SerializedModelString = vtkStringArray::New();
}

vtkCMBModelStateOperationBase::~vtkCMBModelStateOperationBase()
{
  this->SerializedModelString->Delete();
  this->SerializedModelString = NULL;
}

bool vtkCMBModelStateOperationBase::AbleToOperate(vtkDiscreteModel* Model)
{
  if (!Model)
  {
    vtkErrorMacro("Passed in a null model.");
    return 0;
  }
  return 1;
}
void vtkCMBModelStateOperationBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "SerializedModelString: " << this->SerializedModelString << "\n";
}
