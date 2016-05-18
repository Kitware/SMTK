//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

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
  poly->GetPointCells(modelEdgeEndPoint, pointCells);
  if(pointCells->GetNumberOfIds() != 1 &&
     (pointCells->GetNumberOfIds() != 2 && modelEdgeStartPoint == modelEdgeEndPoint))
    {
    vtkWarningMacro("Improper grid.");
    return 0;
    }
  poly->GetPointCells(splitPointId, pointCells);
  if(pointCells->GetNumberOfIds() < 2)
    {
    vtkWarningMacro("Improper splitting point for splitting an edge.");
    poly->DeleteLinks();
    return 0;
    }

  // we do a cell walk from the split point to the end the edge polydata
  // and those are the cells that get assigned to the new model edge
  vtkSmartPointer<vtkIdList> newModelEdgeCells =
    vtkSmartPointer<vtkIdList>::New(); 
  vtkSmartPointer<vtkIdList> currentPtIds = vtkSmartPointer<vtkIdList>::New();

  /*
  * Before we start walking the edges to go from the split 
  * point to the end point we first need to determine the
  * cell to start at. 
  * 
  *    p1 ----c1---- p2 ----- c2 ----- p3
  *
  * If we consider the split point is p2 and the end point is p3,
  * we need to determine if we should start walking at c1 or c2. 
  * Do do this we look at all the cells that use p2, and select
  * the cell that uses p2 as the first point id (this does presume
  * all cells are ordered head to tail ). 
  * 
  */

  vtkIdType currentCellId = -1;
  for(int i=0; i < pointCells->GetNumberOfIds(); ++i)
    {
    poly->GetCellPoints(pointCells->GetId(i), currentPtIds);
    if( splitPointId == currentPtIds->GetId(0) )
      {
      currentCellId = pointCells->GetId(i);
      break;
      }
    }

  if(currentCellId < 0)
    {
    vtkWarningMacro("Could not find a proper line cell to start splitting edge.");
    poly->DeleteLinks();
    return false;
    }

  /*
  * Now start the walk from the cell that uses the split
  * point as its first point Id, intill we find a cell
  * that uses the end point 
  *
  * The logic is:
  * Start at a given cell
  * Find the points of said cell. ( label start )
  * Find the first point used by the cell that isn't our
  *   marked point ( currentPointId )
  * For this point find the first cell that isn't our self
  * Loop back to label start
  * 
  */

  newModelEdgeCells->InsertNextId(currentCellId);
  vtkIdType currentPointId = splitPointId;
  while(currentCellId >= 0)
    {
    vtkIdType nextCellId = -1;
    poly->GetCellPoints(currentCellId, currentPtIds);

    for(int i=0; i<2 && nextCellId == -1 ;i++)
      {
      if(currentPtIds->GetId(i) != currentPointId)
        {
        poly->GetPointCells(currentPtIds->GetId(i), pointCells);
        if(currentPtIds->GetId(i) == modelEdgeEndPoint)
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
              newModelEdgeCells->InsertNextId(nextCellId);
              currentPointId = currentPtIds->GetId(i);
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

  //Now we convert from local id space to global id space
    {
    const vtkIdType size = newModelEdgeCells->GetNumberOfIds();
    for(vtkIdType s=0; s < size; ++s)
      {
      const vtkIdType mc = this->GetMasterCellId(newModelEdgeCells->GetId(s));
      newModelEdgeCells->SetId(s, mc);
      }
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

bool vtkDiscreteModelEdge::AddCellsToGeometry(vtkIdList* masterCellIds)
{
  const vtkIdType numCells = masterCellIds->GetNumberOfIds();
  vtkIdType* cellids = masterCellIds->GetPointer(0);

  //only transform the ids if they are already positive
  //otherwise we are doing something like a split and we already have
  //have ids in edge index space
  bool idsModified = false;
  if(numCells > 0 && cellids[0] >= 0)
    {
    DiscreteMesh::FlatIdSpaceToEdgeIdSpace(cellids,numCells);
    idsModified = true;
    }

  bool retVal = vtkDiscreteModelGeometricEntity::AddCellsToGeometry(masterCellIds);

  if(idsModified)
    { // transform the ids back
    DiscreteMesh::EdgeIdSpaceToFlatIdSpace(cellids,numCells);
    }
  return retVal;
}

bool vtkDiscreteModelEdge::AddCellsClassificationToMesh(vtkIdList* cellids)
{
  // now add cells on this entity
  vtkModelGeometricEntity* thisEntity =
    vtkModelGeometricEntity::SafeDownCast(this->GetThisModelEntity());
  vtkDiscreteModel* model = vtkDiscreteModel::SafeDownCast(
                                thisEntity->GetModel());

  vtkPolyData* entityPoly = vtkPolyData::SafeDownCast(
                                                    thisEntity->GetGeometry());


  const DiscreteMesh& mesh = model->GetMesh();
  vtkDiscreteModel::ClassificationType& classification =
                                            model->GetMeshClassification();

  vtkNew<vtkIdList> pointIds;
  for(vtkIdType i=0;i<cellids->GetNumberOfIds();i++)
    {
    const vtkIdType masterCellId = cellids->GetId(i);
    const vtkIdType cellType = mesh.GetCellType(masterCellId);
    mesh.GetCellPointIds(masterCellId,pointIds.GetPointer());

    const vtkIdType newLocalCellId =
              entityPoly->InsertNextCell(cellType,pointIds.GetPointer());
    this->GetReverseClassificationArray()->InsertNextTypedTuple(&masterCellId);

    // update the classification on the model to this info
    classification.SetEntity(masterCellId, newLocalCellId, this);
    }
  if(cellids->GetNumberOfIds())
    {
    entityPoly->Modified();
    }

  return true;
}

bool vtkDiscreteModelEdge::IsEdgeCellPoint(vtkIdType pointId)
{
  vtkPolyData* edgePoly = vtkPolyData::SafeDownCast(this->GetGeometry());
  if(edgePoly == NULL)
    {  // we're on the client and don't know this info
    return false;
    }
  vtkSmartPointer<vtkIdList> pointCells =
    vtkSmartPointer<vtkIdList>::New();
  edgePoly->GetPointCells(pointId, pointCells);
  return (pointCells->GetNumberOfIds() != 0);
}

void vtkDiscreteModelEdge::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
