//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkDiscreteModelGeometricEntity.h"

#include "vtkModelMaterial.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkMergeEventData.h"
#include "vtkModelEdgeUse.h"
#include "vtkModelFaceUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelLoopUse.h"
#include "vtkModelShellUse.h"
#include "vtkModelVertex.h"
#include "vtkModelVertexUse.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include <map>

namespace
{
  const char ReverseClassificationArrayName[] = "ReverseClassification";
}

vtkDiscreteModelGeometricEntity::vtkDiscreteModelGeometricEntity()
{
}

vtkDiscreteModelGeometricEntity::~vtkDiscreteModelGeometricEntity()
{
}

vtkModelMaterial* vtkDiscreteModelGeometricEntity::GetMaterial()
{
  vtkModelItemIterator* iter = this->GetThisModelEntity()->NewIterator(vtkModelMaterialType);
  iter->Begin();
  vtkModelMaterial* material = 0;
  if(!iter->IsAtEnd())
    {
    material = vtkModelMaterial::SafeDownCast(iter->GetCurrentItem());
    }
  iter->Delete();
  return material;
}

vtkIdType vtkDiscreteModelGeometricEntity::GetMasterCellId(vtkIdType id)
{
  vtkModelGeometricEntity* thisEntity =
    vtkModelGeometricEntity::SafeDownCast(this->GetThisModelEntity());
  vtkPolyData* geometry = vtkPolyData::SafeDownCast(
    thisEntity->GetGeometry());
  if(!geometry || geometry->GetNumberOfCells() <= id)
    {
    return -1;
    }
  vtkIdTypeArray* masterCellIds = vtkIdTypeArray::SafeDownCast(
    geometry->GetCellData()->GetArray(ReverseClassificationArrayName));
  return masterCellIds->GetValue(id);
}

vtkIdType vtkDiscreteModelGeometricEntity::GetNumberOfCells()
{
  vtkModelGeometricEntity* thisEntity =
    vtkModelGeometricEntity::SafeDownCast(this->GetThisModelEntity());
  vtkPolyData* geometry = vtkPolyData::SafeDownCast(
    thisEntity->GetGeometry());
  if(!geometry)
    {
    return -1;
    }
  return geometry->GetNumberOfCells();
}

vtkDiscreteModelGeometricEntity*
vtkDiscreteModelGeometricEntity::GetThisDiscreteModelGeometricEntity(vtkModelEntity* entity)
{
  if(!entity)
    {
    return 0;
    }
  if(vtkDiscreteModelFace* face = vtkDiscreteModelFace::SafeDownCast(entity))
    {
    return face;
    }
  else if(vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(entity))
    {
    return edge;
    }
  return vtkDiscreteModelRegion::SafeDownCast(entity);
}

bool vtkDiscreteModelGeometricEntity::Merge(
  vtkDiscreteModelGeometricEntity* sourceEntity, vtkIdTypeArray* lowerDimensionalIds)
{
  vtkModelGeometricEntity* thisEntity =
    vtkModelGeometricEntity::SafeDownCast(this->GetThisModelEntity());
  vtkDiscreteModel* model = vtkDiscreteModel::SafeDownCast(thisEntity->GetModel());
  vtkMergeEventData* mergeEventData = vtkMergeEventData::New();
  mergeEventData->SetSourceEntity(sourceEntity);
  mergeEventData->SetTargetEntity(this);
  mergeEventData->SetLowerDimensionalIds(lowerDimensionalIds);
  model->InvokeModelGeometricEntityEvent(ModelGeometricEntitiesAboutToMerge,
                                         mergeEventData);
  mergeEventData->Delete();

  // check to see if we're on the server or client
  if(model->HasValidMesh())
    { // for the server side only
    vtkIdList* cells = vtkIdList::New();
    vtkIdTypeArray* masterGeometryCellIndex = sourceEntity->GetReverseClassificationArray();
    vtkIdType numCells = masterGeometryCellIndex->GetNumberOfTuples();
    cells->SetNumberOfIds(numCells);
    for(vtkIdType i=0;i<numCells;i++)
      {
      cells->InsertId(i, masterGeometryCellIndex->GetValue(i));
      }
    this->AddCellsToGeometry(cells);
    cells->Delete();
    }

  vtkModelGeometricEntity* sourceGeometricEntity =
    vtkModelGeometricEntity::SafeDownCast(sourceEntity->GetThisModelEntity());

  // need to correct model topology info still
  if(vtkDiscreteModelFace* face =
     vtkDiscreteModelFace::SafeDownCast(sourceGeometricEntity))
    {
    for(int i=0;i<2;i++)
      {
      vtkModelFaceUse* sourceFaceUse = face->GetModelFaceUse(i);
      vtkModelShellUse* sourceShellUse = sourceFaceUse->GetModelShellUse();
      if(sourceShellUse)
        {
        sourceShellUse->RemoveModelFaceUse(sourceFaceUse);
        }
      }
    if(face->GetNumberOfModelEdges())
      {
      // for models where faces are not adjacent to regions:
      // tricky part is to figure out which target face's loop use
      // the edge use should belong to when there are more than 1 loop uses
      vtkGenericWarningMacro("Problem merging.");
      throw 1;
      }
    }
  else if(vtkDiscreteModelRegion* region =
          vtkDiscreteModelRegion::SafeDownCast(sourceGeometricEntity))
    {
    // all lower dimensional model entities that were adjacent to this source
    // are now adjacent to the target
    vtkModelItemIterator* shellUses = region->NewModelShellUseIterator();
    for(shellUses->Begin();!shellUses->IsAtEnd();shellUses->Next())
      {
      vtkModelShellUse* shellUse = vtkModelShellUse::SafeDownCast(shellUses->GetCurrentItem());
      vtkModelItemIterator* faceUses = shellUse->NewModelFaceUseIterator();
      for(faceUses->Begin();!faceUses->IsAtEnd();faceUses->Next())
        {
        // tricky part is to figure out which target region's shell use
        // the face use should belong to when there are more than 1 shell uses
        vtkGenericWarningMacro("Problem merging.");
        throw 1;
        }
      faceUses->Delete();
      }
    shellUses->Delete();
    }
  else if(vtkDiscreteModelEdge* sourceEdge =
          vtkDiscreteModelEdge::SafeDownCast(sourceGeometricEntity))
    {
    if(lowerDimensionalIds->GetNumberOfTuples() == 0)
      {
      vtkGenericWarningMacro("No end node specified when merging model edges.");
      return 0;
      }
    vtkModelVertex* sharedVertex = vtkModelVertex::SafeDownCast(
      model->GetModelEntity(vtkModelVertexType, lowerDimensionalIds->GetValue(0)));
    int targetSharedVertexNumber = -1;
    vtkDiscreteModelEdge* targetEdge = vtkDiscreteModelEdge::SafeDownCast(this->GetThisModelEntity());
    if(targetEdge->GetAdjacentModelVertex(0) == sharedVertex)
      {
      targetSharedVertexNumber = 0;
      }
    else if(targetEdge->GetAdjacentModelVertex(1) == sharedVertex)
      {
      targetSharedVertexNumber = 1;
      }
    else
      {
      vtkGenericWarningMacro("End node is not part of arc.");
      return 0;
      }
    int sourceSharedVertexNumber = -1;
    if(sourceEdge->GetAdjacentModelVertex(0) == sharedVertex)
      {
      sourceSharedVertexNumber = 0;
      }
    else if(sourceEdge->GetAdjacentModelVertex(1) == sharedVertex)
      {
      sourceSharedVertexNumber = 1;
      }
    else
      {
      vtkGenericWarningMacro("End node is not part of arc.");
      return 0;
      }

    //get rid of sourceEdge adjacencies
    vtkModelItemIterator* sourceEdgeUses = sourceEdge->NewIterator(vtkModelEdgeUseType);
    for(sourceEdgeUses->Begin();!sourceEdgeUses->IsAtEnd();sourceEdgeUses->Next())
      {
      vtkModelEdgeUse* sourceEdgeUse = vtkModelEdgeUse::SafeDownCast(sourceEdgeUses->GetCurrentItem());
      int actualSourceVertexUseNumber = (1+sourceEdgeUse->GetDirection() + sourceSharedVertexNumber)%2;
      vtkModelLoopUse* loopUse = sourceEdgeUse->GetModelLoopUse();
      int sourceEdgeUseIndex = loopUse->GetModelEdgeUseIndex(sourceEdgeUse);
      int tempIndex = (sourceEdgeUseIndex+1)%loopUse->GetNumberOfModelEdgeUses();
      if(loopUse->GetModelEdgeUse(tempIndex)->GetModelEdge() != targetEdge)
        {
        tempIndex = (sourceEdgeUseIndex+loopUse->GetNumberOfModelEdgeUses()-1)
          % loopUse->GetNumberOfModelEdgeUses();
        }
      vtkModelEdgeUse* targetEdgeUse = loopUse->GetModelEdgeUse(tempIndex);
      int actualTargetVertexUseNumber = (1+targetEdgeUse->GetDirection() - targetSharedVertexNumber)%2;
      /*vtkModelVertexUse* targetVertexUse =*/ targetEdgeUse->GetModelVertexUse(
        actualTargetVertexUseNumber);
      /*vtkModelVertexUse* sourceVertexUse =*/ sourceEdgeUse->GetModelVertexUse(
        actualSourceVertexUseNumber);
      if(actualTargetVertexUseNumber == 0)
        {
        targetEdgeUse->SetModelVertexUses(sourceEdgeUse->GetModelVertexUse(1-actualSourceVertexUseNumber),
                                          targetEdgeUse->GetModelVertexUse(1));
        }
      else
        {
        targetEdgeUse->SetModelVertexUses(targetEdgeUse->GetModelVertexUse(0),
                                          sourceEdgeUse->GetModelVertexUse(1-actualSourceVertexUseNumber));
        }

      loopUse->RemoveModelEdgeUseAssociation(sourceEdgeUse);
      sourceEdgeUse->Destroy();
      }
    sourceEdgeUses->Delete();
    }
  if(!model->Superclass::DestroyModelGeometricEntity(sourceGeometricEntity))
    {
    vtkGenericWarningMacro("Problem destroying entity.");
    }
  for(vtkIdType i=0;i<lowerDimensionalIds->GetNumberOfTuples() *
        lowerDimensionalIds->GetNumberOfComponents();i++)
    {
    vtkModelGeometricEntity* entity = vtkModelGeometricEntity::SafeDownCast(
      model->GetModelEntity(lowerDimensionalIds->GetValue(i)));
    if(!model->Superclass::DestroyModelGeometricEntity(entity))
      {
      vtkGenericWarningMacro("Problem destroying entity.");
      }
    }

  if(vtkDiscreteModelFace* face =
     vtkDiscreteModelFace::SafeDownCast(thisEntity))
    {
    for(int i=0;i<2;i++)
      {
      if(vtkModelRegion* region = face->GetModelRegion(i))
        {
        region->GetModel()->InvokeModelGeometricEntityEvent(
          ModelGeometricEntityBoundaryModified, region);
        }
      }
    }
  else if(vtkDiscreteModelEdge* sourceEdge =
          vtkDiscreteModelEdge::SafeDownCast(thisEntity))
    {
    vtkModelItemIterator* faces = sourceEdge->NewAdjacentModelFaceIterator();
    for(faces->Begin();!faces->IsAtEnd();faces->Next())
      {
      vtkModelFace* faceTmp = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
      faceTmp->GetModel()->InvokeModelGeometricEntityEvent(
        ModelGeometricEntityBoundaryModified, faceTmp);
      }
    faces->Delete();
    }

  return 1;
}

bool vtkDiscreteModelGeometricEntity::AddCellsToGeometry(vtkIdList* masterCellIds)
{
  vtkModelGeometricEntity* thisEntity =
    vtkModelGeometricEntity::SafeDownCast(this->GetThisModelEntity());
  vtkDiscreteModel* model = vtkDiscreteModel::SafeDownCast(
    thisEntity->GetModel());
  vtkObject* geometry = thisEntity->GetGeometry();
  vtkPolyData* entityPoly = vtkPolyData::SafeDownCast(geometry);
  if(model->HasInValidMesh())
    {
    // we are on the client
    return 1;
    }
  if(entityPoly == 0)
    {
    if(geometry)
      {
      cerr << "vtkDiscreteModelGeometricEntity: Bad geometry.\n";
      return 0;
      }
    entityPoly= vtkPolyData::New();
    vtkModelGeometricEntity::SafeDownCast(
      this->GetThisModelEntity())->SetGeometry(entityPoly);

    entityPoly->Allocate(masterCellIds->GetNumberOfIds());
    vtkIdTypeArray* reverseClassificationArray = vtkIdTypeArray::New();
    reverseClassificationArray->SetNumberOfComponents(1);
    reverseClassificationArray->SetNumberOfTuples(0);
    reverseClassificationArray->SetName(ReverseClassificationArrayName);;
    entityPoly->GetCellData()->AddArray(reverseClassificationArray);
    reverseClassificationArray->Delete();

    //extract the points from the master polydata and use them for this
    //geometeric entity
    entityPoly->SetPoints(model->GetMesh().SharePointsPtr());
    entityPoly->Delete();
    }

  // first remove the cells from other model faces
  std::map<vtkDiscreteModelGeometricEntity*, vtkSmartPointer<vtkIdList> > removeInfo;
  vtkDiscreteModel::ClassificationType& classification =
                                            model->GetMeshClassification();
  for(vtkIdType i=0;i<masterCellIds->GetNumberOfIds();i++)
    {
    vtkDiscreteModelGeometricEntity* sourceEntity =
        classification.GetEntity(masterCellIds->GetId(i));

    std::map<vtkDiscreteModelGeometricEntity*, vtkSmartPointer<vtkIdList> >::iterator it=
      removeInfo.find(sourceEntity);
    if(it != removeInfo.end() && it->first == this)
      { // it shouldn't get here but just to be safe
      continue;
      }
    else if(it == removeInfo.end())
      {
      vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
      idList->InsertNextId(classification.GetEntityIndex(masterCellIds->GetId(i)));
      removeInfo[sourceEntity] = idList;
      }
    else
      {
      vtkIdType localId = classification.GetEntityIndex(masterCellIds->GetId(i));
      it->second->InsertNextId(localId);
      }
    }
  for(std::map<vtkDiscreteModelGeometricEntity*, vtkSmartPointer<vtkIdList> >::iterator it=
        removeInfo.begin();it!=removeInfo.end();it++)
    {
    if(it->first)
      {
      it->first->RemoveCellsFromGeometry(it->second);
      }
    }

  return this->AddCellsClassificationToMesh(masterCellIds);
}

bool vtkDiscreteModelGeometricEntity::AddCellsClassificationToMesh(vtkIdList* cellids)
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

  return 1;
}

bool vtkDiscreteModelGeometricEntity::RemoveCellsFromGeometry(vtkIdList* cellIds)
{
  if(cellIds->GetNumberOfIds() == 0)
    {
    return true;
    }
  vtkModelGeometricEntity* thisEntity =
    vtkModelGeometricEntity::SafeDownCast(this->GetThisModelEntity());
  vtkObject* geometry = thisEntity->GetGeometry();
  vtkPolyData* poly = vtkPolyData::SafeDownCast(geometry);
  if(poly == 0)
    {
    return 1;
    }

  vtkIdType numberOfOriginalCells = poly->GetNumberOfCells();
  // on server, go ahead and remove the cells
  for(vtkIdType i=0;i<cellIds->GetNumberOfIds();i++)
    {
    vtkIdType cellId = cellIds->GetId(i);
    if(cellId < numberOfOriginalCells && cellId >= 0)
      {
      poly->DeleteCell(cellId);
      }
    else
      {
      cerr << "vtkDiscreteModelGeometricEntity: Bad cell index to remove.\n";
      }
    }
  poly->RemoveDeletedCells();
  poly->Modified();

  // now we need to update the mapping from the master grid cells to the
  // local grid cell id
  vtkDiscreteModel* model = vtkDiscreteModel::SafeDownCast(thisEntity->GetModel());
  vtkDiscreteModel::ClassificationType& classified = model->GetMeshClassification();
  for(vtkIdType i=0;i<poly->GetNumberOfCells();i++)
    {
    vtkIdType masterCellId = this->GetMasterCellId(i);
    classified.SetEntity(masterCellId, i, this);
    }

  return 1;
}

const char* vtkDiscreteModelGeometricEntity::GetReverseClassificationArrayName()
{
  return ReverseClassificationArrayName;
}

vtkIdTypeArray* vtkDiscreteModelGeometricEntity::GetReverseClassificationArray()
{
  vtkModelGeometricEntity* thisEntity =
    vtkModelGeometricEntity::SafeDownCast(this->GetThisModelEntity());
  vtkObject* geometry = thisEntity->GetGeometry();
  vtkPolyData* poly = vtkPolyData::SafeDownCast(geometry);
  if(poly == 0)
    {
    return 0;
    }

  return vtkIdTypeArray::SafeDownCast(poly->GetCellData()->GetArray(
                                        ReverseClassificationArrayName));
}

