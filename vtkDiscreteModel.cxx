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

vtkCxxRevisionMacro(vtkDiscreteModel, "$Revision: 2586 $");
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

vtkModelVertex* vtkDiscreteModel::BuildModelVertex(vtkIdType PointId)
{
  return this->BuildModelVertex(PointId, this->GetNextUniquePersistentId());
}

vtkModelVertex* vtkDiscreteModel::BuildModelVertex(vtkIdType PointId, vtkIdType vertexId)
{
  vtkDiscreteModelVertex* vertex = vtkDiscreteModelVertex::New();
  vertex->SetUniquePersistentId(vertexId);
  this->AddAssociation(vertex->GetType(), vertex);
  vertex->Delete();
  vertex->SetPointId(PointId);
  if(vertexId > this->GetLargestUsedUniqueId())
    {
    this->SetLargestUsedUniqueId(vertexId);
    }
  this->InvokeModelGeometricEntityEvent(ModelGeometricEntityCreated, vertex);
  return vertex;
}

vtkModelEdge* vtkDiscreteModel::BuildModelEdge(vtkModelVertex* Vertex0,
                                          vtkModelVertex* vertex1)
{
  return this->BuildModelEdge(
    Vertex0, vertex1, this->GetNextUniquePersistentId());
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

  this->AddAssociation(edge->GetType(), edge);
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
    this->AddAssociation(edge->GetType(), edge);
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
  int NumEdges, vtkModelEdge** Edges, int* EdgeDirections, vtkModelMaterial* Material)
{
  vtkModelFace* Face = this->BuildModelFace(NumEdges, Edges, EdgeDirections);
  Material->AddModelGeometricEntity(vtkDiscreteModelFace::SafeDownCast(Face));
  return Face;
}

vtkModelFace* vtkDiscreteModel::BuildModelFace(int NumEdges, vtkModelEdge** Edges,
                                          int* EdgeDirections)
{
  return this->BuildModelFace(
    NumEdges, Edges, EdgeDirections, this->GetNextUniquePersistentId());
}

vtkModelFace* vtkDiscreteModel::BuildModelFace(int numEdges, vtkModelEdge** edges,
                                          int* edgeDirections, vtkIdType modelFaceId)
{
  vtkDiscreteModelFace* face = vtkDiscreteModelFace::New();
  this->AddAssociation(face->GetType(), face);
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
  this->AddAssociation(region->GetType(), region);

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

vtkModelRegion* vtkDiscreteModel::BuildModelRegion(int NumFaces,
                                              vtkModelFace** Faces,
                                              int* FaceSides)
{
  return this->BuildModelRegion(NumFaces, Faces, FaceSides,
                                this->GetNextUniquePersistentId());
}

vtkModelRegion* vtkDiscreteModel::BuildModelRegion(
  int numFaces, vtkModelFace** faces, int* faceSides, vtkIdType modelRegionId)
{
  vtkDiscreteModelRegion* region = vtkDiscreteModelRegion::New();
  region->Initialize(numFaces, faces, faceSides,
                     modelRegionId);
  this->AddAssociation(region->GetType(), region);

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
  int NumFaces, vtkModelFace** Faces, int* FaceSides,
  vtkModelMaterial* Material)
{
  return this->BuildModelRegion(
    NumFaces, Faces, FaceSides,
    this->GetNextUniquePersistentId(), Material);
}

vtkModelRegion* vtkDiscreteModel::BuildModelRegion(
  int NumFaces, vtkModelFace** Faces, int* FaceSides,
  vtkIdType ModelRegionId, vtkModelMaterial* Material)
{
  vtkDiscreteModelRegion* Region = vtkDiscreteModelRegion::SafeDownCast(
    this->BuildModelRegion(NumFaces, Faces, FaceSides, ModelRegionId));
  if(Material)
    {
    Material->AddModelGeometricEntity(Region);
    }
  return Region;
}

bool vtkDiscreteModel::DestroyModelGeometricEntity(vtkDiscreteModelGeometricEntity* GeomEntity)
{
  vtkModelMaterial* Material = GeomEntity->GetMaterial();
  if(Material)
    {
    vtkModelGeometricEntity* vtkGeomEntity =
      vtkModelGeometricEntity::SafeDownCast(GeomEntity->GetThisModelEntity());
    Material->RemoveModelGeometricEntity(vtkGeomEntity);
    }
  vtkModelItemIterator* iter = GeomEntity->NewModelEntityGroupIterator();
  while(GeomEntity->GetNumberOfModelEntityGroups())
    {
    iter->Begin();
    vtkDiscreteModelEntityGroup::SafeDownCast(iter->GetCurrentItem())->RemoveModelEntity(GeomEntity);
    }
  iter->Delete();

  vtkModelGeometricEntity* vtkGeomEntity =
    vtkModelGeometricEntity::SafeDownCast(GeomEntity->GetThisModelEntity());
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
  vtkModelMaterial* Material = vtkModelMaterial::New();
  Material->Initialize(id);
  this->AddAssociation(Material->GetType(), Material);
  this->SetLargestUsedUniqueId(std::max(this->GetLargestUsedUniqueId(), id));
  Material->Delete();

  std::string DefaultEntityName;
  this->GetModelEntityDefaultName(vtkModelMaterialType, "DomainSet",
                                  DefaultEntityName);
  vtkModelUserName::SetUserName(Material, DefaultEntityName.c_str());
  vtkIdType entityId = Material->GetUniquePersistentId();
  this->InternalInvokeEvent(DomainSetCreated, &entityId);

  return Material;
}

bool vtkDiscreteModel::DestroyMaterial(vtkModelMaterial* Material)
{
  if(!Material->IsDestroyable())
    {
    return false;
    }
  vtkIdType entityId = Material->GetUniquePersistentId();
  this->InternalInvokeEvent(DomainSetAboutToDestroy, &entityId);
  if(!Material->Destroy())
    {
    vtkErrorMacro("Problem destroying material.");
    return false;
    }
  this->RemoveAssociation(Material->GetType(), Material);
  this->InternalInvokeEvent(DomainSetDestroyed, &entityId);

  this->Modified();
  return true;
}

vtkDiscreteModelEntityGroup* vtkDiscreteModel::BuildModelEntityGroup(
  int itemType, int NumEntities, vtkDiscreteModelEntity** Entities)
{
  return this->BuildModelEntityGroup(
    itemType, NumEntities, Entities, this->GetNextUniquePersistentId());
}

vtkDiscreteModelEntityGroup* vtkDiscreteModel::BuildModelEntityGroup(
  int itemType, int NumEntities, vtkDiscreteModelEntity** Entities,
  vtkIdType id)
{
  vtkDiscreteModelEntityGroup* EntityGroup = vtkDiscreteModelEntityGroup::New();
  EntityGroup->Initialize(id);
  EntityGroup->SetEntityType(itemType);
  for(int i=0;i<NumEntities;i++)
    {
    EntityGroup->AddModelEntity(Entities[i]);
    }
  this->AddAssociation(EntityGroup->GetType(), EntityGroup);
  EntityGroup->Delete();
  this->SetLargestUsedUniqueId(std::max(this->GetLargestUsedUniqueId(), id));

  std::string DefaultEntityName;
  this->GetModelEntityDefaultName(vtkDiscreteModelEntityGroupType, "BCS Group",
                                  DefaultEntityName);
  vtkModelUserName::SetUserName(EntityGroup, DefaultEntityName.c_str());
  vtkIdType entityId = EntityGroup->GetUniquePersistentId();
  this->InternalInvokeEvent(ModelEntityGroupCreated, &entityId);

  return EntityGroup;
}

bool vtkDiscreteModel::DestroyModelEntityGroup(vtkDiscreteModelEntityGroup* EntityGroup)
{
  if(!EntityGroup->IsDestroyable())
    {
    return 0;
    }
  vtkIdType entityId = EntityGroup->GetUniquePersistentId();
  this->InternalInvokeEvent(ModelEntityGroupAboutToDestroy, &entityId);

  if(!EntityGroup->Destroy())
    {
    vtkErrorMacro("Problem destroying entity group.");
    return 0;
    }
  this->RemoveAssociation(EntityGroup->GetType(), EntityGroup);
  this->InternalInvokeEvent(ModelEntityGroupDestroyed, &entityId);

  this->Modified();
  return 1;
}

vtkModelNodalGroup* vtkDiscreteModel::BuildNodalGroup(int Type, vtkIdList* PointIds)
{
  return this->BuildNodalGroup(Type, PointIds, this->GetNextUniquePersistentId());
}

vtkModelNodalGroup* vtkDiscreteModel::BuildNodalGroup(int Type, vtkIdList* PointIds,
                                               vtkIdType Id)
{
  vtkModelNodalGroup* NodalGroup = 0;
  if(Type == BASE_NODAL_GROUP)
    {
    NodalGroup = vtkModelNodalGroup::New();
    }
  else if(Type == UNIQUE_NODAL_GROUP)
    {
    NodalGroup = vtkModelUniqueNodalGroup::New();
    }
  else
    {
    vtkErrorMacro("Bad Type.");
    return 0;
    }
  if(PointIds && PointIds->GetNumberOfIds())
    {
    NodalGroup->AddPointIds(PointIds);
    }
  NodalGroup->Initialize(Id);

  this->AddAssociation(NodalGroup->GetType(), NodalGroup);
  NodalGroup->Delete();

  std::string DefaultEntityName;
  this->GetModelEntityDefaultName(vtkModelNodalGroupType, "Nodal Group",
                                  DefaultEntityName);
  vtkModelUserName::SetUserName(NodalGroup, DefaultEntityName.c_str());
  vtkIdType entityId = NodalGroup->GetUniquePersistentId();
  this->InternalInvokeEvent(NodalGroupCreated, &entityId);

  return NodalGroup;
}

bool vtkDiscreteModel::DestroyNodalGroup(vtkModelNodalGroup* NodalGroup)
{
  if(!NodalGroup->IsDestroyable())
    {
    return 0;
    }
  vtkIdType entityId = NodalGroup->GetUniquePersistentId();
  this->InternalInvokeEvent(NodalGroupAboutToDestroy, &entityId);
  if(!NodalGroup->Destroy())
    {
    vtkErrorMacro("Problem destroying entity group.");
    return 0;
    }
  this->RemoveAssociation(NodalGroup->GetType(), NodalGroup);
  this->InternalInvokeEvent(NodalGroupDestroyed, &entityId);

  this->Modified();
  return 1;
}

bool vtkDiscreteModel::DestroyModelEdge(vtkDiscreteModelEdge* ModelEdge)
{
  if(!ModelEdge->IsDestroyable())
    {
    return 0;
    }
  this->Modified();
  if(!ModelEdge->Destroy())
    {
    vtkErrorMacro("Problem destroying entity group.");
    return 0;
    }
  this->RemoveAssociation(ModelEdge->GetType(), ModelEdge);

  return 1;
}

vtkDiscreteModelGeometricEntity* vtkDiscreteModel::GetCellModelGeometricEntity(
  vtkIdType CellId)
{
  size_t size = CellId;
  if(CellId < 0 || size >= this->CellClassification.size())
    {
    vtkWarningMacro("Bad CellId.");
    return 0;
    }
  return this->CellClassification[CellId];
}

void vtkDiscreteModel::SetGeometry(vtkObject* Geometry)
{
  // may want to make a deep copy of geometry
  // for now assume that we don't need to
  this->GetProperties()->Set(GEOMETRY(), Geometry);
  vtkPolyData* poly = vtkPolyData::SafeDownCast(Geometry);
  if(poly)
    {
    poly->GetBounds(this->ModelBounds);
    // we're on the server so set the vector sizes for classification
    vtkIdType NumCells = poly->GetNumberOfCells();
    vtkDiscreteModelGeometricEntity* GeomEnt = 0;
    this->CellClassification.resize(NumCells, GeomEnt);
    this->ClassifiedCellIndex.resize(NumCells, -1);
    vtkModelUniqueNodalGroup* NodalGroup = 0;
    this->UniquePointGroup.resize(poly->GetNumberOfPoints(), NodalGroup);
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
    vtkIdType NumCells = poly->GetNumberOfCells();
    vtkDiscreteModelGeometricEntity* GeomEnt = 0;
    this->CellClassification.resize(NumCells, GeomEnt);
    this->ClassifiedCellIndex.resize(NumCells, -1);
    vtkModelUniqueNodalGroup* NodalGroup = 0;
    this->UniquePointGroup.resize(poly->GetNumberOfPoints(), NodalGroup);
    }
  this->Modified();
}

void vtkDiscreteModel::GetModelEntityDefaultName(int EntityType, const char* BaseName,
                                            std::string & DefaultEntityName)
{
  // Default names are BaseName + LocalEntityNumber (e.g. "Material 8").
  int LargestDefaultNameNumber = 0;
  vtkModelItemIterator* Iter = this->NewIterator(EntityType);
  for(Iter->Begin();!Iter->IsAtEnd();Iter->Next())
    {
    int BaseNameLength = strlen(BaseName);
    vtkModelEntity* Entity = vtkModelEntity::SafeDownCast(
      Iter->GetCurrentItem());
    if(!Entity || vtkModelUserName::GetUserName(Entity) == 0)
      {
      continue;
      }
    const char* UserName = vtkModelUserName::GetUserName(Entity);
    if(!strncmp(BaseName, UserName, BaseNameLength))
      {
      int Number = atoi(UserName+BaseNameLength);
      if(Number > LargestDefaultNameNumber)
        {
        LargestDefaultNameNumber = Number;
        }

      }
    }
  char Name[200];
  sprintf(Name, "%s%d", BaseName, LargestDefaultNameNumber+1);
  DefaultEntityName = Name;

  Iter->Delete();
}

vtkObject* vtkDiscreteModel::GetGeometry()
{
  vtkObject* object = vtkObject::SafeDownCast(
    this->GetProperties()->Get(GEOMETRY()));
  return object;
}

const char* vtkDiscreteModel::GetPointMapArrayName()
{
  return "CMBPointMapArray";
}

const char* vtkDiscreteModel::GetCellMapArrayName()
{
  return "CMBCellMapArray";
}

const char* vtkDiscreteModel::GetCanonicalSideArrayName()
{
  return "CMBCanonicalSideArray";
}

vtkIdType vtkDiscreteModel::GetCellModelGeometricEntityIndex(vtkIdType CellId)
{
  size_t size = CellId;
  if(CellId < 0 || size >= this->ClassifiedCellIndex.size())
    {
    vtkWarningMacro("Bad CellId.");
    return 0;
    }
  return this->ClassifiedCellIndex[CellId];
}

void vtkDiscreteModel::SetCellClassification(
  vtkIdType MasterCellId, vtkIdType GeomEntityCellId, vtkDiscreteModelGeometricEntity* Entity)
{
  vtkIdType NumCells = this->ClassifiedCellIndex.size();
  if(MasterCellId >= 0 && MasterCellId < NumCells)
    {
    this->ClassifiedCellIndex[MasterCellId] = GeomEntityCellId;
    this->CellClassification[MasterCellId] = Entity;
    }
  else
    {
    vtkErrorMacro("Bad master cell id value.");
    }
}

void vtkDiscreteModel::SetPointUniqueNodalGroup(
  vtkModelUniqueNodalGroup* NodalGroup, vtkIdType PointId)
{
  vtkIdType NumPoints = this->UniquePointGroup.size();
  if(PointId < 0 || PointId >= NumPoints)
    {
    vtkErrorMacro("Bad master point id value.");
    }
  else
    {
    if(this->UniquePointGroup[PointId] &&
       this->UniquePointGroup[PointId] != NodalGroup)
      {
      this->UniquePointGroup[PointId]->Superclass::RemovePointId(PointId);
      }
    this->UniquePointGroup[PointId] = NodalGroup;
    }
}

vtkModelUniqueNodalGroup* vtkDiscreteModel::GetPointUniqueNodalGroup(vtkIdType PointId)
{
  vtkIdType NumPoints = this->UniquePointGroup.size();
  if(PointId < 0 || PointId >= NumPoints)
    {
    vtkErrorMacro("Bad master point id value.");
    return 0;
    }
  return this->UniquePointGroup[PointId];
}

void vtkDiscreteModel::Reset()
{
 // Destroy entity groups
  // model entity groups
  vtkModelItemIterator* EntityGroupIter = this->NewIterator(vtkDiscreteModelEntityGroupType);
  for(EntityGroupIter->Begin();!EntityGroupIter->IsAtEnd();EntityGroupIter->Next())
    {
    bool destroyed =
      vtkDiscreteModelEntityGroup::SafeDownCast(EntityGroupIter->GetCurrentItem())->Destroy();
    if(!destroyed)
      {
      vtkErrorMacro("Problem destroying an entity group.");
      }
    }
  EntityGroupIter->Delete();
  this->RemoveAllAssociations(vtkDiscreteModelEntityGroupType);
  // nodal groups
  vtkModelItemIterator* NodalGroupIter = this->NewIterator(vtkModelNodalGroupType);
  for(NodalGroupIter->Begin();!NodalGroupIter->IsAtEnd();NodalGroupIter->Next())
    {
    bool destroyed =
      vtkModelNodalGroup::SafeDownCast(NodalGroupIter->GetCurrentItem())->Destroy();
    if(!destroyed)
      {
      vtkErrorMacro("Problem destroying a nodal group.");
      }
    }
  NodalGroupIter->Delete();
  this->RemoveAllAssociations(vtkModelNodalGroupType);
  this->UniquePointGroup.clear();

  // Destroy materials
  vtkModelItemIterator* MaterialIter = this->NewIterator(vtkModelMaterialType);
  for(MaterialIter->Begin();!MaterialIter->IsAtEnd();MaterialIter->Next())
    {
    bool destroyed =
      vtkModelMaterial::SafeDownCast(MaterialIter->GetCurrentItem())->Destroy();
    if(!destroyed)
      {
      vtkErrorMacro("Problem destroying a material.");
      }
    }
  MaterialIter->Delete();
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
