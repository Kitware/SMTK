/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

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
  this->LowerDimensionalIds->InsertNextTupleValue(&Id);
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
