//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

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

vtkDiscreteModel::vtkDiscreteModel():
  Mesh(),
  MeshClassificationInstance()
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

vtkModelVertex* vtkDiscreteModel::BuildModelVertex(vtkIdType pointId,
  bool bCreateGeometry)
{
  return this->BuildModelVertex(pointId,
    this->GetNextUniquePersistentId(), bCreateGeometry);
}

vtkModelVertex* vtkDiscreteModel::BuildModelVertex(
  vtkIdType pointId, vtkIdType vertexId, bool bCreateGeometry)
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
  if(bCreateGeometry)
    {
    vertex->CreateGeometry();
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
  double point1[3], double point2[3], int /*resolution*/, vtkIdType regionId)
{
  if(this->HasValidMesh())
    {
    DiscreteMesh::EdgePoints dmEdge(point1,point2);
    DiscreteMesh::EdgePointIds edgeIds = this->Mesh.AddEdgePoints(dmEdge);

    vtkModelVertex* vertex1 = this->BuildModelVertex(edgeIds.first);
    vtkModelVertex* vertex2 = this->BuildModelVertex(edgeIds.second);

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

bool vtkDiscreteModel::DestroyModelGeometricEntity(vtkModelGeometricEntity* ge)
{
  vtkDiscreteModelGeometricEntity* geomEntity = dynamic_cast<vtkDiscreteModelGeometricEntity*>(ge);
  if (!geomEntity)
    {
    vtkErrorMacro(<< "Tried to destroy " << ge << " (" << ge->GetClassName()
      << ") which is not of type vtkDiscreteModelGeometricEntity.");
    return false;
    }
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

  std::string baseEntityName = itemType==vtkModelFaceType ? "Face Group" : "Edge Group";
  std::string defaultEntityName;
  this->GetModelEntityDefaultName(vtkDiscreteModelEntityGroupType, baseEntityName.c_str(),
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

void vtkDiscreteModel::SetMesh(DiscreteMesh& m)
{
  this->Mesh = m;
  this->UpdateMesh();
}

void vtkDiscreteModel::UpdateMesh()
{
  this->Mesh.GetBounds(this->ModelBounds);

  this->MeshClassificationInstance.resize(this->Mesh.GetNumberOfEdges(),
                                          ClassificationType::EDGE_DATA);

  this->MeshClassificationInstance.resize(this->Mesh.GetNumberOfFaces(),
                                          ClassificationType::FACE_DATA);

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
    int baseNameLength = static_cast<int>(strlen(baseName));
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

bool vtkDiscreteModel::HasValidMesh() const
{
  return this->Mesh.IsValid();
}

bool vtkDiscreteModel::HasInValidMesh() const
{
  return !this->Mesh.IsValid();
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

  // reset the mesh to an empty mesh
  this->Mesh = DiscreteMesh();
  this->MeshClassificationInstance = vtkDiscreteModel::ClassificationType();

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
