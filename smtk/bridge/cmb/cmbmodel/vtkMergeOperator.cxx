//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkMergeOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#

vtkStandardNewMacro(vtkMergeOperator);

vtkMergeOperator::vtkMergeOperator()
{
  this->OperateSucceeded = 0;
}

vtkMergeOperator::~vtkMergeOperator()
{
}

void vtkMergeOperator::Operate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!this->AbleToOperate(ModelWrapper))
    {
    this->OperateSucceeded = 0;
    return;
    }

  this->OperateSucceeded = this->Superclass::Operate(ModelWrapper->GetModel());
  if(this->OperateSucceeded)
    {
    ModelWrapper->Modified();
    }

  return;
}

bool vtkMergeOperator::AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper)
    {
    vtkErrorMacro("Passed in a null model wrapper.");
    return 0;
    }
  return this->Superclass::AbleToOperate(ModelWrapper->GetModel());
}

vtkDiscreteModelGeometricEntity* vtkMergeOperator::GetTargetModelEntity(
  vtkDiscreteModelWrapper* ModelWrapper)
{
  return this->Superclass::GetTargetModelEntity(ModelWrapper->GetModel());
}

void vtkMergeOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
