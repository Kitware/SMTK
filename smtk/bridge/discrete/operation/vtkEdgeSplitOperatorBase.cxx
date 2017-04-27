//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkEdgeSplitOperatorBase.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkEdgeSplitOperatorBase);

vtkEdgeSplitOperatorBase::vtkEdgeSplitOperatorBase()
{
  this->EdgeId = 0;
  this->IsEdgeIdSet = 0;
  this->PointId = 0;
  this->IsPointIdSet = 0;
  this->CreatedModelEdgeId = -1;
  this->CreatedModelVertexId = -1;
}

vtkEdgeSplitOperatorBase::~vtkEdgeSplitOperatorBase()
{
}

void vtkEdgeSplitOperatorBase::SetEdgeId(vtkIdType id)
{
  this->IsEdgeIdSet = 1;
  if (id != this->EdgeId)
  {
    this->Modified();
    this->EdgeId = id;
  }
}

void vtkEdgeSplitOperatorBase::SetPointId(vtkIdType id)
{
  this->IsPointIdSet = 1;
  if (id != this->PointId)
  {
    this->Modified();
    this->PointId = id;
  }
}

vtkModelEntity* vtkEdgeSplitOperatorBase::GetModelEntity(vtkDiscreteModel* Model)
{
  if (!Model || !this->GetIsEdgeIdSet())
  {
    return 0;
  }
  return Model->GetModelEntity(vtkModelEdgeType, this->GetEdgeId());
}

bool vtkEdgeSplitOperatorBase::AbleToOperate(vtkDiscreteModel* Model)
{
  if (!Model)
  {
    vtkErrorMacro("Passed in a null model.");
    return 0;
  }
  if (this->GetIsEdgeIdSet() == 0)
  {
    vtkErrorMacro("No entity id specified.");
    return 0;
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
    return 0;
  }
  return 1;
}

void vtkEdgeSplitOperatorBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "CreatedModelEdgeId: " << this->CreatedModelEdgeId << endl;
  os << indent << "CreatedModelVertexId: " << this->CreatedModelVertexId << endl;
  os << indent << "EdgeId: " << this->EdgeId << endl;
  os << indent << "IsEdgeIdSet: " << this->IsEdgeIdSet << endl;
  os << indent << "PointId: " << this->PointId << endl;
  os << indent << "IsPointIdSet: " << this->IsPointIdSet << endl;
}
