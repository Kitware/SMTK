//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkMaterialOperation.h"

#include "smtk/session/discrete/kernel/Model/vtkModelEntity.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdList.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkMaterialOperation);

vtkMaterialOperation::vtkMaterialOperation()
{
  this->OperateSucceeded = 0;
  this->BuiltMaterialId = -1;
  this->DestroySucceeded = 0;
}

vtkMaterialOperation::~vtkMaterialOperation() = default;

bool vtkMaterialOperation::AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if (!ModelWrapper)
  {
    vtkErrorMacro("Passed in a null model wrapper.");
    return 0;
  }
  return this->Superclass::AbleToOperate(ModelWrapper->GetModel());
}

void vtkMaterialOperation::Operate(vtkDiscreteModelWrapper* ModelWrapper)
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

void vtkMaterialOperation::Build(vtkDiscreteModelWrapper* ModelWrapper)
{
  this->BuiltMaterialId = this->Superclass::Build(ModelWrapper->GetModel());
}

void vtkMaterialOperation::Destroy(vtkDiscreteModelWrapper* ModelWrapper)
{
  this->DestroySucceeded = this->Superclass::Destroy(ModelWrapper->GetModel());
}

void vtkMaterialOperation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
  os << indent << "BuiltMaterialId: " << this->BuiltMaterialId << endl;
  os << indent << "DestroySucceeded: " << this->DestroySucceeded << endl;
}
