//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkModelEntityOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkInformation.h"
#include "vtkModelEntity.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

vtkStandardNewMacro(vtkModelEntityOperator);

vtkModelEntityOperator::vtkModelEntityOperator()
{
  this->OperateSucceeded = 0;
}

vtkModelEntityOperator::~vtkModelEntityOperator()
{
}

vtkModelEntity* vtkModelEntityOperator::GetModelEntity(
  vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper)
    {
    return 0;
    }
  return this->Superclass::GetModelEntity(ModelWrapper->GetModel());
}

bool vtkModelEntityOperator::AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper)
    {
    vtkErrorMacro("Passed in a null model wrapper.");
    return 0;
    }
  return this->Superclass::AbleToOperate(ModelWrapper->GetModel());
}

void vtkModelEntityOperator::Operate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!this->AbleToOperate(ModelWrapper))
    {
    this->OperateSucceeded = 0;
    return;
    }

  this->OperateSucceeded  = this->Superclass::Operate(ModelWrapper->GetModel());
  if(this->OperateSucceeded)
    {
    ModelWrapper->Modified();
    }
}

void vtkModelEntityOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
