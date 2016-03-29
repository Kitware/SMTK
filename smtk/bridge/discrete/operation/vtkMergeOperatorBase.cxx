//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkMergeOperatorBase.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteModelVertex.h"
#include <vtkIdTypeArray.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro(vtkMergeOperatorBase);
vtkCxxSetObjectMacro(vtkMergeOperatorBase, LowerDimensionalIds, vtkIdTypeArray);

vtkMergeOperatorBase::vtkMergeOperatorBase()
{
  this->LowerDimensionalIds = vtkIdTypeArray::New();
  this->LowerDimensionalIds->SetNumberOfComponents(1);
  this->LowerDimensionalIds->SetNumberOfTuples(0);
  this->TargetId = 0;
  this->IsTargetIdSet = 0;
  this->SourceId = 0;
  this->IsSourceIdSet = 0;
}

vtkMergeOperatorBase::~vtkMergeOperatorBase()
{
  if(this->LowerDimensionalIds)
    {
    LowerDimensionalIds->Delete();
    }
}

void vtkMergeOperatorBase::SetTargetId(vtkIdType targetId)
{
  this->IsTargetIdSet = 1;
  if(targetId != this->TargetId)
    {
    this->Modified();
    this->TargetId = targetId;
    }
}

void vtkMergeOperatorBase::SetSourceId(vtkIdType sourceId)
{
  this->IsSourceIdSet = 1;
  if(sourceId != this->SourceId)
    {
    this->Modified();
    this->SourceId = sourceId;
    }
}

void vtkMergeOperatorBase::AddLowerDimensionalId(vtkIdType Id)
{
  this->LowerDimensionalIds->InsertNextTypedTuple(&Id);
}

void vtkMergeOperatorBase::RemoveAllLowerDimensionalIds()
{
  this->LowerDimensionalIds->SetNumberOfTuples(0);
}

vtkDiscreteModelGeometricEntity* vtkMergeOperatorBase::GetTargetModelEntity(
  vtkDiscreteModel* Model)
{
  if(Model == 0 || this->IsTargetIdSet == 0)
    {
    return 0;
    }
  vtkDiscreteModelFace* Face = vtkDiscreteModelFace::SafeDownCast(
    Model->GetModelEntity(vtkModelFaceType, this->TargetId));
  if(Face)
    {
    return Face;
    }
  vtkDiscreteModelRegion* Region = vtkDiscreteModelRegion::SafeDownCast(
    Model->GetModelEntity(vtkModelRegionType, this->TargetId));
  if(Region)
    {
    return Region;
    }
  vtkDiscreteModelEdge* Edge = vtkDiscreteModelEdge::SafeDownCast(
    Model->GetModelEntity(vtkModelEdgeType, this->TargetId));
  if(Edge)
    {
    return Edge;
    }

  vtkWarningMacro("Problem getting model entity.");
  return 0;
}

vtkDiscreteModelGeometricEntity* vtkMergeOperatorBase::GetSourceModelEntity(
  vtkDiscreteModel* model)
{
  if(model == 0 || this->IsSourceIdSet == 0)
    {
    return 0;
    }
  if(vtkDiscreteModelFace* face = vtkDiscreteModelFace::SafeDownCast(
       model->GetModelEntity(vtkModelFaceType, this->SourceId)))
    {
    return face;
    }
  if(vtkDiscreteModelRegion* region = vtkDiscreteModelRegion::SafeDownCast(
       model->GetModelEntity(vtkModelRegionType, this->SourceId)))
    {
    return region;
    }
  if(vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(
       model->GetModelEntity(vtkModelEdgeType, this->SourceId)))
    {
    return edge;
    }

  vtkWarningMacro("Problem getting model entity.");
  return 0;
}

bool vtkMergeOperatorBase::AbleToOperate(vtkDiscreteModel* model)
{
  if(!model)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }
  if(this->GetIsTargetIdSet() == 0)
    {
    vtkErrorMacro("No target id specified.");
    return 0;
    }
  if(this->GetIsSourceIdSet() == 0)
    {
    vtkErrorMacro("No source id set.");
    return 0;
    }
  vtkModelEntity* targetEntity = model->GetModelEntity(this->TargetId);
  if(vtkDiscreteModelFace::SafeDownCast(targetEntity) == 0 &&
     vtkDiscreteModelRegion::SafeDownCast(targetEntity) == 0 &&
     vtkDiscreteModelEdge::SafeDownCast(targetEntity) == 0 )
    {
    return 0;
    }
  int targetEntityType = targetEntity->GetType();
  vtkModelEntity* sourceEntity = model->GetModelEntity(this->SourceId);
  if(targetEntityType != sourceEntity->GetType())
    {
    return 0;
    }
  // if we are merging model faces for a 3D model we need to make sure that
  // all of them have the same regions/materials on both sides
  if(targetEntityType == vtkModelFaceType)
    {
    vtkModelRegion* sides[2] = {0,0};
    sides[0] = vtkModelFace::SafeDownCast(targetEntity)->GetModelRegion(0);
    sides[1] = vtkModelFace::SafeDownCast(targetEntity)->GetModelRegion(1);
    if(sides[0]!=vtkModelFace::SafeDownCast(sourceEntity)->GetModelRegion(0) &&
       sides[0]!=vtkModelFace::SafeDownCast(sourceEntity)->GetModelRegion(1))
      {
      vtkDebugMacro("Model faces do not share the same regions.");
      return 0;
      }
    if(sides[1]!=vtkModelFace::SafeDownCast(sourceEntity)->GetModelRegion(0) &&
       sides[1]!=vtkModelFace::SafeDownCast(sourceEntity)->GetModelRegion(1))
      {
      vtkDebugMacro("Model faces do not share the same regions.");
      return 0;
      }
    }
  else if(targetEntityType == vtkModelEdgeType)
    {
    if(this->LowerDimensionalIds->GetNumberOfTuples() < 1 ||
       this->LowerDimensionalIds->GetNumberOfTuples() > 2)
      {
      vtkDebugMacro("Wrong number of end nodes inputted for merge.");
      return 0;
      }
    vtkModelEdge* sourceEdge = vtkModelEdge::SafeDownCast(sourceEntity);
    vtkModelEdge* targetEdge = vtkModelEdge::SafeDownCast(targetEntity);
    for(vtkIdType i=0;i<this->LowerDimensionalIds->GetNumberOfTuples();i++)
      {
      vtkModelVertex* sharedVertex = vtkModelVertex::SafeDownCast(
        model->GetModelEntity(vtkModelVertexType, this->LowerDimensionalIds->GetValue(i)));
      if(sharedVertex == 0)
        {
        return 0;
        }
      if(targetEdge->GetAdjacentModelVertex(0) != sharedVertex &&
         targetEdge->GetAdjacentModelVertex(1) != sharedVertex)
        {
        return 0;
        }
      if(sourceEdge->GetAdjacentModelVertex(0) != sharedVertex &&
         sourceEdge->GetAdjacentModelVertex(1) != sharedVertex)
        {
        return 0;
        }
      }
    }

  return 1;
}

bool vtkMergeOperatorBase::Operate(vtkDiscreteModel* model)
{
  vtkDiscreteModelGeometricEntity* targetEntity = this->GetTargetModelEntity(model);

  return targetEntity->Merge(this->GetSourceModelEntity(model), this->LowerDimensionalIds);
}

void vtkMergeOperatorBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "SourceId: " << this->SourceId << endl;
  os << indent << "IsSourceIdSet: " << this->IsSourceIdSet << endl;
  os << indent << "LowerDimensionalIds: " << this->LowerDimensionalIds << endl;
  os << indent << "TargetId: " << this->TargetId << endl;
  os << indent << "IsTargetIdSet: " << this->IsTargetIdSet << endl;
}
