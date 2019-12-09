//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkMergeOperation.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#

vtkStandardNewMacro(vtkMergeOperation);

vtkMergeOperation::vtkMergeOperation()
{
  this->OperateSucceeded = 0;
}

vtkMergeOperation::~vtkMergeOperation() = default;

void vtkMergeOperation::Operate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if (!this->AbleToOperate(ModelWrapper))
  {
    this->OperateSucceeded = 0;
    return;
  }

  this->OperateSucceeded = this->Superclass::Operate(ModelWrapper->GetModel());
  if (this->OperateSucceeded)
  {
    ModelWrapper->Modified();
  }

  return;
}

bool vtkMergeOperation::AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if (!ModelWrapper)
  {
    vtkErrorMacro("Passed in a null model wrapper.");
    return false;
  }
  return this->Superclass::AbleToOperate(ModelWrapper->GetModel());
}

vtkDiscreteModelGeometricEntity* vtkMergeOperation::GetTargetModelEntity(
  vtkDiscreteModelWrapper* ModelWrapper)
{
  return this->Superclass::GetTargetModelEntity(ModelWrapper->GetModel());
}

void vtkMergeOperation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
