//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModel3dm2DGridRepresentation.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkModelItemIterator.h"
#include "vtkNew.h"
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro(vtkModel3dm2DGridRepresentation);

vtkModel3dm2DGridRepresentation::vtkModel3dm2DGridRepresentation()
{
}

vtkModel3dm2DGridRepresentation::~vtkModel3dm2DGridRepresentation()
{
}

bool vtkModel3dm2DGridRepresentation::GetBCSNodalAnalysisGridPointIds(
  vtkDiscreteModel* model, vtkIdType bcsGroupId,
  int bcGroupType, vtkIdList* pointIds)
{
  pointIds->Reset();
  if(this->IsModelConsistent(model) == false)
    {
    this->Reset();
    return false;
    }
  if(model->HasInValidMesh())
    {  // we're on the client and don't know this info
    return false;
    }

  if(vtkDiscreteModelEntityGroup* bcsNodalGroup =
     vtkDiscreteModelEntityGroup::SafeDownCast(
       model->GetModelEntity(vtkDiscreteModelEntityGroupType, bcsGroupId)))
    {
    vtkModelItemIterator* iterEdge=bcsNodalGroup->NewIterator(vtkModelEdgeType);
    for(iterEdge->Begin();!iterEdge->IsAtEnd();iterEdge->Next())
      {
      vtkDiscreteModelEdge* entity =
        vtkDiscreteModelEdge::SafeDownCast(iterEdge->GetCurrentItem());
      if(entity)
        {
        vtkNew<vtkIdList> newPointIds;
        if(bcGroupType == 1)// vtkSBBCInstance::enBCModelEntityAllNodesType)
          {
          entity->GetAllPointIds(newPointIds.GetPointer());
          }
        else if(bcGroupType == 2)//vtkSBBCInstance::enBCModelEntityBoundaryNodesType)
          {
          entity->GetBoundaryPointIds(newPointIds.GetPointer());
          }
        else if(bcGroupType == 3)//vtkSBBCInstance::enBCModelEntityInteriorNodesType)
          {
          entity->GetInteriorPointIds(newPointIds.GetPointer());
          }
        // Adding the new point ids
        for(vtkIdType i=0; i<newPointIds->GetNumberOfIds(); i++)
          {
          pointIds->InsertUniqueId(newPointIds->GetId(i));
          }
        }
      }
    iterEdge->Delete();
    return true;
    }
  return false;
}

bool vtkModel3dm2DGridRepresentation::GetFloatingEdgeAnalysisGridPointIds(
  vtkDiscreteModel* /*model*/, vtkIdType /*nodalGroupId*/, vtkIdList* /*pointIds*/)
{
  vtkErrorMacro("3dm file does not support floating edges.");
  return false;
}

bool vtkModel3dm2DGridRepresentation::GetModelEdgeAnalysisPoints(
  vtkDiscreteModel* model, vtkIdType edgeId, vtkIdTypeArray* edgePoints)
{
  edgePoints->SetNumberOfComponents(2);
  if(model->HasInValidMesh())
    {  // we're on the client and don't know this info
    return false;
    }
  vtkDiscreteModelEdge* edge =
    vtkDiscreteModelEdge::SafeDownCast(model->GetModelEntity(vtkModelEdgeType, edgeId));
  if(!edge)
    {
    return false;
    }
  vtkPolyData* edgePoly = vtkPolyData::SafeDownCast(edge->GetGeometry());
  vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
  for(vtkIdType i=0;i<edgePoly->GetNumberOfCells();i++)
    {
    edgePoly->GetCellPoints(i, pointIds);
    if(pointIds->GetNumberOfIds() != 2)
      {
      vtkErrorMacro("Bad cell type.");
      return false;
      }
    edgePoints->InsertValue(i*2, pointIds->GetId(0));
    edgePoints->InsertValue(i*2+1, pointIds->GetId(1));
    }
  edgePoints->SetNumberOfTuples(edgePoly->GetNumberOfCells());
  return true;
}

bool vtkModel3dm2DGridRepresentation::GetBoundaryGroupAnalysisFacets(
  vtkDiscreteModel* /*model*/, vtkIdType /*boundaryGroupId*/,
  vtkIdList* /*cellIds*/, vtkIdList* /*cellSides*/)
{
  return false;
}

void vtkModel3dm2DGridRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

