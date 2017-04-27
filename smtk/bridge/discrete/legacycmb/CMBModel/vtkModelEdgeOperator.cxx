//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelEdgeOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#
#include "vtkDiscreteModelEdge.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkModelEdgeOperator);

vtkModelEdgeOperator::vtkModelEdgeOperator()
{
}

vtkModelEdgeOperator::~vtkModelEdgeOperator()
{
}

bool vtkModelEdgeOperator::AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if (!ModelWrapper)
  {
    vtkErrorMacro("Passed in a null model wrapper.");
    return 0;
  }

  return this->Superclass::AbleToOperate(ModelWrapper->GetModel());
}

void vtkModelEdgeOperator::Operate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if (!this->AbleToOperate(ModelWrapper))
  {
    this->OperateSucceeded = 0;
    return;
  }

  vtkDiscreteModelEdge* Entity = this->GetModelEdgeEntity(ModelWrapper->GetModel());
  if (!Entity)
  {
    vtkErrorMacro("Could not find edge entity with id " << this->GetId());
    return;
  }
  Entity->SetLineResolution(this->GetLineResolution());

  this->OperateSucceeded = this->Superclass::Operate(ModelWrapper->GetModel());
  if (this->OperateSucceeded)
  {
    ModelWrapper->Modified();
  }
}

void vtkModelEdgeOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
