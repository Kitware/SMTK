//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkModelEntityGroupOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdList.h"
#include "vtkModelEntity.h"
#include "vtkObjectFactory.h"
#

vtkStandardNewMacro(vtkModelEntityGroupOperator);

vtkModelEntityGroupOperator::vtkModelEntityGroupOperator()
{
  this->OperateSucceeded = 0;
  this->BuiltModelEntityGroupId = -1;
  this->DestroySucceeded = 0;
}

vtkModelEntityGroupOperator::~vtkModelEntityGroupOperator()
{
}

bool vtkModelEntityGroupOperator::AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper)
    {
    vtkErrorMacro("Passed in a null model wrapper.");
    return 0;
    }
  return this->Superclass::AbleToOperate(ModelWrapper->GetModel());
}

void vtkModelEntityGroupOperator::Operate(vtkDiscreteModelWrapper* ModelWrapper)
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

void vtkModelEntityGroupOperator::Build(vtkDiscreteModelWrapper* ModelWrapper)
{
  this->BuiltModelEntityGroupId =
    this->Superclass::Build(ModelWrapper->GetModel());
}

void vtkModelEntityGroupOperator::Destroy(vtkDiscreteModelWrapper* ModelWrapper)
{
  this->DestroySucceeded = this->Superclass::Destroy(ModelWrapper->GetModel());
}

void vtkModelEntityGroupOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
  os << indent << "BuiltModelEntityGroupId: " <<
    this->BuiltModelEntityGroupId << endl;
  os << indent << "DestroySucceeded: " << this->DestroySucceeded << endl;
}
