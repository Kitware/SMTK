//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelStateOperatorBase.h"

#include "vtkDiscreteModel.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"

vtkStandardNewMacro(vtkCMBModelStateOperatorBase);

//-----------------------------------------------------------------------------
vtkCMBModelStateOperatorBase::vtkCMBModelStateOperatorBase()
{
  this->SerializedModelString = vtkStringArray::New();
}

//-----------------------------------------------------------------------------
vtkCMBModelStateOperatorBase::~vtkCMBModelStateOperatorBase()
{
  this->SerializedModelString->Delete();
  this->SerializedModelString = NULL;
}

//-----------------------------------------------------------------------------
bool vtkCMBModelStateOperatorBase::AbleToOperate(vtkDiscreteModel* Model)
{
  if(!Model)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }
  return 1;
}
//-----------------------------------------------------------------------------
void vtkCMBModelStateOperatorBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
    os << indent << "SerializedModelString: " << this->SerializedModelString << "\n";
}
