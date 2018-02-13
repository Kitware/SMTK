//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelEdgeOperationBase.h"

#include "vtkDiscreteModel.h"
#
#include "vtkDiscreteModelEdge.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkModelEdgeOperationBase);

vtkModelEdgeOperationBase::vtkModelEdgeOperationBase()
{
  this->IsLineResolutionSet = 0;
  this->LineResolution = 1;
}

vtkModelEdgeOperationBase::~vtkModelEdgeOperationBase()
{
}

vtkDiscreteModelEdge* vtkModelEdgeOperationBase::GetModelEdgeEntity(vtkDiscreteModel* Model)
{
  if (!Model)
  {
    return 0;
  }
  return vtkDiscreteModelEdge::SafeDownCast(this->Superclass::GetModelEntity(Model));
}

bool vtkModelEdgeOperationBase::AbleToOperate(vtkDiscreteModel* Model)
{
  if (!Model)
  {
    vtkErrorMacro("Passed in a null model.");
    return 0;
  }

  if (this->GetIsIdSet() == 0)
  {
    vtkErrorMacro("No entity id specified.");
    return 0;
  }
  if (this->GetIsLineResolutionSet() == 0)
  {
    vtkErrorMacro("No line resolution is set.");
    return 0;
  }

  return 1;
}

void vtkModelEdgeOperationBase::SetLineResolution(int resolution)
{
  this->IsLineResolutionSet = 1;
  if (resolution != this->LineResolution)
  {
    this->LineResolution = resolution;
    this->Modified();
  }
}

bool vtkModelEdgeOperationBase::Operate(vtkDiscreteModel* Model)
{
  if (!this->AbleToOperate(Model))
  {
    return 0;
  }

  return this->Superclass::Operate(Model);
}

void vtkModelEdgeOperationBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "IsLineResolutionSet: " << this->IsLineResolutionSet << endl;
  os << indent << "LineResolution: " << this->LineResolution << endl;
}
