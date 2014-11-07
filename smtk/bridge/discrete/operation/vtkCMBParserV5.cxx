//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCMBParserV5.h"

#include "Bridge.h"

#include "vtkCharArray.h"
#include "vtkModel3dm2DGridRepresentation.h"
#include "vtkModelBCGridRepresentation.h"
#include "vtkModelMaterial.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkModelUserName.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkModelFaceUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelShellUse.h"
#include "vtkModelVertex.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStdString.h"
#include "vtkStringArray.h"
#include "ModelParserHelper.h"

#include <map>
#include <vector>
#include <stdio.h>
#include <string.h>

/// The parser keeps a single bridge around to track all the models it imports.
smtk::shared_ptr<smtk::bridge::discrete::Bridge> vtkCMBParserV5::s_bridge;

vtkStandardNewMacro(vtkCMBParserV5);



vtkCMBParserV5::vtkCMBParserV5()
{
  if (!vtkCMBParserV5::s_bridge)
    vtkCMBParserV5::s_bridge = smtk::bridge::discrete::Bridge::create();
}

vtkCMBParserV5:: ~vtkCMBParserV5()
{
}

bool vtkCMBParserV5::Parse(vtkPolyData* MasterPoly, vtkDiscreteModel* Model)
{
  Model->Reset();

  this->SetGeometry(Model, MasterPoly);

  // find out what the largest Unique ID is...
  vtkIdTypeArray* MaterialIdsArray = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetMaterialUniquePersistentIdsString()));
  vtkIdTypeArray* RegionIdsArray = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelRegionUniquePersistentIdsString()));
  vtkIdTypeArray* FaceIdsArray = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelFaceUniquePersistentIdsString()));
  vtkIdTypeArray* EdgeIdsArray = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelEdgeUniquePersistentIdsString()));
  vtkIdTypeArray* VertexIdsArray = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelVertexUniquePersistentIdsString()));

  vtkIdType maxUniqueId = MaterialIdsArray->GetRange()[1];
  if(RegionIdsArray)
    {
    vtkIdType Max = RegionIdsArray->GetRange()[1];
    maxUniqueId = maxUniqueId > Max ? maxUniqueId : Max;
    }
  if(FaceIdsArray)
    {
    vtkIdType Max = FaceIdsArray->GetRange()[1];
    maxUniqueId = maxUniqueId > Max ? maxUniqueId : Max;
    }
  if(EdgeIdsArray)
    {
    vtkIdType Max = EdgeIdsArray->GetRange()[1];
    maxUniqueId = maxUniqueId > Max ? maxUniqueId : Max;
    }
  if(VertexIdsArray)
    {
    vtkIdType Max = VertexIdsArray->GetRange()[1];
    maxUniqueId = maxUniqueId > Max ? maxUniqueId : Max;
    }
  this->SetLargestUsedUniqueId(Model, maxUniqueId);

  // load in the model's UUID if it exists.
  std::vector<vtkModelItem*> modelRecords;
  modelRecords.push_back(Model);
  vtkCMBParserV5::s_bridge->assignUUIDs(modelRecords,
    MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelUUIDsString()));
  // TODO: Handle submodels whose UUIDs are also in the field-data array.
  //       Need to know how the order in the file may be obtained.

  // first load in the materials
  std::vector<vtkModelEntity*> ModelEntities(MaterialIdsArray->GetNumberOfTuples());
  for(vtkIdType i=0;i<MaterialIdsArray->GetNumberOfTuples();i++)
    {
    vtkModelMaterial* material = Model->BuildMaterial(MaterialIdsArray->GetValue(i));
    ModelEntities[i] = material;
    }
  MaterialIdsArray->Delete();
  vtkCMBParserV5::s_bridge->assignUUIDs(
    std::vector<vtkModelItem*>(ModelEntities.begin(), ModelEntities.end()),
    MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetMaterialUUIDsString()));
  this->SetModelEntityData(MasterPoly, ModelEntities, "Material", Model);

  // next load in the model vertices
  if(VertexIdsArray)
    {
    if(VertexIdsArray->GetNumberOfTuples() > 0)
      {
      const vtkIdType numVertices  = VertexIdsArray->GetNumberOfTuples();
      vtkFieldData* masterFieldData = MasterPoly->GetFieldData();

      vtkIdTypeArray* vertexPointIds = this->NewIdTypeArray(
        masterFieldData->GetArray(ModelParserHelper::GetVertexPointIdString()));

      if(!vertexPointIds)
        {
        vtkErrorMacro("Could not add model vertex.");
        return 0;
        }

      ModelEntities.resize(numVertices);
      for(vtkIdType i=0;i<numVertices;i++)
        {
        const vtkIdType pointId = vertexPointIds->GetValue(i);
        ModelEntities[i] = Model->BuildModelVertex(pointId);
        }
      vertexPointIds->Delete();
      this->SetModelEntityData(MasterPoly, ModelEntities, "ModelVertex", Model);
      vtkCMBParserV5::s_bridge->assignUUIDs(
        std::vector<vtkModelItem*>(ModelEntities.begin(), ModelEntities.end()),
        MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelVertexUUIDsString()));
      }
    VertexIdsArray->Delete();
    VertexIdsArray = 0;
    }
  // next load in the model edges
  if(EdgeIdsArray)
    {
    vtkIdTypeArray* EdgeVertices = this->NewIdTypeArray(
      MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelEdgeVerticesString()));
    ModelEntities.resize(EdgeIdsArray->GetNumberOfTuples());
    for(vtkIdType i=0;i<EdgeIdsArray->GetNumberOfTuples();i++)
      {
      vtkModelVertex* Vertices[2] = {NULL,NULL};
      vtkIdType VertexIds[2];
      EdgeVertices->GetTupleValue(i, VertexIds);
      for(int j=0;j<2;j++)
        {
        Vertices[j] = vtkModelVertex::SafeDownCast(
          Model->GetModelEntity(vtkModelVertexType, VertexIds[j]));
        if(Vertices[j] == 0 && VertexIds[j] >= 0)
          {
          vtkErrorMacro("Could not find vertex needed by edge.");
          return 0;
          }
        }
      vtkModelEdge* Edge = Model->BuildModelEdge(Vertices[0], Vertices[1], EdgeIdsArray->GetValue(i));
      ModelEntities[i] = Edge;
      }
    EdgeIdsArray->Delete();
    EdgeIdsArray = 0;
    this->SetModelEntityData(MasterPoly, ModelEntities, "ModelEdge", Model);
    vtkCMBParserV5::s_bridge->assignUUIDs(
      std::vector<vtkModelItem*>(ModelEntities.begin(), ModelEntities.end()),
      MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelEdgeUUIDsString()));
    EdgeVertices->Delete();
    }

  // next load in the model faces and add in the model face uses
  vtkIdTypeArray* ModelFaceRegions = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelFaceRegionsString()));
  vtkIdTypeArray* EdgesOfModelFace = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelFaceEdgesString()));
  vtkIntArray* EdgeDirections = vtkIntArray::SafeDownCast(
    MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelEdgeDirectionsString()));
  std::vector<vtkModelEdge*> Edges;
  std::vector<int> EdgeDirs;
  if(EdgesOfModelFace)
    {
    Edges.resize(EdgesOfModelFace->GetNumberOfTuples());
    EdgeDirs.resize(EdgeDirections->GetNumberOfTuples());
    }
  ModelEntities.resize(FaceIdsArray->GetNumberOfTuples());
  vtkIdTypeArray* FaceMaterials =
    this->NewIdTypeArray(MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetFaceMaterialIdString()));

  // map from region id to vector of pair of model face and side (i.e. face use)
  std::map<vtkIdType, std::vector<std::pair<vtkDiscreteModelFace*, int> > > FacesOfRegion;
  vtkIdType ArrayCounter = 0;
  for(vtkIdType i=0;i<FaceIdsArray->GetNumberOfTuples();i++)
    {
    // Create the face
    vtkDiscreteModelFace* face;
    if(FaceMaterials && FaceMaterials->GetValue(i) >= 0)
      { // a model face in a 2D model
      // First get the number of loops
      int nLoops = EdgesOfModelFace->GetValue(ArrayCounter++);
      int loop;
      vtkModelMaterial* Material = vtkModelMaterial::SafeDownCast(
        Model->GetModelEntity(vtkModelMaterialType, FaceMaterials->GetValue(i)));
      face = vtkDiscreteModelFace::SafeDownCast(
        Model->BuildModelFace(0, NULL, NULL, Material) );
      // Process each face loop (assume that the first loop is the outer one
      for (loop = 0; loop < nLoops; ++loop)
        {
        int EdgeCounter = 0;
        while(ArrayCounter < EdgesOfModelFace->GetNumberOfTuples() &&
              EdgesOfModelFace->GetValue(ArrayCounter) >= 0)
          {
          vtkIdType EdgeId = EdgesOfModelFace->GetValue(ArrayCounter);
          Edges[EdgeCounter] =
            vtkModelEdge::SafeDownCast(Model->GetModelEntity(vtkModelEdgeType, EdgeId));
          EdgeDirs[EdgeCounter] = EdgeDirections->GetValue(ArrayCounter);
          ArrayCounter++;
          EdgeCounter++;
          }
        ArrayCounter++; // incremented because we want to skip the -1 value
        bool blockEvent = Model->GetBlockModelGeometricEntityEvent();
        Model->SetBlockModelGeometricEntityEvent(true);
        face->AddLoop(EdgeCounter, &Edges[0], &EdgeDirs[0]);
        Model->SetBlockModelGeometricEntityEvent(blockEvent);
        }
      }
    else
      { // a model face in a 3D model
      face = vtkDiscreteModelFace::SafeDownCast(
        Model->BuildModelFace(0, NULL, NULL, FaceIdsArray->GetValue(i)));
      vtkIdType RegionIds[2];
      ModelFaceRegions->GetTupleValue(i, RegionIds);
      for(int j=0;j<2;j++)
        {
        if(RegionIds[j] >= 0) // should be -1 if no region exists on that side
          {
          std::map<vtkIdType, std::vector<std::pair<vtkDiscreteModelFace*, int> > >::iterator it =
            FacesOfRegion.find(RegionIds[j]);
          if(it != FacesOfRegion.end())
            {
            it->second.push_back(std::pair<vtkDiscreteModelFace*,int>(face, j));
            }
          else
            {
            std::vector<std::pair<vtkDiscreteModelFace*, int> > Data(1, std::pair<vtkDiscreteModelFace*,int>(face, j));
            FacesOfRegion[RegionIds[j]] = Data;
            }
          }
        }
      }
    ModelEntities[i] = face;
    }
  if(EdgesOfModelFace)
    {
    EdgesOfModelFace->Delete();
    EdgesOfModelFace = 0;
    }
  FaceIdsArray->Delete();
  FaceIdsArray = 0;
  if(ModelFaceRegions)
    {
    ModelFaceRegions->Delete();
    ModelFaceRegions = 0;
    }
  if(FaceMaterials)
    {
    FaceMaterials->Delete();
    FaceMaterials = 0;
    }
  this->SetModelEntityData(MasterPoly, ModelEntities, "ModelFace", Model);
  vtkCMBParserV5::s_bridge->assignUUIDs(
    std::vector<vtkModelItem*>(ModelEntities.begin(), ModelEntities.end()),
    MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelFaceUUIDsString()));
  // now that the ids are properly set we can add the cells to the model faces and/or edges

  CellToModelType CellToModelEntityIds; //classification
  vtkIdTypeArray* CellClassification = this->NewIdTypeArray(
    MasterPoly->GetCellData()->GetArray(
    ModelParserHelper::GetModelFaceTagName()));
  if(!CellClassification)
    {
    vtkErrorMacro("Cannot get cell classification information.");
    return 0;
    }

  this->SeparateCellClassification(Model,CellClassification,CellToModelEntityIds);

  for(std::map<vtkIdType, vtkSmartPointer<vtkIdList> >::iterator it=CellToModelEntityIds.begin();
      it!=CellToModelEntityIds.end();it++)
    {
    vtkDiscreteModelGeometricEntity* ModelEntity = vtkDiscreteModelFace::SafeDownCast(
      Model->GetModelEntity(vtkModelFaceType, it->first));
    if(ModelEntity)
      {
      this->AddCellsToGeometry(ModelEntity, it->second);
      }
    else if( (ModelEntity = vtkDiscreteModelEdge::SafeDownCast(
              Model->GetModelEntity(vtkModelEdgeType, it->first))))
      {
      this->AddCellsToGeometry(ModelEntity, it->second);
      }
    else
      {
      vtkErrorMacro("Cannot find geometric model entity to classify cells on.");
      }

    }
  CellClassification->Delete();
  CellClassification = 0;

  // next load in the regions
  if(RegionIdsArray)
    {
    vtkIdTypeArray* RegionMaterials = this->NewIdTypeArray(
      MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelRegionsMaterialsString()));
    vtkDoubleArray* PointInside =  vtkDoubleArray::SafeDownCast(
      MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelRegionPointInsideString()));
    vtkIntArray* pointInsideValidity = vtkIntArray::SafeDownCast(
      MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelRegionPointInsideValidity()));
    ModelEntities.resize(RegionIdsArray->GetNumberOfTuples());
    for(vtkIdType i=0;i<RegionIdsArray->GetNumberOfTuples();i++)
      {
      vtkIdType RegionId = RegionIdsArray->GetValue(i);
      std::vector<std::pair<vtkDiscreteModelFace*, int> > FaceData =
        FacesOfRegion[RegionId];
      size_t numFaces = FaceData.size();
      std::vector<vtkModelFace*> Faces(numFaces);
      std::vector<int> FaceSides(numFaces);
      for(size_t j=0;j<numFaces;j++)
        {
        Faces[j] = FaceData[j].first;
        FaceSides[j] = FaceData[j].second;
        }
      vtkModelMaterial* Material = vtkModelMaterial::SafeDownCast(
        Model->GetModelEntity(vtkModelMaterialType, RegionMaterials->GetValue(i)));
      vtkDiscreteModelRegion* region = vtkDiscreteModelRegion::SafeDownCast(
        Model->BuildModelRegion(static_cast<vtkIdType>(numFaces), &Faces[0],
                                &FaceSides[0], Material));
      // if the pointInsideValidity array exists, then value must equal 1 for
      // this region to add the point
      if (!pointInsideValidity || pointInsideValidity->GetValue(i))
        {
        double Point[3];
        PointInside->GetTupleValue(i, Point);
        region->SetPointInside(Point);
        }
      ModelEntities[i] = region;
      }
    RegionIdsArray->Delete();
    RegionIdsArray = 0;
    RegionMaterials->Delete();
    RegionMaterials = 0;
    this->SetModelEntityData(MasterPoly, ModelEntities, "ModelRegion", Model);
    vtkCMBParserV5::s_bridge->assignUUIDs(
      std::vector<vtkModelItem*>(ModelEntities.begin(), ModelEntities.end()),
      MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelRegionUUIDsString()));
    }

  // next associate any floating edges with their region
  if(MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetFloatingEdgesString()))
    {
    vtkIdTypeArray* EdgeRegionId = this->NewIdTypeArray(
      MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetFloatingEdgesString()));
    vtkIdTypeArray* EdgeId = this->NewIdTypeArray(
      MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelEdgeUniquePersistentIdsString()));
    if(EdgeRegionId)
      {
      for(vtkIdType i=0;i<EdgeRegionId->GetNumberOfTuples();i++)
        {
        vtkIdType RegionId = EdgeRegionId->GetValue(i);
        if(RegionId >= 0)
          {
          vtkDiscreteModelEdge* Edge = vtkDiscreteModelEdge::SafeDownCast(
            Model->GetModelEntity(vtkModelEdgeType, EdgeId->GetValue(i)));
          Edge->AddRegionAssociation(RegionId);
          }
        }
      EdgeRegionId->Delete();
      EdgeId->Delete();
      }
    }

  // next load in the entity groups (in the file called bcs's)
  vtkIdTypeArray* EntityGroupIds = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray("ModelEntityGroupIds"));
  if(EntityGroupIds)
    {
    vtkIdTypeArray* GroupedEntityIds = this->NewIdTypeArray(
      MasterPoly->GetFieldData()->GetArray("ModelEntityGroupEntityIds"));
    ModelEntities.resize(EntityGroupIds->GetNumberOfTuples());
    ArrayCounter = 0; // used to keep track of where we are in GroupedEntityIds
    for(vtkIdType i=0;i<EntityGroupIds->GetNumberOfTuples();i++)
      {
      int itemType = -1;
      std::vector<vtkDiscreteModelEntity*> Entities;
      vtkIdType numEntities = GroupedEntityIds->GetValue(ArrayCounter);
      ArrayCounter++;
      if(numEntities)
        {
        vtkModelEntity* Entity = Model->GetModelEntity(GroupedEntityIds->GetValue(ArrayCounter));
        ArrayCounter++;
        itemType = Entity->GetType();
        vtkDiscreteModelEntity* CMBEntity = vtkDiscreteModelEntity::GetThisDiscreteModelEntity(Entity);
        if(CMBEntity)
          {
          Entities.push_back(CMBEntity);
          }
        for(vtkIdType j=1;j<numEntities;j++)
          {
          Entity = Model->GetModelEntity(
            itemType, GroupedEntityIds->GetValue(ArrayCounter));
          ArrayCounter++;
          if(itemType == Entity->GetType())
            {
            CMBEntity = vtkDiscreteModelEntity::GetThisDiscreteModelEntity(Entity);
            if(CMBEntity)
              {
              Entities.push_back(CMBEntity);
              }
            }
          }
        }
      if(numEntities != static_cast<vtkIdType>(Entities.size()))
        { // may have filtered out "bad" entities so adjust numEntities
        vtkErrorMacro("Problem with entities in a group");
        numEntities = static_cast<vtkIdType>(Entities.size());
        }
      ModelEntities[i] =
        Model->BuildModelEntityGroup(itemType, numEntities,
          numEntities ? (&Entities[0]) : NULL);
      }
    EntityGroupIds->Delete();
    EntityGroupIds = 0;
    GroupedEntityIds->Delete();
    GroupedEntityIds = 0;
    this->SetModelEntityData(MasterPoly, ModelEntities, "ModelEntityGroup", Model);
    vtkCMBParserV5::s_bridge->assignUUIDs(
      std::vector<vtkModelItem*>(ModelEntities.begin(), ModelEntities.end()),
      MasterPoly->GetFieldData()->GetArray(ModelParserHelper::GetModelGroupUUIDsString()));
    }

  // TODO: Handle uses and shells whose UUIDs are also in field-data arrays.
  //       Need to know how the order in the file may be obtained.

  // deal with the mapping from model grid to analysis grid
  this->SetAnalysisGridData(MasterPoly, Model);

  MasterPoly->GetFieldData()->RemoveArray(ModelParserHelper::GetFileVersionString());
  MasterPoly->GetFieldData()->RemoveArray(ModelParserHelper::GetModelFaceRegionsString());
  MasterPoly->GetFieldData()->RemoveArray("ModelFaceMaterials");
  MasterPoly->GetFieldData()->RemoveArray("ModelRegionMaterials");
  MasterPoly->GetFieldData()->RemoveArray("ModelRegionPointInside");
  MasterPoly->GetFieldData()->RemoveArray("ModelRegionPointInsideValidity");
  MasterPoly->GetFieldData()->RemoveArray("NodalGroupPointIds");
  MasterPoly->GetFieldData()->RemoveArray("EdgeVertices");
  MasterPoly->GetFieldData()->RemoveArray("edgedirections");
  MasterPoly->GetFieldData()->RemoveArray("ModelFaceEdges");
  MasterPoly->GetFieldData()->RemoveArray(ModelParserHelper::GetModelUUIDsString());
  MasterPoly->GetFieldData()->RemoveArray(ModelParserHelper::GetMaterialUUIDsString());
  MasterPoly->GetFieldData()->RemoveArray(ModelParserHelper::GetModelGroupUUIDsString());
  MasterPoly->GetFieldData()->RemoveArray(ModelParserHelper::GetModelVertexUUIDsString());
  MasterPoly->GetFieldData()->RemoveArray(ModelParserHelper::GetModelRegionUUIDsString());
  MasterPoly->GetFieldData()->RemoveArray(ModelParserHelper::GetModelEdgeUUIDsString());
  MasterPoly->GetFieldData()->RemoveArray(ModelParserHelper::GetModelFaceUUIDsString());
  MasterPoly->GetFieldData()->RemoveArray(ModelParserHelper::GetModelVertexUseUUIDsString());
  MasterPoly->GetFieldData()->RemoveArray(ModelParserHelper::GetModelEdgeUseUUIDsString());
  MasterPoly->GetFieldData()->RemoveArray(ModelParserHelper::GetModelFaceUseUUIDsString());
  MasterPoly->GetFieldData()->RemoveArray(ModelParserHelper::GetModelShellUUIDsString());
  MasterPoly->GetFieldData()->RemoveArray(ModelParserHelper::GetModelLoopUUIDsString());

  // put warning if people try to store other arrays in the master polydata
  if(MasterPoly->GetPointData()->GetNumberOfArrays() != 0)
    {
    vtkDebugMacro("There is point data on the master poly -- we may be wasting memory.");
    for(int ii=0;ii<MasterPoly->GetPointData()->GetNumberOfArrays();ii++)
      {
      vtkDebugMacro("Point data array name is " <<
                      MasterPoly->GetPointData()->GetArrayName(ii));
      }
    }
  if(MasterPoly->GetCellData()->GetNumberOfArrays() != 0)
    {
    vtkDebugMacro("There is cell data on the master poly -- we may be wasting memory.");
    for(int ii=0;ii<MasterPoly->GetCellData()->GetNumberOfArrays();ii++)
      {
      vtkDebugMacro("Cell data array name is " <<
                      MasterPoly->GetCellData()->GetArrayName(ii));
      }
    }
  if(MasterPoly->GetFieldData()->GetNumberOfArrays() != 0)
    {
    vtkDebugMacro("There is field data on the master poly -- we may be wasting memory.");
    for(int ii=0;ii<MasterPoly->GetFieldData()->GetNumberOfArrays();ii++)
      {
      vtkDebugMacro("Field data array name is " <<
                      MasterPoly->GetFieldData()->GetArrayName(ii));
      }
    }
  return 1;
}

void vtkCMBParserV5::SetModelEntityData(
  vtkPolyData* Poly, std::vector<vtkModelEntity*> & ModelEntities,
  const char* BaseArrayName, vtkDiscreteModel* Model)
{
  std::string Base(BaseArrayName);
  std::string Name = Base+"Ids";
  vtkIdTypeArray* EntityIds = this->NewIdTypeArray(
    Poly->GetFieldData()->GetArray(Name.c_str()));
  if(strcmp(Base.c_str(), "ModelEdge"))
    {
    Poly->GetFieldData()->RemoveArray(Name.c_str());
    }
  Name = Base + "Color";
  vtkDoubleArray* EntityRGBA = vtkDoubleArray::SafeDownCast(
    Poly->GetFieldData()->GetArray(Name.c_str()));
  Name = Base + "Visibility";
  vtkIntArray* EntityVisibility = vtkIntArray::SafeDownCast(
    Poly->GetFieldData()->GetArray(Name.c_str()));
  Name = Base + "UserName";
  vtkStringArray* EntityUserName = vtkStringArray::SafeDownCast(
    Poly->GetFieldData()->GetAbstractArray(Name.c_str()));

  vtkIdType NumEnts = ModelEntities.size();
  vtkIdType MaxId = 0;
  for(vtkIdType i=0;i<NumEnts;i++)
    {
    vtkModelEntity* Entity = ModelEntities[i];
    // unique persistent id
    vtkIdType Id = EntityIds->GetValue(i);
    if(Id > MaxId)
      {
      MaxId = Id;
      }
    this->SetUniquePersistentId(Entity, Id);
    // color
    double RGBA[4];
    EntityRGBA->GetTupleValue(i, RGBA);
    Entity->SetColor(RGBA[0], RGBA[1], RGBA[2], RGBA[3]);
    // visibility
    Entity->SetVisibility(1);
//    Entity->SetVisibility(EntityVisibility->GetValue(i));
    // username
    vtkStdString username = EntityUserName->GetValue(i);
    vtkModelUserName::SetUserName(Entity, username);
    if(vtkDiscreteModelVertex* vertex = vtkDiscreteModelVertex::SafeDownCast(
      Entity))
      {
      vertex->CreateGeometry();
      }
    }
  Poly->GetFieldData()->RemoveArray(EntityRGBA->GetName());
  Poly->GetFieldData()->RemoveArray(EntityVisibility->GetName());
  Poly->GetFieldData()->RemoveArray(EntityUserName->GetName());

  EntityIds->Delete();
  this->SetLargestUsedUniqueId(Model, MaxId);
}

void vtkCMBParserV5::SetAnalysisGridData(vtkPolyData* masterPoly, vtkDiscreteModel* model)
{
  //the first part is for when the analysis grid info comes from importing
  // a 3dm file directly

  // For backward compatibility, we have to check the old array name for
  // previously save files.
  vtkPointData* masterPoints = masterPoly->GetPointData();
  vtkCellData* masterCells = masterPoly->GetCellData();
  const char* pointMapName = masterPoints->HasArray(
    vtkDiscreteModel::GetPointMapArrayName()) ?
    vtkDiscreteModel::GetPointMapArrayName() : "CMBPointMapArray";
  const char* cellMapName = masterCells->HasArray(
    vtkDiscreteModel::GetCellMapArrayName()) ?
    vtkDiscreteModel::GetCellMapArrayName() : "CMBCellMapArray";
  const char* cellCSName = masterCells->HasArray(
    vtkDiscreteModel::GetCanonicalSideArrayName()) ?
    vtkDiscreteModel::GetCanonicalSideArrayName() : "CMBCanonicalSideArray";

  vtkDataArray* pointMapArray = masterPoints->GetArray(pointMapName);
  vtkDataArray* cellMapArray = masterCells->GetArray(cellMapName);
  vtkCharArray* canonicalSideArray = vtkCharArray::SafeDownCast(
    masterCells->GetArray(cellCSName));
  vtkStringArray* analysisGridType = vtkStringArray::SafeDownCast(
    masterPoly->GetFieldData()->GetAbstractArray(ModelParserHelper::GetAnalysisGridFileType()));
  if(pointMapArray || cellMapArray || canonicalSideArray)
    {
    if(pointMapArray && cellMapArray && canonicalSideArray)
      {
      this->Superclass::SetAnalysisGridInfo(
        model, pointMapArray, cellMapArray, canonicalSideArray);
      }
    else
      {
      vtkWarningMacro("There seems to be some information missing for"
                      << " mapping back to the analysis mesh.");
      }
    }
  else if(analysisGridType && analysisGridType->GetVariantValue(0).ToString() == "2D ADH")
    {
    vtkModel3dm2DGridRepresentation* gridRepresentation =
      vtkModel3dm2DGridRepresentation::New();
    model->SetAnalysisGridInfo(gridRepresentation);
    gridRepresentation->Delete();
    }
  // we now try to load information if we imported a bc file
  else
    {
    // the analysis grid info came from a bc file
    vtkDebugMacro("Currently the vtkModelBCGridRepresentation is saved out separately in a m2m file");
    }
  if(vtkStringArray* gridName = vtkStringArray::SafeDownCast(
       masterPoly->GetFieldData()->GetAbstractArray(ModelParserHelper::GetAnalysisGridFileName())))
    {
    vtkVariant name = gridName->GetVariantValue(0);
    model->GetAnalysisGridInfo()->SetGridFileName(name.ToString().c_str());
    }

  // delete out some unneeded arrays
  masterPoints->RemoveArray(pointMapName);

  masterCells->RemoveArray(ModelParserHelper::GetCellClassificationName());
  masterCells->RemoveArray(cellMapName);
  masterCells->RemoveArray(cellCSName);
}
void vtkCMBParserV5::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
