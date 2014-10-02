//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkModelEdgeOperatorBase.h"

#include "vtkDiscreteModel.h"
#
#include "vtkObjectFactory.h"
#include "vtkDiscreteModelEdge.h"

vtkStandardNewMacro(vtkModelEdgeOperatorBase);

vtkModelEdgeOperatorBase::vtkModelEdgeOperatorBase()
{
  this->IsLineResolutionSet = 0;
  this->LineResolution = 1;
}

vtkModelEdgeOperatorBase::~vtkModelEdgeOperatorBase()
{
}

vtkDiscreteModelEdge* vtkModelEdgeOperatorBase::GetModelEdgeEntity(
  vtkDiscreteModel* Model)
{
  if(!Model)
    {
    return 0;
    }
  return vtkDiscreteModelEdge::SafeDownCast(
    this->Superclass::GetModelEntity(Model));
}

bool vtkModelEdgeOperatorBase::AbleToOperate(vtkDiscreteModel* Model)
{
  if(!Model)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }

  if(this->GetIsIdSet() == 0)
    {
    vtkErrorMacro("No entity id specified.");
    return 0;
    }
  if(this->GetIsLineResolutionSet() == 0)
    {
    vtkErrorMacro("No line resolution is set.");
    return 0;
    }

  return 1;
}

void vtkModelEdgeOperatorBase::SetLineResolution(int resolution)
{
  this->IsLineResolutionSet = 1;
  if(resolution != this->LineResolution)
    {
    this->LineResolution = resolution;
    this->Modified();
    }
}

bool vtkModelEdgeOperatorBase::Operate(vtkDiscreteModel* Model)
{
  if(!this->AbleToOperate(Model))
    {
    return 0;
    }

  return this->Superclass::Operate(Model);
}

void vtkModelEdgeOperatorBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "IsLineResolutionSet: " << this->IsLineResolutionSet << endl;
  os << indent << "LineResolution: " << this->LineResolution << endl;
}
