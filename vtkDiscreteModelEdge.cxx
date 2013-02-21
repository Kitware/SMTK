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
#include "vtkDiscreteModelEdge.h"

#include "vtkCell.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkModelEdgeUse.h"
#include "vtkModelFace.h"
#include "vtkModelItemIterator.h"
#include "vtkModelVertexUse.h"
#include "vtkObjectFactory.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkLineSource.h"
#include "vtkInformation.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationObjectBaseKey.h"
#include "vtkNew.h"
#include "vtkProperty.h"
#include "vtkSmartPointer.h"
#include "vtkSplitEventData.h"

vtkInformationKeyMacro(vtkDiscreteModelEdge, LINERESOLUTION, Integer);
vtkInformationKeyMacro(vtkDiscreteModelEdge, LINEADNPOINTSGEOMETRY, ObjectBase);

vtkDiscreteModelEdge* vtkDiscreteModelEdge::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkDiscreteModelEdge");
  if(ret)
    {
    return static_cast<vtkDiscreteModelEdge*>(ret);
    }
  return new vtkDiscreteModelEdge;
}

vtkDiscreteModelEdge::vtkDiscreteModelEdge()
{
  this->GetProperties()->Set(LINERESOLUTION(), 1);
  vtkProperty* displayProp = this->GetDisplayProperty();
  displayProp->SetLineWidth(2.0);
  displayProp->SetPointSize(6.0);
}

vtkDiscreteModelEdge::~vtkDiscreteModelEdge()
{
}

bool vtkDiscreteModelEdge::IsDestroyable()
{
  vtkModelItemIterator* edgeUseIter = this->NewModelEdgeUseIterator();
  for(edgeUseIter->Begin();!edgeUseIter->IsAtEnd();edgeUseIter->Next())
    {
    if(vtkModelEdgeUse::SafeDownCast(edgeUseIter->GetCurrentItem())->GetModelLoopUse())
      {
      edgeUseIter->Delete();
      return false;
      }
    }
  edgeUseIter->Delete();
  return true;
}

bool vtkDiscreteModelEdge::Destroy()
{
  this->GetModel()->InvokeModelGeometricEntityEvent(
    ModelGeometricEntityAboutToDestroy, this);
  vtkModelItemIterator* EdgeUseIter = this->NewModelEdgeUseIterator();
  for(EdgeUseIter->Begin();!EdgeUseIter->IsAtEnd();EdgeUseIter->Next())
    {
    if(!vtkModelEdgeUse::SafeDownCast(EdgeUseIter->GetCurrentItem())->Destroy())
      {
      vtkErrorMacro("Problem destroying edge use of an edge.");
      this->Modified();
      EdgeUseIter->Delete();
      return false;
      }
    }
  EdgeUseIter->Delete();
  this->RemoveAllAssociations(vtkModelEdgeUseType);
  // For the floating edge, we have associations with regions
  this->RemoveAllAssociations(vtkModelRegionType);
  this->RemoveAllAssociations(vtkDiscreteModelEntityGroupType);
  this->RemoveAllAssociations(vtkModelNodalGroupType);
  this->Modified();
  return true;
}

vtkModelEntity* vtkDiscreteModelEdge::GetThisModelEntity()
{
  return this;
}

void vtkDiscreteModelEdge::AddRegionAssociation(vtkIdType regionId)
{
  vtkDiscreteModelRegion* region = vtkDiscreteModelRegion::SafeDownCast(
    this->GetModel()->GetModelEntity(vtkModelRegionType, regionId));
  if(region)
    {
    this->AddAssociation(region);
    this->GetModel()->InvokeModelGeometricEntityEvent(
      ModelGeometricEntityBoundaryModified, region);
    }
}

vtkObject* vtkDiscreteModelEdge::GetGeometry()
{
  if(vtkObject* object = this->Superclass::GetGeometry())
    {
    return object;
    }
  if(this->GetNumberOfAssociations(vtkModelRegionType))
    {
    vtkDiscreteModel* model = vtkDiscreteModel::SafeDownCast(this->GetModel());
    if(model->HasValidMesh())
      { // only construct the representation if it is a floating edge on the server
      this->ConstructRepresentation();
      return this->Superclass::GetGeometry();
      }
    }
  return 0;
}

bool vtkDiscreteModelEdge::ConstructRepresentation()
{
  if(this->GetGeometry())
    {
    return 1;
    }
  vtkModelVertex* vertex1 = this->GetAdjacentModelVertex(0);
  vtkModelVertex* vertex2 = this->GetAdjacentModelVertex(1);
  if(!vertex1 || !vertex2)
    {
    vtkWarningMacro("Cannot construct model edge representation without model vertices.");
    return false;
    }
  vtkSmartPointer<vtkLineSource> lineSource =
    vtkSmartPointer<vtkLineSource>::New();
  double xyz[3];
  if(vertex1->GetPoint(xyz))
    {
    lineSource->SetPoint1(xyz);
    }
  else
    {
    vtkErrorMacro("Model vertex does not have a point set yet.");
    }
  if(vertex2->GetPoint(xyz))
    {
    lineSource->SetPoint2(xyz);
    }
  else
    {
    vtkErrorMacro("Model vertex does not have a point set yet.");
    }
  lineSource->SetResolution(this->GetLineResolution());
  lineSource->Update();

  vtkPolyData* poly = vtkPolyData::New();
  poly->DeepCopy(lineSource->GetOutputDataObject(0));
  this->SetGeometry(poly);

  poly->Delete();
  return true;
}

bool vtkDiscreteModelEdge::Split(
  vtkIdType splitPointId, vtkIdType & createdVertexId,
  vtkIdType & createdEdgeId)
{
  vtkObject* geometry = this->GetGeometry();
  vtkPolyData* poly = vtkPolyData::SafeDownCast(geometry);

  if(poly == 0 || poly->GetNumberOfCells() == 0)
    {
    if(geometry)
      {
      // we are on the server...
      return 0;
      }
    // BoundaryRep has not been set -> return error
    return 0;
    }
  if(splitPointId >= poly->GetNumberOfPoints() || splitPointId < 0)
    {
    vtkErrorMacro("Bad point id for model edge split.");
    return 0;
    }

  // on server, go ahead and perform the split first determine if
  // it is a loop with no starting or ending point because if it
  // is splitting won't create a new model edge, it will just
  // modify the current model edge

  if(this->GetAdjacentModelVertex(0) == 0)
    {
    createdEdgeId = -1;
    createdVertexId = -1;
    if(this->SplitModelEdgeLoop(splitPointId) == false)
      {
      vtkErrorMacro("Unable to split edge loop.");
      return 0;
      }
    vtkModelVertex* newVertex = this->GetAdjacentModelVertex(0);
    createdVertexId = newVertex->GetUniquePersistentId();
    vtkIdList* createdEntityIds = vtkIdList::New();
    createdEntityIds->InsertNextId(createdVertexId);
    vtkSplitEventData* splitEventData = vtkSplitEventData::New();
    splitEventData->SetSourceEntity(this);
    splitEventData->SetCreatedModelEntityIds(createdEntityIds);
    createdEntityIds->Delete();
    this->GetModel()->InvokeModelGeometricEntityEvent(ModelGeometricEntitySplit,
                                                      splitEventData);
    splitEventData->Delete();
    return true;
    }

  vtkIdType modelEdgeStartPoint = vtkDiscreteModelVertex::SafeDownCast(
    this->GetAdjacentModelVertex(0))->GetPointId();
  vtkIdType modelEdgeEndPoint = vtkDiscreteModelVertex::SafeDownCast(
    this->GetAdjacentModelVertex(1))->GetPointId();
  if(splitPointId == modelEdgeStartPoint || splitPointId == modelEdgeEndPoint)
    {
    vtkWarningMacro("Picked an end point for splitting a model edge.");
    return 0;
    }

  poly->BuildLinks();
  // if a model vertex already exists at splitPointId we don't need to split
  vtkSmartPointer<vtkIdList> pointCells =
    vtkSmartPointer<vtkIdList>::New();
  poly->GetPointCells(splitPointId, pointCells);
  if(pointCells->GetNumberOfIds() < 2)
    {
    vtkWarningMacro("Improper splitting point for splitting an edge.");
    poly->DeleteLinks();
    return 0;
    }

  poly->GetPointCells(modelEdgeEndPoint, pointCells);
  if(pointCells->GetNumberOfIds() != 1 &&
     (pointCells->GetNumberOfIds() != 2 && modelEdgeStartPoint == modelEdgeEndPoint))
    {
    vtkWarningMacro("Improper grid.");
    return 0;
    }
  // we do a cell walk from the last point id of the edge polydata
  // to the split point and those are the cells that get assigned to
  // the new model edge
  vtkSmartPointer<vtkIdList> newModelEdgeCells =
    vtkSmartPointer<vtkIdList>::New();
  vtkIdType currentCellId = pointCells->GetId(0);
  newModelEdgeCells->InsertNextId(this->GetMasterCellId(currentCellId));
  vtkIdType currentPointId = modelEdgeEndPoint;
  vtkSmartPointer<vtkIdList> tempIds = vtkSmartPointer<vtkIdList>::New();

  while(currentCellId >= 0)
    {
    vtkIdType nextCellId = -1;
    poly->GetCellPoints(currentCellId, tempIds);
    for(int i=0; i<2 && nextCellId == -1 ;i++)
      {
      if(tempIds->GetId(i) != currentPointId)
        {
        poly->GetPointCells(tempIds->GetId(i), pointCells);
        if(tempIds->GetId(i) == splitPointId)
          {
          nextCellId = -2; // stop condition
          }
        else
          {
          for(int j=0;j<2 && nextCellId == -1;j++)
            {
            if(pointCells->GetId(j) != currentCellId)
              {
              nextCellId = pointCells->GetId(j);
              newModelEdgeCells->InsertNextId(this->GetMasterCellId(nextCellId));
              currentPointId = tempIds->GetId(i);
              }
            }
          if(nextCellId < 0)
            {
            vtkWarningMacro("Could not perform grid walk.");
            return 0;
            }
          }
        }
      }
    currentCellId = nextCellId;
    }
  // we could save this for later use but for now assume we should
  // delete to save memory
  poly->DeleteLinks();
  if(newModelEdgeCells->GetNumberOfIds() >= poly->GetNumberOfCells())
    {
    vtkErrorMacro("Bad list of cells to split off from existing model edge.");
    return 0;
    }
  vtkDiscreteModel* model = vtkDiscreteModel::SafeDownCast(this->GetModel());
  vtkModelVertex* vertex = model->BuildModelVertex(splitPointId);
  createdVertexId = vertex->GetUniquePersistentId();
  bool blockEvent = this->GetModel()->GetBlockModelGeometricEntityEvent();
  this->GetModel()->SetBlockModelGeometricEntityEvent(1);
  vtkDiscreteModelEdge* newEdge = vtkDiscreteModelEdge::SafeDownCast(model->BuildModelEdge(0,0));
  this->GetModel()->SetBlockModelGeometricEntityEvent(blockEvent);
  this->Superclass::SplitModelEdge(vertex, newEdge);
  // now create the poly data for the new edge
  newEdge->AddCellsToGeometry(newModelEdgeCells);
  createdEdgeId = newEdge->GetUniquePersistentId();
  this->GetModel()->InvokeModelGeometricEntityEvent(ModelGeometricEntityCreated, newEdge);
  this->GetModel()->InvokeModelGeometricEntityEvent(
    ModelGeometricEntityBoundaryModified, this);

  vtkIdList* createdEntityIds = vtkIdList::New();
  createdEntityIds->InsertNextId(createdVertexId);
  createdEntityIds->InsertNextId(createdEdgeId);
  vtkSplitEventData* splitEventData = vtkSplitEventData::New();
  splitEventData->SetSourceEntity(this);
  splitEventData->SetCreatedModelEntityIds(createdEntityIds);
  createdEntityIds->Delete();
  this->GetModel()->InvokeModelGeometricEntityEvent(ModelGeometricEntitySplit,
                                                    splitEventData);
  splitEventData->Delete();

  vtkModelItemIterator* faces = newEdge->NewAdjacentModelFaceIterator();
  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
    face->GetModel()->InvokeModelGeometricEntityEvent(
      ModelGeometricEntityBoundaryModified, face);
    }
  faces->Delete();
  vtkDiscreteModelVertex* ModelVertex = vtkDiscreteModelVertex::SafeDownCast(vertex);
  if(ModelVertex)
    {
    ModelVertex->CreateGeometry();
    }
  return 1;
}

bool vtkDiscreteModelEdge::SplitModelEdgeLoop(vtkIdType pointId)
{
  vtkDiscreteModel* model = vtkDiscreteModel::SafeDownCast(this->GetModel());
  vtkModelVertex* vertex = model->BuildModelVertex(pointId);
  bool result = this->Superclass::SplitModelEdgeLoop(vertex);
  if(result)
    {
    vtkDiscreteModelVertex* ModelVertex = vtkDiscreteModelVertex::SafeDownCast(vertex);
    if(ModelVertex)
      {
      ModelVertex->CreateGeometry();
      }
    }
  return result;
}

void vtkDiscreteModelEdge::SetLineResolution(int lineResolution)
{
  if(lineResolution > 0 && lineResolution != this->GetLineResolution())
    {
    this->GetProperties()->Set(LINERESOLUTION(), lineResolution);
    // this->UpdateGeometry();
    this->Modified();
    }
}

int vtkDiscreteModelEdge::GetLineResolution()
{
  return this->GetProperties()->Get(LINERESOLUTION());
}

void vtkDiscreteModelEdge::SetLineAndPointsGeometry(vtkObject* geometry)
{
  this->GetProperties()->Set(LINEADNPOINTSGEOMETRY(), geometry);
  this->Modified();
}

vtkObject* vtkDiscreteModelEdge::GetLineAndPointsGeometry()
{
  vtkObject* object = vtkObject::SafeDownCast(
    this->GetProperties()->Get(LINEADNPOINTSGEOMETRY()));
  return object;
}

void vtkDiscreteModelEdge::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

vtkModelRegion* vtkDiscreteModelEdge::GetModelRegion()
{
  vtkModelItemIterator* iter =
    this->NewIterator(vtkModelRegionType);
  iter->Begin();
  vtkModelRegion* region =
    vtkModelRegion::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return region;
}
void vtkDiscreteModelEdge::GetAllPointIds(vtkIdList* ptsList)
{
  ptsList->Reset();
  vtkPolyData* edgePoly = vtkPolyData::SafeDownCast(this->GetGeometry());
  if(edgePoly == NULL)
    {  // we're on the client and don't know this info
    return;
    }
  //ptsList->SetNumberOfIds(edgePoly->GetNumberOfPoints());
  vtkNew<vtkIdList> cellPtsIds;
  for(vtkIdType i=0;i<edgePoly->GetNumberOfCells();i++)
    {
    edgePoly->GetCellPoints(i, cellPtsIds.GetPointer());
    if(cellPtsIds->GetNumberOfIds() != 2)
      {
      vtkErrorMacro("Bad cell type.");
      return;
      }
    ptsList->InsertUniqueId(cellPtsIds->GetId(0));
    ptsList->InsertUniqueId(cellPtsIds->GetId(1));
    }
  //ptsList->Squeeze();
}
void vtkDiscreteModelEdge::GetInteriorPointIds(vtkIdList* ptsList)
{
  vtkNew<vtkIdList> boundaryPtsList;
  this->GetAllPointIds(ptsList);
  this->GetBoundaryPointIds(boundaryPtsList.GetPointer());
  for(vtkIdType i=0;i<boundaryPtsList->GetNumberOfIds();i++)
    {
    ptsList->DeleteId(boundaryPtsList->GetId(i));
    }
}
void vtkDiscreteModelEdge::GetBoundaryPointIds(vtkIdList* ptsList)
{
  ptsList->Reset();
  vtkPolyData* edgePoly = vtkPolyData::SafeDownCast(this->GetGeometry());
  if(edgePoly == NULL)
    {  // we're on the client and don't know this info
    return;
    }
  for(int i=0; i<2; i++)
    {
    vtkDiscreteModelVertex* modelVertex =
      vtkDiscreteModelVertex::SafeDownCast(this->GetAdjacentModelVertex(i));
    if(modelVertex)
      {
      ptsList->InsertUniqueId(modelVertex->GetPointId());
      }
    }
}

void vtkDiscreteModelEdge::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
