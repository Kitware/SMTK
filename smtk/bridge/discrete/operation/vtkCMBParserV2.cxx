//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCMBParserV2.h"

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
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStdString.h"
#include "vtkStringArray.h"

#include <map>
#


vtkStandardNewMacro(vtkCMBParserV2);



vtkCMBParserV2::vtkCMBParserV2()
{
}

vtkCMBParserV2:: ~vtkCMBParserV2()
{
}

bool vtkCMBParserV2::Parse(vtkPolyData* MasterPoly, vtkDiscreteModel* Model)
{
  Model->Reset();

  this->SetGeometry(Model, MasterPoly);

  // find out what the largest Unique ID is...
  vtkIdTypeArray* MaterialIdsArray = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray("MaterialIds"));
  vtkIdTypeArray* RegionIdsArray = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray("ModelRegionIds"));
  vtkIdTypeArray* FaceIdsArray = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray("ModelFaceIds"));
  vtkIdType materialMax = MaterialIdsArray->GetRange()[1];
  vtkIdType regionMax = RegionIdsArray->GetRange()[1];
  vtkIdType faceMax = FaceIdsArray->GetRange()[1];
  vtkIdType maxUniqueId = materialMax > regionMax ? materialMax : regionMax;
  maxUniqueId = maxUniqueId > faceMax ? maxUniqueId : faceMax;
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

  // next load in the model faces and add in the model face uses
  vtkIdTypeArray* ModelFaceRegions = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetModelFaceRegionsString()));
  ModelEntities.resize(ModelFaceRegions->GetNumberOfTuples());
  // map from region id to vector of pair of model face and side (i.e. face use)
  std::map<vtkIdType, std::vector<std::pair<vtkDiscreteModelFace*, int> > > FacesOfRegion;
  for(vtkIdType i=0;i<FaceIdsArray->GetNumberOfTuples();i++)
    {
    vtkDiscreteModelFace* face = vtkDiscreteModelFace::SafeDownCast(
      Model->BuildModelFace(0, 0, 0, FaceIdsArray->GetValue(i)));
    ModelEntities[i] = face;
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
  FaceIdsArray->Delete();
  ModelFaceRegions->Delete();
  ModelFaceRegions = 0;
  this->SetModelEntityData(MasterPoly, ModelEntities, "ModelFace", Model);
  // now that the ids are properly set we can add the cells to the model faces
  CellToModelType faceToIds; //classification
  vtkIdTypeArray* CellClassification = this->NewIdTypeArray(
    MasterPoly->GetCellData()->GetArray(
    vtkCMBParserBase::GetModelFaceTagName()));
  if(!CellClassification)
    {
    vtkErrorMacro("Cannot get cell classification information.");
    return 0;
    }

  //in a V2 idspace the ids are going from 0 to N, and what we do
  //is drop all the edges off and just bring in faces
  this->SeparateCellClassification(Model,CellClassification,faceToIds);

  for(std::map<vtkIdType, vtkSmartPointer<vtkIdList> >::iterator it=faceToIds.begin();
      it!=faceToIds.end();it++)
    {
    vtkDiscreteModelFace* Face = vtkDiscreteModelFace::SafeDownCast(
      Model->GetModelEntity(vtkModelFaceType, it->first));
    this->AddCellsToGeometry(Face, it->second);
    }
  CellClassification->Delete();
  CellClassification = 0;

  // next load in the regions
  vtkIdTypeArray* RegionMaterials = this->NewIdTypeArray(
    MasterPoly->GetFieldData()->GetArray("ModelRegionMaterials"));
  vtkDoubleArray* PointInside =  vtkDoubleArray::SafeDownCast(
    MasterPoly->GetFieldData()->GetArray("ModelRegionPointInside"));
  vtkIntArray* pointInsideValidity = vtkIntArray::SafeDownCast(
    MasterPoly->GetFieldData()->GetArray("ModelRegionPointInsideValidity"));
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

  // next load in floating edges
  if(MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetFloatingEdgesString()))
    {
    vtkIdTypeArray* EdgeRegionlId = this->NewIdTypeArray(
      MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetFloatingEdgesString()));
    if(EdgeRegionlId)
      {
      vtkDoubleArray* endPoints =  vtkDoubleArray::SafeDownCast(
        MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetModelEdgeVerticesString()));
      vtkIntArray* lineSpacing = vtkIntArray::SafeDownCast(
        MasterPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetEdgeLineResolutionString()));
      ModelEntities.resize(EdgeRegionlId->GetNumberOfTuples());
      double point1[3], point2[3];
      for(vtkIdType i=0;i<EdgeRegionlId->GetNumberOfTuples();i++)
        {
        vtkIdType RegionId = EdgeRegionlId->GetValue(i);
        endPoints->GetTupleValue(2*i, point1);
        endPoints->GetTupleValue(2*i+1, point2);
        vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(
          Model->BuildFloatingRegionEdge(point1, point2, lineSpacing->GetValue(i), RegionId));
        ModelEntities[i] = edge;
        }
      this->SetModelEntityData(MasterPoly, ModelEntities, "ModelEdge", Model);
      EdgeRegionlId->Delete();
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
    vtkIdType ArrayCounter = 0; // used to keep track of where we are in GroupedEntityIds
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

  // deal with the model grid to analysis grid mapping
  vtkDataArray* pointMapArray =
    MasterPoly->GetPointData()->GetArray(vtkDiscreteModel::GetPointMapArrayName());
  vtkDataArray* cellMapArray =
    MasterPoly->GetCellData()->GetArray(vtkDiscreteModel::GetCellMapArrayName());
  vtkCharArray* canonicalSideArray = vtkCharArray::SafeDownCast(
    MasterPoly->GetCellData()->GetArray(vtkDiscreteModel::GetCanonicalSideArrayName()));
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
  MasterPoly->GetPointData()->RemoveArray("ModelPointMapArray");
  MasterPoly->GetCellData()->RemoveArray("modelfaceids");
  MasterPoly->GetCellData()->RemoveArray("ModelCellMapArray");
  MasterPoly->GetCellData()->RemoveArray("ModelCanonicalSideArray");
  MasterPoly->GetFieldData()->RemoveArray("version");
  MasterPoly->GetFieldData()->RemoveArray("ModelFaceRegions");
  MasterPoly->GetFieldData()->RemoveArray("ModelFaceIds");
  MasterPoly->GetFieldData()->RemoveArray("ModelFaceColor");
  MasterPoly->GetFieldData()->RemoveArray("ModelFaceVisibility");
  MasterPoly->GetFieldData()->RemoveArray("ModelFaceUserName");
  MasterPoly->GetFieldData()->RemoveArray("ModelRegionMaterials");
  MasterPoly->GetFieldData()->RemoveArray("ModelRegionPointInside");
  MasterPoly->GetFieldData()->RemoveArray("ModelRegionPointInsideValidity");
  MasterPoly->GetFieldData()->RemoveArray("ModelRegionIds");
  MasterPoly->GetFieldData()->RemoveArray("ModelRegionColor");
  MasterPoly->GetFieldData()->RemoveArray("ModelRegionVisibility");
  MasterPoly->GetFieldData()->RemoveArray("ModelRegionUserName");
  MasterPoly->GetFieldData()->RemoveArray("MaterialIds");
  MasterPoly->GetFieldData()->RemoveArray("MaterialColor");
  MasterPoly->GetFieldData()->RemoveArray("MaterialVisibility");
  MasterPoly->GetFieldData()->RemoveArray("MaterialUserName");

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

void vtkCMBParserV2::SetModelEntityData(
  vtkPolyData* Poly, std::vector<vtkModelEntity*> & ModelEntities,
  const char* BaseArrayName, vtkDiscreteModel* Model)
{
  std::string Base(BaseArrayName);
  std::string Name = Base+"Ids";
  vtkIdTypeArray* EntityIds = this->NewIdTypeArray(
    Poly->GetFieldData()->GetArray(Name.c_str()));
  Name = Base + "Color";
  vtkDoubleArray* EntityRGBA = vtkDoubleArray::SafeDownCast(
    Poly->GetFieldData()->GetArray(Name.c_str()));
  Name = Base + "Visibility";
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
    // username
    vtkStdString username = EntityUserName->GetValue(i);
    vtkModelUserName::SetUserName(Entity, username);
    }
  EntityIds->Delete();
  this->SetLargestUsedUniqueId(Model, MaxId);
}

void vtkCMBParserV2::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
