/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed, or modified, in any form or by any means, without
permission in writing from Kitware Inc.

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
#include "vtkDiscreteModel.h"

#include "vtkCell.h"
#include "vtkCharArray.h"
#include "vtkModelGridRepresentation.h"
#include "vtkModelMaterial.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkModelUniqueNodalGroup.h"
#include "vtkModelUserName.h"
#include "vtkConnectivityFilter.h"
#include "vtkDataObject.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationObjectBaseKey.h"
#include "vtkModelFaceUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelShellUse.h"
#include "vtkModelVertexUse.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkObjectFactory.h"
#include "vtkSerializer.h"

#include <algorithm>

vtkCxxSetObjectMacro(vtkDiscreteModel, AnalysisGridInfo, vtkModelGridRepresentation);
vtkStandardNewMacro(vtkDiscreteModel);
vtkInformationKeyMacro(vtkDiscreteModel, POINTMAPARRAY, ObjectBase);
vtkInformationKeyMacro(vtkDiscreteModel, CELLMAPARRAY, ObjectBase);

vtkDiscreteModel::vtkDiscreteModel()
{
  // initialize bounds to be invalid
  this->ModelBounds[0] = this->ModelBounds[2] = this->ModelBounds[4] = 1;
  this->ModelBounds[1] = this->ModelBounds[3] = this->ModelBounds[5] = -1;
  this->AnalysisGridInfo = NULL;
  this->BlockEvent = false;
}

vtkDiscreteModel::~vtkDiscreteModel()
{
  this->SetAnalysisGridInfo(NULL);
}

vtkModelVertex* vtkDiscreteModel::BuildModelVertex(vtkIdType pointId)
{
  return this->BuildModelVertex(pointId, this->GetNextUniquePersistentId());
}

vtkModelVertex* vtkDiscreteModel::BuildModelVertex(vtkIdType pointId, vtkIdType vertexId)
{
  vtkDiscreteModelVertex* vertex = vtkDiscreteModelVertex::New();
  vertex->SetUniquePersistentId(vertexId);
  this->AddAssociation(vertex);
  vertex->Delete();
  vertex->SetPointId(pointId);
  if(vertexId > this->GetLargestUsedUniqueId())
    {
    this->SetLargestUsedUniqueId(vertexId);
    }
  this->InvokeModelGeometricEntityEvent(ModelGeometricEntityCreated, vertex);
  return vertex;
}

vtkModelEdge* vtkDiscreteModel::BuildModelEdge(vtkModelVertex* vertex0,
                                          vtkModelVertex* vertex1)
{
  return this->BuildModelEdge(
    vertex0, vertex1, this->GetNextUniquePersistentId());
}

vtkModelEdge* vtkDiscreteModel::BuildModelEdge(
  vtkModelVertex* vertex0, vtkModelVertex* vertex1,
  vtkIdType edgeId)
{
  if(edgeId > this->GetLargestUsedUniqueId())
    {
    this->SetLargestUsedUniqueId(edgeId);
    }
  vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::New();
  edge->Initialize(vertex0, vertex1, edgeId);

  this->AddAssociation(edge);
  edge->Delete();
  std::string defaultEntityName;
  this->GetModelEntityDefaultName(vtkModelEdgeType, "Edge",
                                  defaultEntityName);
  vtkModelUserName::SetUserName(edge, defaultEntityName.c_str());

  this->InvokeModelGeometricEntityEvent(ModelGeometricEntityCreated, edge);

  return edge;
}

vtkModelEdge* vtkDiscreteModel::BuildFloatingRegionEdge(vtkIdType edgeId,
  double point1[3], double point2[3], int resolution, vtkIdType regionId)
{
  if(vtkPolyData* masterPoly =
     vtkPolyData::SafeDownCast(this->GetGeometry()))
    {
    vtkIdType pointId = masterPoly->GetPoints()->InsertNextPoint(point1);
    vtkModelVertex* vertex1 = this->BuildModelVertex(pointId);
    pointId = masterPoly->GetPoints()->InsertNextPoint(point2);
    vtkModelVertex* vertex2 = this->BuildModelVertex(pointId);
    vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::New();
    edge->Initialize(vertex1, vertex2, edgeId);
    this->AddAssociation(edge);
    edge->AddRegionAssociation(regionId);
    edge->Delete();

    std::string defaultEntityName;
    this->GetModelEntityDefaultName(vtkModelEdgeType, "Line",
                                    defaultEntityName);
    vtkModelUserName::SetUserName(edge, defaultEntityName.c_str());
    this->InvokeModelGeometricEntityEvent(ModelGeometricEntityCreated, edge);
    return edge;
    }
  vtkErrorMacro("Problem creating floating edge.");
  return NULL;
}

vtkModelFace* vtkDiscreteModel::BuildModelFace(
  int numEdges, vtkModelEdge** edges, int* edgeDirections, vtkModelMaterial* material)
{
  vtkModelFace* face = this->BuildModelFace(numEdges, edges, edgeDirections);
  material->AddModelGeometricEntity(vtkDiscreteModelFace::SafeDownCast(face));
  return face;
}

vtkModelFace* vtkDiscreteModel::BuildModelFace(int numEdges, vtkModelEdge** edges,
                                          int* edgeDirections)
{
  return this->BuildModelFace(
    numEdges, edges, edgeDirections, this->GetNextUniquePersistentId());
}

vtkModelFace* vtkDiscreteModel::BuildModelFace(int numEdges, vtkModelEdge** edges,
                                          int* edgeDirections, vtkIdType modelFaceId)
{
  vtkDiscreteModelFace* face = vtkDiscreteModelFace::New();
  this->AddAssociation(face);
  face->Initialize(numEdges, edges, edgeDirections, modelFaceId);

  if(modelFaceId > this->GetLargestUsedUniqueId())
    {
    this->SetLargestUsedUniqueId(modelFaceId);
    }
  face->Delete();

  std::string defaultEntityName;
  this->GetModelEntityDefaultName(vtkModelFaceType, "Face",
                                  defaultEntityName);
  vtkModelUserName::SetUserName(face, defaultEntityName.c_str());
  this->InvokeModelGeometricEntityEvent(ModelGeometricEntityCreated, face);
  return face;
}

vtkModelRegion* vtkDiscreteModel::BuildModelRegion()
{
  return this->BuildModelRegion(this->GetNextUniquePersistentId());
}

vtkModelRegion* vtkDiscreteModel::BuildModelRegion(vtkIdType modelRegionId)
{
  vtkDiscreteModelRegion* region = vtkDiscreteModelRegion::New();
  region->Initialize(modelRegionId);
  this->AddAssociation(region);

  if(modelRegionId > this->GetLargestUsedUniqueId())
    {
    this->SetLargestUsedUniqueId(modelRegionId);
    }
  region->Delete();

  std::string defaultEntityName;
  this->GetModelEntityDefaultName(vtkModelRegionType, "Region",
                                  defaultEntityName);
  vtkModelUserName::SetUserName(region, defaultEntityName.c_str());
  this->InvokeModelGeometricEntityEvent(ModelGeometricEntityCreated, region);

  return region;
}

vtkModelRegion* vtkDiscreteModel::BuildModelRegion(
  int numFaces, vtkModelFace** faces, int* faceSides)
{
  return this->BuildModelRegion(numFaces, faces, faceSides,
                                this->GetNextUniquePersistentId());
}

vtkModelRegion* vtkDiscreteModel::BuildModelRegion(
  int numFaces, vtkModelFace** faces, int* faceSides, vtkIdType modelRegionId)
{
  vtkDiscreteModelRegion* region = vtkDiscreteModelRegion::New();
  region->Initialize(numFaces, faces, faceSides,
                     modelRegionId);
  this->AddAssociation(region);

  if(modelRegionId > this->GetLargestUsedUniqueId())
    {
    this->SetLargestUsedUniqueId(modelRegionId);
    }
  region->Delete();

  std::string defaultEntityName;
  this->GetModelEntityDefaultName(vtkModelRegionType, "Region",
                                  defaultEntityName);
  vtkModelUserName::SetUserName(region, defaultEntityName.c_str());
  this->InvokeModelGeometricEntityEvent(ModelGeometricEntityCreated, region);

  return region;
}

vtkModelRegion* vtkDiscreteModel::BuildModelRegion(
  int numFaces, vtkModelFace** faces, int* faceSides,
  vtkModelMaterial* material)
{
  return this->BuildModelRegion(
    numFaces, faces, faceSides,
    this->GetNextUniquePersistentId(), material);
}

vtkModelRegion* vtkDiscreteModel::BuildModelRegion(
  int numFaces, vtkModelFace** faces, int* faceSides,
  vtkIdType modelRegionId, vtkModelMaterial* material)
{
  vtkDiscreteModelRegion* region = vtkDiscreteModelRegion::SafeDownCast(
    this->BuildModelRegion(numFaces, faces, faceSides, modelRegionId));
  if(material)
    {
    material->AddModelGeometricEntity(region);
    }
  return region;
}

bool vtkDiscreteModel::DestroyModelGeometricEntity(vtkDiscreteModelGeometricEntity* geomEntity)
{
  vtkModelMaterial* material = geomEntity->GetMaterial();
  if(material)
    {
    vtkModelGeometricEntity* vtkGeomEntity =
      vtkModelGeometricEntity::SafeDownCast(geomEntity->GetThisModelEntity());
    material->RemoveModelGeometricEntity(vtkGeomEntity);
    }
  vtkModelItemIterator* iter = geomEntity->NewModelEntityGroupIterator();
  while(geomEntity->GetNumberOfModelEntityGroups())
    {
    iter->Begin();
    vtkDiscreteModelEntityGroup::SafeDownCast(iter->GetCurrentItem())->RemoveModelEntity(geomEntity);
    }
  iter->Delete();

  vtkModelGeometricEntity* vtkGeomEntity =
    vtkModelGeometricEntity::SafeDownCast(geomEntity->GetThisModelEntity());
  this->Modified();
  return this->Superclass::DestroyModelGeometricEntity(vtkGeomEntity);
}

void vtkDiscreteModel::GetBounds(double bounds[6])
{
  memcpy(bounds, this->ModelBounds, sizeof(double) * 6);
}

vtkModelMaterial* vtkDiscreteModel::BuildMaterial()
{
  return this->BuildMaterial(this->GetNextUniquePersistentId());
}

vtkModelMaterial* vtkDiscreteModel::BuildMaterial(vtkIdType id)
{
  vtkModelMaterial* material = vtkModelMaterial::New();
  material->Initialize(id);
  this->AddAssociation(material);
  this->SetLargestUsedUniqueId(std::max(this->GetLargestUsedUniqueId(), id));
  material->Delete();

  std::string defaultEntityName;
  this->GetModelEntityDefaultName(vtkModelMaterialType, "DomainSet",
                                  defaultEntityName);
  vtkModelUserName::SetUserName(material, defaultEntityName.c_str());
  vtkIdType entityId = material->GetUniquePersistentId();
  this->InternalInvokeEvent(DomainSetCreated, &entityId);

  return material;
}

bool vtkDiscreteModel::DestroyMaterial(vtkModelMaterial* material)
{
  if(!material->IsDestroyable())
    {
    return false;
    }
  vtkIdType entityId = material->GetUniquePersistentId();
  this->InternalInvokeEvent(DomainSetAboutToDestroy, &entityId);
  if(!material->Destroy())
    {
    vtkErrorMacro("Problem destroying material.");
    return false;
    }
  this->RemoveAssociation(material);
  this->InternalInvokeEvent(DomainSetDestroyed, &entityId);

  this->Modified();
  return true;
}

vtkDiscreteModelEntityGroup* vtkDiscreteModel::BuildModelEntityGroup(
  int itemType, int numEntities, vtkDiscreteModelEntity** entities)
{
  return this->BuildModelEntityGroup(
    itemType, numEntities, entities, this->GetNextUniquePersistentId());
}

vtkDiscreteModelEntityGroup* vtkDiscreteModel::BuildModelEntityGroup(
  int itemType, int numEntities, vtkDiscreteModelEntity** entities,
  vtkIdType id)
{
  vtkDiscreteModelEntityGroup* entityGroup = vtkDiscreteModelEntityGroup::New();
  entityGroup->Initialize(id);
  entityGroup->SetEntityType(itemType);
  for(int i=0;i<numEntities;i++)
    {
    entityGroup->AddModelEntity(entities[i]);
    }
  this->AddAssociation(entityGroup);
  entityGroup->Delete();
  this->SetLargestUsedUniqueId(std::max(this->GetLargestUsedUniqueId(), id));

  std::string defaultEntityName;
  this->GetModelEntityDefaultName(vtkDiscreteModelEntityGroupType, "BCS Group",
                                  defaultEntityName);
  vtkModelUserName::SetUserName(entityGroup, defaultEntityName.c_str());
  vtkIdType entityId = entityGroup->GetUniquePersistentId();
  this->InternalInvokeEvent(ModelEntityGroupCreated, &entityId);

  return entityGroup;
}

bool vtkDiscreteModel::DestroyModelEntityGroup(vtkDiscreteModelEntityGroup* entityGroup)
{
  if(!entityGroup->IsDestroyable())
    {
    return 0;
    }
  vtkIdType entityId = entityGroup->GetUniquePersistentId();
  this->InternalInvokeEvent(ModelEntityGroupAboutToDestroy, &entityId);

  if(!entityGroup->Destroy())
    {
    vtkErrorMacro("Problem destroying entity group.");
    return 0;
    }
  this->RemoveAssociation(entityGroup);
  this->InternalInvokeEvent(ModelEntityGroupDestroyed, &entityId);

  this->Modified();
  return 1;
}

vtkModelNodalGroup* vtkDiscreteModel::BuildNodalGroup(int type, vtkIdList* pointIds)
{
  return this->BuildNodalGroup(type, pointIds, this->GetNextUniquePersistentId());
}

vtkModelNodalGroup* vtkDiscreteModel::BuildNodalGroup(int type, vtkIdList* pointIds,
                                               vtkIdType id)
{
  vtkModelNodalGroup* nodalGroup = 0;
  if(type == BASE_NODAL_GROUP)
    {
    nodalGroup = vtkModelNodalGroup::New();
    }
  else if(type == UNIQUE_NODAL_GROUP)
    {
    nodalGroup = vtkModelUniqueNodalGroup::New();
    }
  else
    {
    vtkErrorMacro("Bad Type.");
    return 0;
    }
  if(pointIds && pointIds->GetNumberOfIds())
    {
    nodalGroup->AddPointIds(pointIds);
    }
  nodalGroup->Initialize(id);

  this->AddAssociation(nodalGroup);
  nodalGroup->Delete();

  std::string defaultEntityName;
  this->GetModelEntityDefaultName(vtkModelNodalGroupType, "Nodal Group",
                                  defaultEntityName);
  vtkModelUserName::SetUserName(nodalGroup, defaultEntityName.c_str());
  vtkIdType entityId = nodalGroup->GetUniquePersistentId();
  this->InternalInvokeEvent(NodalGroupCreated, &entityId);

  return nodalGroup;
}

bool vtkDiscreteModel::DestroyNodalGroup(vtkModelNodalGroup* nodalGroup)
{
  if(!nodalGroup->IsDestroyable())
    {
    return 0;
    }
  vtkIdType entityId = nodalGroup->GetUniquePersistentId();
  this->InternalInvokeEvent(NodalGroupAboutToDestroy, &entityId);
  if(!nodalGroup->Destroy())
    {
    vtkErrorMacro("Problem destroying entity group.");
    return 0;
    }
  this->RemoveAssociation(nodalGroup);
  this->InternalInvokeEvent(NodalGroupDestroyed, &entityId);

  this->Modified();
  return 1;
}

bool vtkDiscreteModel::DestroyModelEdge(vtkDiscreteModelEdge* modelEdge)
{
  if(!modelEdge->IsDestroyable())
    {
    return 0;
    }
  this->Modified();
  if(!modelEdge->Destroy())
    {
    vtkErrorMacro("Problem destroying entity group.");
    return 0;
    }
  this->RemoveAssociation(modelEdge);

  return 1;
}

vtkDiscreteModelGeometricEntity* vtkDiscreteModel::GetCellModelGeometricEntity(
  vtkIdType cellId)
{
  size_t size = static_cast<size_t>(cellId);
  if(cellId < 0 || size >= this->CellClassification.size())
    {
    vtkWarningMacro("Bad CellId.");
    return 0;
    }
  return this->CellClassification[cellId];
}

void vtkDiscreteModel::SetGeometry(vtkObject* geometry)
{
  // may want to make a deep copy of geometry
  // for now assume that we don't need to
  this->GetProperties()->Set(GEOMETRY(), geometry);
  vtkPolyData* poly = vtkPolyData::SafeDownCast(geometry);
  if(poly)
    {
    poly->GetBounds(this->ModelBounds);
    // we're on the server so set the vector sizes for classification
    vtkIdType numCells = poly->GetNumberOfCells();
    vtkDiscreteModelGeometricEntity* geomEnt = 0;
    this->CellClassification.resize(numCells, geomEnt);
    this->ClassifiedCellIndex.resize(numCells, -1);
    vtkModelUniqueNodalGroup* nodalGroup = 0;
    this->UniquePointGroup.resize(poly->GetNumberOfPoints(), nodalGroup);
    }
  this->Modified();
}
void vtkDiscreteModel::UpdateGeometry()
{
  vtkPolyData* poly = vtkPolyData::SafeDownCast(
    this->GetGeometry());
  if(poly)
    {
    poly->GetBounds(this->ModelBounds);
    // we're on the server so set the vector sizes for classification
    vtkIdType numCells = poly->GetNumberOfCells();
    vtkDiscreteModelGeometricEntity* geomEnt = 0;
    this->CellClassification.resize(numCells, geomEnt);
    this->ClassifiedCellIndex.resize(numCells, -1);
    vtkModelUniqueNodalGroup* nodalGroup = 0;
    this->UniquePointGroup.resize(poly->GetNumberOfPoints(), nodalGroup);
    }
  this->Modified();
}

void vtkDiscreteModel::GetModelEntityDefaultName(int entityType, const char* baseName,
                                            std::string & defaultEntityName)
{
  // Default names are BaseName + LocalEntityNumber (e.g. "Material 8").
  int largestDefaultNameNumber = 0;
  vtkModelItemIterator* iter = this->NewIterator(entityType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    int baseNameLength = strlen(baseName);
    vtkModelEntity* entity = vtkModelEntity::SafeDownCast(
      iter->GetCurrentItem());
    if(!entity || vtkModelUserName::GetUserName(entity) == 0)
      {
      continue;
      }
    const char* userName = vtkModelUserName::GetUserName(entity);
    if(!strncmp(baseName, userName, baseNameLength))
      {
      int number = atoi(userName+baseNameLength);
      if(number > largestDefaultNameNumber)
        {
        largestDefaultNameNumber = number;
        }

      }
    }
  char name[200];
  sprintf(name, "%s%d", baseName, largestDefaultNameNumber+1);
  defaultEntityName = name;

  iter->Delete();
}

vtkObject* vtkDiscreteModel::GetGeometry()
{
  vtkObject* object = vtkObject::SafeDownCast(
    this->GetProperties()->Get(GEOMETRY()));
  return object;
}

const char* vtkDiscreteModel::GetPointMapArrayName()
{
  return "ModelPointMapArray";
}

const char* vtkDiscreteModel::GetCellMapArrayName()
{
  return "ModelCellMapArray";
}

const char* vtkDiscreteModel::GetCanonicalSideArrayName()
{
  return "ModelCanonicalSideArray";
}

vtkIdType vtkDiscreteModel::GetCellModelGeometricEntityIndex(vtkIdType cellId)
{
  size_t size = static_cast<size_t>(cellId);
  if(cellId < 0 || size >= this->ClassifiedCellIndex.size())
    {
    vtkWarningMacro("Bad cell id.");
    return 0;
    }
  return this->ClassifiedCellIndex[cellId];
}

void vtkDiscreteModel::SetCellClassification(
  vtkIdType masterCellId, vtkIdType geomEntityCellId, vtkDiscreteModelGeometricEntity* entity)
{
  size_t numCells = this->ClassifiedCellIndex.size();
  if(masterCellId >= 0 && static_cast<size_t>(masterCellId) < numCells)
    {
    this->ClassifiedCellIndex[masterCellId] = geomEntityCellId;
    this->CellClassification[masterCellId] = entity;
    }
  else
    {
    vtkErrorMacro("Bad master cell id value.");
    }
}

void vtkDiscreteModel::SetPointUniqueNodalGroup(
  vtkModelUniqueNodalGroup* nodalGroup, vtkIdType pointId)
{
  size_t numPoints = this->UniquePointGroup.size();
  if(pointId < 0 || static_cast<size_t>(pointId) >= numPoints)
    {
    vtkErrorMacro("Bad master point id value.");
    }
  else
    {
    if(this->UniquePointGroup[pointId] &&
       this->UniquePointGroup[pointId] != nodalGroup)
      {
      this->UniquePointGroup[pointId]->vtkModelUniqueNodalGroup::Superclass::RemovePointId(pointId);
      }
    this->UniquePointGroup[pointId] = nodalGroup;
    }
}

vtkModelUniqueNodalGroup* vtkDiscreteModel::GetPointUniqueNodalGroup(vtkIdType pointId)
{
  size_t numPoints = this->UniquePointGroup.size();
  if(pointId < 0 || static_cast<size_t>(pointId) >= numPoints)
    {
    vtkErrorMacro("Bad master point id value.");
    return 0;
    }
  return this->UniquePointGroup[pointId];
}

void vtkDiscreteModel::Reset()
{
 // Destroy entity groups
  // model entity groups
  vtkModelItemIterator* entityGroupIter = this->NewIterator(vtkDiscreteModelEntityGroupType);
  for(entityGroupIter->Begin();!entityGroupIter->IsAtEnd();entityGroupIter->Next())
    {
    bool destroyed =
      vtkDiscreteModelEntityGroup::SafeDownCast(entityGroupIter->GetCurrentItem())->Destroy();
    if(!destroyed)
      {
      vtkErrorMacro("Problem destroying an entity group.");
      }
    }
  entityGroupIter->Delete();
  this->RemoveAllAssociations(vtkDiscreteModelEntityGroupType);
  // nodal groups
  vtkModelItemIterator* nodalGroupIter = this->NewIterator(vtkModelNodalGroupType);
  for(nodalGroupIter->Begin();!nodalGroupIter->IsAtEnd();nodalGroupIter->Next())
    {
    bool destroyed =
      vtkModelNodalGroup::SafeDownCast(nodalGroupIter->GetCurrentItem())->Destroy();
    if(!destroyed)
      {
      vtkErrorMacro("Problem destroying a nodal group.");
      }
    }
  nodalGroupIter->Delete();
  this->RemoveAllAssociations(vtkModelNodalGroupType);
  this->UniquePointGroup.clear();

  // Destroy materials
  vtkModelItemIterator* materialIter = this->NewIterator(vtkModelMaterialType);
  for(materialIter->Begin();!materialIter->IsAtEnd();materialIter->Next())
    {
    bool destroyed =
      vtkModelMaterial::SafeDownCast(materialIter->GetCurrentItem())->Destroy();
    if(!destroyed)
      {
      vtkErrorMacro("Problem destroying a material.");
      }
    }
  materialIter->Delete();
  this->RemoveAllAssociations(vtkModelMaterialType);

  this->Superclass::Reset();

  // and finally the polydata representation and classification
  this->GetProperties()->Remove(GEOMETRY());
  this->CellClassification.clear();
  this->ClassifiedCellIndex.clear();
  this->InternalInvokeEvent(ModelReset, this);
}

void vtkDiscreteModel::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
  if (ser->IsWriting())
    {
    double *bounds = &this->ModelBounds[0];
    unsigned int length = 6;
    ser->Serialize("ModelBounds", bounds, length);
    }
  else
    {
    double *bounds = 0;
    unsigned int length = 0;
    ser->Serialize("ModelBounds", bounds, length);
    if (length > 0)
      {
      memcpy(this->ModelBounds, bounds, sizeof(double) * 6);
      delete [] bounds;
      }
    }
}
void vtkDiscreteModel::InternalInvokeEvent(unsigned long theevent, void *callData)
{
  if(!this->BlockEvent)
    {
    this->InvokeEvent(theevent, callData);
    }
}

void vtkDiscreteModel::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "BlockEvent: " << this->BlockEvent << "\n";
}
