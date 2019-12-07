//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkEdgeSplitOperationBase.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkEdgeSplitOperationBase);

vtkEdgeSplitOperationBase::vtkEdgeSplitOperationBase()
{
  this->EdgeId = 0;
  this->IsEdgeIdSet = 0;
  this->PointId = 0;
  this->IsPointIdSet = 0;
  this->CreatedModelEdgeId = -1;
  this->CreatedModelVertexId = -1;
}

vtkEdgeSplitOperationBase::~vtkEdgeSplitOperationBase() = default;

void vtkEdgeSplitOperationBase::SetEdgeId(vtkIdType id)
{
  this->IsEdgeIdSet = 1;
  if (id != this->EdgeId)
  {
    this->Modified();
    this->EdgeId = id;
  }
}

void vtkEdgeSplitOperationBase::SetPointId(vtkIdType id)
{
  this->IsPointIdSet = 1;
  if (id != this->PointId)
  {
    this->Modified();
    this->PointId = id;
  }
}

vtkModelEntity* vtkEdgeSplitOperationBase::GetModelEntity(vtkDiscreteModel* Model)
{
  if (!Model || !this->GetIsEdgeIdSet())
  {
    return nullptr;
  }
  return Model->GetModelEntity(vtkModelEdgeType, this->GetEdgeId());
}

bool vtkEdgeSplitOperationBase::AbleToOperate(vtkDiscreteModel* Model)
{
  if (!Model)
  {
    vtkErrorMacro("Passed in a null model.");
    return false;
  }
  if (this->GetIsEdgeIdSet() == 0)
  {
    vtkErrorMacro("No entity id specified.");
    return false;
  }
  if (this->IsPointIdSet == 0)
  {
    vtkErrorMacro("PointId is not set.");
  }

  // make sure the object is really a model edge
  vtkDiscreteModelEdge* Edge = vtkDiscreteModelEdge::SafeDownCast(this->GetModelEntity(Model));
  if (!Edge)
  {
    vtkErrorMacro("No model edge found with Id " << this->GetEdgeId());
    return false;
  }
  return true;
}

void vtkEdgeSplitOperationBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "CreatedModelEdgeId: " << this->CreatedModelEdgeId << endl;
  os << indent << "CreatedModelVertexId: " << this->CreatedModelVertexId << endl;
  os << indent << "EdgeId: " << this->EdgeId << endl;
  os << indent << "IsEdgeIdSet: " << this->IsEdgeIdSet << endl;
  os << indent << "PointId: " << this->PointId << endl;
  os << indent << "IsPointIdSet: " << this->IsPointIdSet << endl;
}
