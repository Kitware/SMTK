//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCMBParserV4.h"

#include "vtkCharArray.h"
#include "vtkModelMaterial.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
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

#include <map>
#include <vector>
#include <stdio.h>
#include <string.h>



vtkStandardNewMacro(vtkCMBParserV4);



vtkCMBParserV4::vtkCMBParserV4()
{
}

vtkCMBParserV4:: ~vtkCMBParserV4()
{
}

bool vtkCMBParserV4::Parse(vtkPolyData* MasterPoly, vtkDiscreteModel* Model)
{
  Model->Reset();

  //The original V4 reader would store all the vertex locations in the
  //GetVertexLocationString array which represents the vtkModelVertex.
  //Commit eeede4daa converted to storing model vertex coordinates in the master
  //polydata. But by this point we had given out V4 files which has vertex locations
  //stored inside the GetVertexLocationString array.

  //So to fix the problem we insert more points into the poly data before we
  //make it the mesh for our model
  vtkFieldData* masterFieldData = MasterPoly->GetFieldData();
  vtkIdTypeArray* vertexPointIds = this->NewIdTypeArray(
    masterFieldData->GetArray(vtkCMBParserBase::GetVertexPointIdString()));

  vtkDoubleArray* locationArray = vtkDoubleArray::SafeDownCast(
    masterFieldData->GetArray(vtkCMBParserBase::GetVertexLocationString()));

  if(vertexPointIds && locationArray)
    {
    vtkPoints* points = MasterPoly->GetPoints();
    const vtkIdType numVertices = vertexPointIds->GetNumberOfTuples();
    double xyz[3];
    for(vtkIdType i=0;i<numVertices;i++)
      {
      vtkIdType pointId = vertexPointIds->GetValue(i);
      if(pointId < 0)
        {
        locationArray->GetTuple(i, xyz);
        pointId = points->InsertNextPoint(xyz);
        vertexPointIds->SetValue(i,pointId);
        }
      }
    }

  this->SetGeometry(Model, MasterPoly);

  // find out what the largest Unique ID is...
  vtkIdTypeArray* MaterialIdsArray = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetMaterialUniquePersistentIdsString()));
  vtkIdTypeArray* RegionIdsArray = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetModelRegionUniquePersistentIdsString()));
  vtkIdTypeArray* FaceIdsArray = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetModelFaceUniquePersistentIdsString()));
  vtkIdTypeArray* EdgeIdsArray = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetModelEdgeUniquePersistentIdsString()));
  vtkIdTypeArray* VertexIdsArray = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetModelVertexUniquePersistentIdsString()));

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

  // first load in the materials
  std::vector<vtkModelEntity*> ModelEntities(MaterialIdsArray->GetNumberOfTuples());
  for(vtkIdType i=0;i<MaterialIdsArray->GetNumberOfTuples();i++)
    {
    vtkModelMaterial* material = Model->BuildMaterial(MaterialIdsArray->GetValue(i));
    ModelEntities[i] = material;
    }
  MaterialIdsArray->Delete();
  this->SetModelEntityData(MasterPoly, ModelEntities, "Material", Model);

  // next load in the model vertices
  if(VertexIdsArray)
    {
    if(VertexIdsArray->GetNumberOfTuples() > 0)
      {
      const vtkIdType numVertices  = VertexIdsArray->GetNumberOfTuples();
      vtkFieldData* masterFieldDataTmp = MasterPoly->GetFieldData();

      vtkIdTypeArray* vertexPointIdsTmp = this->NewIdTypeArray(
        masterFieldDataTmp->GetArray(vtkCMBParserBase::GetVertexPointIdString()));
      if(!vertexPointIdsTmp)
        {
        vtkErrorMacro("Could not add model vertex.");
        return 0;
        }

      ModelEntities.resize(numVertices);
      for(vtkIdType i=0;i<numVertices;i++)
        {
        const vtkIdType pointId = vertexPointIdsTmp->GetValue(i);
        ModelEntities[i] = Model->BuildModelVertex(pointId);
        }
      vertexPointIdsTmp->Delete();
      this->SetModelEntityData(MasterPoly, ModelEntities, "ModelVertex", Model);
      }
    VertexIdsArray->Delete();
    VertexIdsArray = 0;
    }
  // next load in the model edges
  if(EdgeIdsArray)
    {
    vtkIdTypeArray* EdgeVertices = this->NewIdTypeArray(
      MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetModelEdgeVerticesString()));
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
    EdgeVertices->Delete();
    }

  // next load in the model faces and add in the model face uses
  vtkIdTypeArray* ModelFaceRegions = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetModelFaceRegionsString()));
  vtkIdTypeArray* EdgesOfModelFace = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetModelFaceEdgesString()));
  vtkIntArray* EdgeDirections = vtkIntArray::SafeDownCast(
    MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetModelEdgeDirectionsString()));
  std::vector<vtkModelEdge*> Edges(EdgesOfModelFace->GetNumberOfTuples());
  std::vector<int> EdgeDirs(EdgeDirections->GetNumberOfTuples());;
  ModelEntities.resize(FaceIdsArray->GetNumberOfTuples());
  vtkIdTypeArray* FaceMaterials =
    this->NewIdTypeArray(MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetFaceMaterialIdString()));

  // map from region id to vector of pair of model face and side (i.e. face use)
  std::map<vtkIdType, std::vector<std::pair<vtkDiscreteModelFace*, int> > > FacesOfRegion;
  vtkIdType ArrayCounter = 0;
  for(vtkIdType i=0;i<FaceIdsArray->GetNumberOfTuples();i++)
    {
    // collect adjacent model edge info first
    int EdgeCounter = 0;
    while(ArrayCounter < EdgesOfModelFace->GetNumberOfTuples() &&
      EdgesOfModelFace->GetValue(ArrayCounter) >= 0)
      {
      vtkIdType EdgeId = EdgesOfModelFace->GetValue(ArrayCounter);
      Edges[EdgeCounter] = vtkModelEdge::SafeDownCast(Model->GetModelEntity(vtkModelEdgeType, EdgeId));
      EdgeDirs[EdgeCounter] = EdgeDirections->GetValue(ArrayCounter);
      ArrayCounter++;
      EdgeCounter++;
      }
    ArrayCounter++; // incremented because we want to skip the -1 value
    vtkDiscreteModelFace* face = 0;
    if(FaceMaterials && FaceMaterials->GetValue(i) >= 0)
      {
      vtkModelMaterial* Material = vtkModelMaterial::SafeDownCast(
        Model->GetModelEntity(vtkModelMaterialType, FaceMaterials->GetValue(i)));
      if(EdgeCounter == 0)
        {
        face = vtkDiscreteModelFace::SafeDownCast(
          Model->BuildModelFace(EdgeCounter, NULL, NULL, Material) );
        }
      else
        {
        face = vtkDiscreteModelFace::SafeDownCast(
          Model->BuildModelFace(EdgeCounter, &Edges[0], &EdgeDirs[0], Material) );
        }
      }
    else
      {
      if(EdgeCounter == 0)
        {
        face = vtkDiscreteModelFace::SafeDownCast(
          Model->BuildModelFace(EdgeCounter, NULL, NULL, FaceIdsArray->GetValue(i)));
        }
      else
        {
        face = vtkDiscreteModelFace::SafeDownCast(
          Model->BuildModelFace(EdgeCounter, &Edges[0], &EdgeDirs[0],
                              FaceIdsArray->GetValue(i)));
        }
      }

    ModelEntities[i] = face;
    if(ModelFaceRegions)
      {
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
      } // if(ModelFaceRegions)
    }
  EdgesOfModelFace->Delete();
  EdgesOfModelFace = 0;
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
  // now that the ids are properly set we can add the cells to the model faces and/or edges

  CellToModelType CellToModelEntityIds; //classification
  vtkIdTypeArray* CellClassification = this->NewIdTypeArray(
    MasterPoly->GetCellData()->GetArray(
    vtkCMBParserBase::GetModelFaceTagName()));

  if(!CellClassification)
    {
    vtkErrorMacro("Cannot get cell classification information.");
    return 0;
    }

  //in a V4 idspace the ids are going from 0 to N, where
  //edges and faces are mixed together. What we need to do
  //is convert them into a negative and positive space
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
              Model->GetModelEntity(vtkModelEdgeType, it->first)) ) )
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
      MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetModelRegionsMaterialsString()));
    vtkDoubleArray* PointInside =  vtkDoubleArray::SafeDownCast(
      MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetModelRegionPointInsideString()));
    vtkIntArray* pointInsideValidity = vtkIntArray::SafeDownCast(
      MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetModelRegionPointInsideValidity()));
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
        Model->BuildModelRegion(static_cast<vtkIdType>(numFaces),
                                &Faces[0], &FaceSides[0], Material));
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
    }

  // next associate any floating edges with their region
  if(MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetFloatingEdgesString()))
    {
    vtkIdTypeArray* EdgeRegionId = this->NewIdTypeArray(
      MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetFloatingEdgesString()));
    vtkIdTypeArray* EdgeId = this->NewIdTypeArray(
      MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetModelEdgeUniquePersistentIdsString()));
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
          vtkModelEntity* Entity = Model->GetModelEntity(
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
      if(static_cast<size_t>(numEntities) != Entities.size())
        { // may have filtered out "bad" entities so adjust numEntities
        vtkErrorMacro("Problem with entities in a group");
        numEntities = static_cast<vtkIdType>(Entities.size());
        }
      ModelEntities[i] =
        Model->BuildModelEntityGroup(itemType, numEntities,&Entities[0]);
      }
    EntityGroupIds->Delete();
    EntityGroupIds = 0;
    GroupedEntityIds->Delete();
    GroupedEntityIds = 0;
    this->SetModelEntityData(MasterPoly, ModelEntities, "ModelEntityGroup", Model);
    }

  // deal with the mapping from model grid to analysis grid.
  // For backward compatibility, we have to check the old array name for
  // previously save files.
  vtkPointData* masterPoints = MasterPoly->GetPointData();
  vtkCellData* masterCells = MasterPoly->GetCellData();
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

  if(pointMapArray || cellMapArray || canonicalSideArray)
    {
    if(pointMapArray && cellMapArray && canonicalSideArray)
      {
      this->SetAnalysisGridInfo(Model, pointMapArray,
                                cellMapArray, canonicalSideArray);
      }
    else
      {
      vtkWarningMacro("There seems to be some information missing for"
                      << " mapping back to the analysis mesh.");
      }
    }

  // delete out some unneeded arrays
  masterPoints->RemoveArray(pointMapName);

  masterCells->RemoveArray(vtkCMBParserBase::GetCellClassificationName());
  masterCells->RemoveArray(cellMapName);
  masterCells->RemoveArray(cellCSName);

  MasterPoly->GetFieldData()->RemoveArray(vtkCMBParserBase::GetFileVersionString());
  MasterPoly->GetFieldData()->RemoveArray(vtkCMBParserBase::GetModelFaceRegionsString());
  MasterPoly->GetFieldData()->RemoveArray("ModelFaceMaterials");
  MasterPoly->GetFieldData()->RemoveArray("ModelRegionMaterials");
  MasterPoly->GetFieldData()->RemoveArray("ModelRegionPointInside");
  MasterPoly->GetFieldData()->RemoveArray("ModelRegionPointInsideValidity");
  MasterPoly->GetFieldData()->RemoveArray("EdgeVertices");
  MasterPoly->GetFieldData()->RemoveArray("edgedirections");
  MasterPoly->GetFieldData()->RemoveArray("ModelFaceEdges");

  // put warning if people try to store other arrays in the master polydata
  if(masterPoints->GetNumberOfArrays() != 0)
    {
    vtkDebugMacro("There is point data on the master poly -- we may be wasting memory.");
    for(int ii=0;ii<masterPoints->GetNumberOfArrays();ii++)
      {
      vtkDebugMacro("Point data array name is " <<
                      masterPoints->GetArrayName(ii));
      }
    }
  if(masterCells->GetNumberOfArrays() != 0)
    {
    vtkDebugMacro("There is cell data on the master poly -- we may be wasting memory.");
    for(int ii=0;ii<masterCells->GetNumberOfArrays();ii++)
      {
      vtkDebugMacro("Cell data array name is " <<
                      masterCells->GetArrayName(ii));
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

void vtkCMBParserV4::SetModelEntityData(
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
    }
  Poly->GetFieldData()->RemoveArray(EntityRGBA->GetName());
  Poly->GetFieldData()->RemoveArray(EntityVisibility->GetName());
  Poly->GetFieldData()->RemoveArray(EntityUserName->GetName());

  EntityIds->Delete();
  this->SetLargestUsedUniqueId(Model, MaxId);
}

void vtkCMBParserV4::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
