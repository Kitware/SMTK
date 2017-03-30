//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkEdgeSplitOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdTypeArray.h"
#include "vtkModelUserName.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkEdgeSplitOperator);

vtkEdgeSplitOperator::vtkEdgeSplitOperator()
{
  this->OperateSucceeded = 0;
}

vtkEdgeSplitOperator::~vtkEdgeSplitOperator()
{
}

vtkModelEntity* vtkEdgeSplitOperator::GetModelEntity(vtkDiscreteModelWrapper* ModelWrapper)
{
  if (!ModelWrapper || !this->GetIsEdgeIdSet())
  {
    return 0;
  }
  return this->Superclass::GetModelEntity(ModelWrapper->GetModel());
}

bool vtkEdgeSplitOperator::AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if (!ModelWrapper)
  {
    vtkErrorMacro("Passed in a null model.");
    return 0;
  }
  return this->Superclass::AbleToOperate(ModelWrapper->GetModel());
}

void vtkEdgeSplitOperator::Operate(vtkDiscreteModelWrapper* ModelWrapper)
{
  vtkDebugMacro("Operating on a model.");

  if (!this->AbleToOperate(ModelWrapper))
  {
    this->OperateSucceeded = 0;
    return;
  }

  vtkDiscreteModelEdge* Edge =
    vtkDiscreteModelEdge::SafeDownCast(this->GetModelEntity(ModelWrapper));

  vtkIdType newEdgeId = -1, newVertexId;
  this->OperateSucceeded = Edge->Split(this->GetPointId(), newVertexId, newEdgeId);
  if (this->OperateSucceeded)
  {
    this->SetCreatedModelEdgeId(newEdgeId);
    this->SetCreatedModelVertexId(newVertexId);
    std::set<vtkIdType> newEnts;
    newEnts.insert(newEdgeId);
    newEnts.insert(newVertexId);
    ModelWrapper->AddGeometricEntities(newEnts);
  }
  vtkDebugMacro("Finished operating on a model.");
  return;
}

void vtkEdgeSplitOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
