//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelBCGridRepresentation.h"

#include <iostream>
#include <string>
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include "vtkModelItemIterator.h"
#include <vtkObjectFactory.h>
#include <vtksys/SystemTools.hxx>

#include "vtkNew.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"

vtkStandardNewMacro(vtkModelBCGridRepresentation);

//----------------------------------------------------------------------------
vtkModelBCGridRepresentation::vtkModelBCGridRepresentation()
{
}

//----------------------------------------------------------------------------
vtkModelBCGridRepresentation::~vtkModelBCGridRepresentation()
{
}

//----------------------------------------------------------------------------
void inline AddPointIds(vtkIdType /*entId*/,vtkIdList* inPtsList,
  vtkIdList* outPtsList)
{
  if(!inPtsList || inPtsList->GetNumberOfIds()==0)
    {
    return;
    }

  vtkIdType newNumIds = inPtsList->GetNumberOfIds();
  for(vtkIdType i=0; i<newNumIds; i++)
    {
    outPtsList->InsertUniqueId(inPtsList->GetId(i));
    }
}

//----------------------------------------------------------------------------
bool vtkModelBCGridRepresentation::GetBCSNodalAnalysisGridPointIds(
  vtkDiscreteModel* model, vtkIdType bcsGroupId,
  int bcGroupType,vtkIdList* pointIds)
{
  pointIds->Reset();
  if(vtkDiscreteModelEntityGroup* bcsNodalGroup =
    vtkDiscreteModelEntityGroup::SafeDownCast(
    model->GetModelEntity(vtkDiscreteModelEntityGroupType, bcsGroupId)))
    {
    vtkModelItemIterator* iterFace=bcsNodalGroup->NewIterator(vtkModelFaceType);
    for(iterFace->Begin();!iterFace->IsAtEnd();iterFace->Next())
      {
      vtkDiscreteModelFace* entity =
        vtkDiscreteModelFace::SafeDownCast(iterFace->GetCurrentItem());
      if(entity)
        {
        vtkIdType entityId = entity->GetUniquePersistentId();
        vtkNew<vtkIdList> inPtsList;
        if(bcGroupType == 1)// vtkSBBCInstance::enBCModelEntityAllNodesType)
          {
          entity->GetAllPointIds(inPtsList.GetPointer());
          AddPointIds(entityId, inPtsList.GetPointer(),pointIds);
          }
        else if(bcGroupType == 2)//vtkSBBCInstance::enBCModelEntityBoundaryNodesType)
          {
          entity->GetBoundaryPointIds(inPtsList.GetPointer());
          AddPointIds(entityId, inPtsList.GetPointer(),pointIds);
          }
        else if(bcGroupType == 3)//vtkSBBCInstance::enBCModelEntityInteriorNodesType)
          {
          entity->GetInteriorPointIds(inPtsList.GetPointer());
          AddPointIds(entityId, inPtsList.GetPointer(),pointIds);
          }
        }
      }
    iterFace->Delete();
    return true;
    }
  else
    {
    vtkErrorMacro("A BCS nodal group can not be found");
    this->Reset();
    }
  pointIds->Reset();
  return false;
}

//----------------------------------------------------------------------------
bool vtkModelBCGridRepresentation::GetFloatingEdgeAnalysisGridPointIds(
  vtkDiscreteModel* model, vtkIdType floatingEdgeId, vtkIdList* pointIds)
{
  pointIds->Reset();
  if(vtkDiscreteModelEdge* floatingEdge =
     vtkDiscreteModelEdge::SafeDownCast(model->GetModelEntity(vtkModelEdgeType, floatingEdgeId)))
    {
    std::map<vtkIdType, std::set<vtkIdType> >::iterator it=
      this->FloatingEdgeToPointIds.find(floatingEdgeId);
    if(it!=this->FloatingEdgeToPointIds.end())
      {
      if(static_cast<size_t>(floatingEdge->GetLineResolution()+1) == it->second.size())
        {
        pointIds->SetNumberOfIds(floatingEdge->GetLineResolution()+1);
        vtkIdType counter = 0;
        for(std::set<vtkIdType>::iterator sit=it->second.begin();
            sit!=it->second.end();sit++)
          {
          pointIds->SetId(counter, *sit);
          counter++;
          }
        return true;
        }
      }
    vtkErrorMacro("A floating edge has changed since loading the bc information."
                  << " The Omicron information will be reset.");
    this->Reset();
    }

  pointIds->Reset();
  return false;
}

//----------------------------------------------------------------------------
bool vtkModelBCGridRepresentation::GetModelEdgeAnalysisPoints(
  vtkDiscreteModel* /*model*/, vtkIdType /*boundaryGroupId*/, vtkIdTypeArray* /*edgePoints*/)
{
  vtkErrorMacro("2D models not supported.");
  return false;
}

//----------------------------------------------------------------------------
bool vtkModelBCGridRepresentation::GetBoundaryGroupAnalysisFacets(
  vtkDiscreteModel* model, vtkIdType boundaryGroupId,
  vtkIdList* cellIds, vtkIdList* cellSides)
{
  cellIds->Reset();
  cellSides->Reset();
  vtkDiscreteModelEntityGroup* boundaryGroup =
    vtkDiscreteModelEntityGroup::SafeDownCast(
      model->GetModelEntity(vtkDiscreteModelEntityGroupType, boundaryGroupId));
  if(!boundaryGroup)
    {
    vtkWarningMacro("Bad boundary group id.");
    return false;
    }
  vtkNew<vtkIdList> faceCellIds;
  vtkNew<vtkIdList> faceCellSides;
  vtkSmartPointer<vtkModelItemIterator> faces;
  faces.TakeReference(boundaryGroup->NewIterator(vtkModelFaceType));
  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    faceCellIds->Reset();
    faceCellSides->Reset();
    vtkDiscreteModelFace* face =
      vtkDiscreteModelFace::SafeDownCast(faces->GetCurrentItem());
    if(!face || this->GetModelFaceAnalysisFacets(model,
      face->GetUniquePersistentId(), faceCellIds.GetPointer(),
      faceCellSides.GetPointer()) == false)
      {
      vtkErrorMacro("A boundary group has changed since loading the bc information."
                    << " The Omicron information will be reset.");
      this->Reset();
      cellIds->Reset();
      cellSides->Reset();
      return false;
      }
    for(vtkIdType i=0;i<faceCellIds->GetNumberOfIds();i++)
      {
      cellIds->InsertNextId(faceCellIds->GetId(i));
      cellSides->InsertNextId(faceCellSides->GetId(i));
      }
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkModelBCGridRepresentation::IsModelConsistent(vtkDiscreteModel* model)
{
  // get the floating edges of the 3D model
  std::set<vtkIdType> floatingEdgeIds;
  vtkModelItemIterator* edges = model->NewIterator(vtkModelEdgeType);
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(
      edges->GetCurrentItem());
    if(edge->GetModelRegion())
      {
      floatingEdgeIds.insert(edge->GetUniquePersistentId());
      }
    }
  edges->Delete();
  int number = static_cast<int>(floatingEdgeIds.size());
  if(number !=  static_cast<int>(this->FloatingEdgeToPointIds.size()))
    {
    vtkErrorMacro("There are " << number << " floating edges but the bc file has "
                  << this->FloatingEdgeToPointIds.size());
    this->Reset();
    return false;
    }
  vtkIdType numCells = model->GetMesh().GetNumberOfCells();
  if(numCells !=  static_cast<vtkIdType>(this->MasterCellToMeshCellInfo.size()))
    {
    vtkErrorMacro("There are " << numCells << " model cells but the bc file has "
                  << this->MasterCellToMeshCellInfo.size() << " boundary model cells.");
    this->Reset();
    return false;
    }

  for(std::map<vtkIdType, std::set<vtkIdType> >::iterator it=this->FloatingEdgeToPointIds.begin();
      it!=this->FloatingEdgeToPointIds.end();it++)
    {
    if(this->IsFloatingEdgeConsistent(model, it->first) == false)
      {
      this->Reset();
      return false;
      }
    }
  for(std::map<vtkIdType, std::set<std::pair<vtkIdType, int> > >::iterator it=
        this->MasterCellToMeshCellInfo.begin();
      it!=this->MasterCellToMeshCellInfo.end();it++)
    {
    if(this->IsModelFaceConsistent(model, it->first) == false)
      {
      this->Reset();
      return false;
      }
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkModelBCGridRepresentation::Initialize(
  const char* bcFileName, vtkDiscreteModel* model)
{
  this->Reset();
  if(bcFileName == NULL ||
     vtksys::SystemTools::FileExists(bcFileName, true) == false)
    {
    if(bcFileName == NULL)
      {
      vtkErrorMacro("Passed in empty file name.");
      }
    else
      {
      vtkErrorMacro("Cannot find file " << bcFileName);
      }
    return false;
    }
  std::ifstream file(bcFileName);
  if(file.is_open() == false)
    {
    vtkErrorMacro("Problem opening file " << bcFileName);
    return false;
    }
  // get the floating edges of the 3D model
  std::set<vtkIdType> floatingEdgeIds;
  vtkModelItemIterator* edges = model->NewIterator(vtkModelEdgeType);
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(
      edges->GetCurrentItem());
    if(edge->GetModelRegion())
      {
      floatingEdgeIds.insert(edge->GetUniquePersistentId());
      }
    }
  edges->Delete();

  std::string line;
  while(!file.eof())
    {
    std::getline(file, line);
    if(line.size() == 0)
      {
      continue;
      }
    std::vector<vtksys::String> values = vtksys::SystemTools::SplitString(line.c_str(), ' ');
    if(vtksys::SystemTools::Strucmp(values[0].c_str(), "NDS") == 0 && values.size() == 3)
      {
      vtkIdType pointId = atoi(values[1].c_str()) -1; // analysis grid point Id in C++ ordering
      vtkIdType entityId = atoi(values[2].c_str());
      if(vtkDiscreteModelEntityGroup::SafeDownCast(model->GetModelEntity(
         vtkDiscreteModelEntityGroupType, entityId)))
        {
        // We should not have NDS from .bc file any more, since the nodal group is now
        // defined as BCS group with nodal-type (ALL, Boundary, Interior)
        // by vtkSBBCInstance::enOtherBCGroupType
        // this->BCSGroupToPointIds[entityId].insert(pointId);
        vtkWarningMacro("This is an old version of .bc file with NDS card " << entityId);
        }
      else if(floatingEdgeIds.find(entityId) != floatingEdgeIds.end())
        {
        this->FloatingEdgeToPointIds[entityId].insert(pointId);
        }
      else
        {
        vtkErrorMacro("Bad NDS card entity Id " << entityId);
        this->Reset();
        file.close();
        return false;
        }
      }
    // The new version (July-29-2011) of the .bc file from omicron
    // added the model-masterpoly-cell-id at the end for FCS card
    else if(vtksys::SystemTools::Strucmp(values[0].c_str(), "FCS") == 0 &&
            (values.size() == 8 || values.size() == 7))
      {
      if(values.size() == 7)
        {
        vtkErrorMacro("This is an old verion of omicron .bc file. Please rerun omicron.");
        this->Reset();
        file.close();
        return false;
        }

      vtkIdType cellId = atoi(values[1].c_str())-1; // analysis grid cell Id in C++ ordering
      int cellSide = atoi(values[2].c_str())-1; // analysis grid cell side (0-3)
      vtkIdType modelCellId = atoi(values[7].c_str());
      this->MasterCellToMeshCellInfo[modelCellId].insert(
        std::pair<vtkIdType, int>(cellId, cellSide));
      }
    else if(line.empty() == 0)
      {
      while(line[line.size()-1] == ' ' && line.empty() == 0)
        {
        line.resize(line.size()-1);
        }
      if(line.empty() == 0)
        {
        vtkErrorMacro("Unable to parse line '" << line << "' in " << bcFileName);
        this->Reset();
        file.close();
        return false;
        }
      }
    }
  file.close();
  if(this->IsModelConsistent(model) == false)
    {
    return false;
    }

  // assume that the grid and bc file name have the same base name so just
  // change the extension to 3dm from .bc
  std::string gridName, path;
  path = vtksys::SystemTools::GetFilenamePath(bcFileName);
  gridName = vtksys::SystemTools::GetFilenameWithoutExtension(bcFileName);
  path.append("/");
  path.append(gridName);
  path.append(".3dm");
  this->SetGridFileName(path.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool vtkModelBCGridRepresentation::AddFloatingEdge(
  vtkIdType floatingEdgeId, vtkIdList* pointIds, vtkDiscreteModel* model)
{
  std::map<vtkIdType, std::set<vtkIdType> >::iterator it=
    this->FloatingEdgeToPointIds.find(floatingEdgeId);
  if(it==this->FloatingEdgeToPointIds.end())
    {
    it = this->FloatingEdgeToPointIds.insert(
      this->FloatingEdgeToPointIds.begin(),
      std::make_pair(floatingEdgeId, std::set<vtkIdType>()));
    }

  for(vtkIdType i=0;i<pointIds->GetNumberOfIds();i++)
    {
    it->second.insert(pointIds->GetId(i));
    }

  if(IsFloatingEdgeConsistent(model, floatingEdgeId) == true)
    {
    return true;
    }
  this->Reset();
  return false;
}

//----------------------------------------------------------------------------
bool vtkModelBCGridRepresentation::AddModelFace(
  vtkIdType modelFaceId, vtkIdList* cellIds,
  vtkIdList* cellSides, vtkDiscreteModel* model)
{
  vtkDiscreteModelFace* face = vtkDiscreteModelFace::SafeDownCast(
    model->GetModelEntity(vtkModelFaceType, modelFaceId));
  if(face == NULL || face->GetNumberOfCells()==0)
    {
    vtkErrorMacro("Could not find model face.");
    this->Reset();
    return false;
    }
  vtkIdTypeArray* MasterCellIds = face->GetReverseClassificationArray();
  vtkIdType numFaceCells = face->GetNumberOfCells();
  vtkIdType masterCellId, cellId;
  int cellSide;
  for(vtkIdType i=0; i<numFaceCells; i++)
    {
    masterCellId = MasterCellIds->GetValue(i);
    cellId = cellIds->GetId(i);
    cellSide = cellSides->GetId(i);
    this->MasterCellToMeshCellInfo[masterCellId].insert(
      std::pair<vtkIdType, int>(cellId, cellSide));
    }
  return true;
}

//----------------------------------------------------------------------------
void vtkModelBCGridRepresentation::Reset()
{
  this->Superclass::Reset();
  this->FloatingEdgeToPointIds.clear();
  this->MasterCellToMeshCellInfo.clear();
}

//----------------------------------------------------------------------------
bool vtkModelBCGridRepresentation::IsFloatingEdgeConsistent(
  vtkDiscreteModel* model, vtkIdType floatingEdgeId)
{
  std::map<vtkIdType, std::set<vtkIdType> >::iterator it=
    this->FloatingEdgeToPointIds.find(floatingEdgeId);
  if(it!=this->FloatingEdgeToPointIds.end())
    {
    if(vtkDiscreteModelEdge* floatingEdge =
       vtkDiscreteModelEdge::SafeDownCast(model->GetModelEntity(vtkModelEdgeType, floatingEdgeId)))
      {
      if(static_cast<size_t>(floatingEdge->GetLineResolution()+1) == it->second.size())
        {
        return true;
        }
      }
    }
  vtkErrorMacro("A nodal group has changed since loading the bc information."
                << " The Omicron output will be reset.");
  this->Reset();
  return false;
}

//----------------------------------------------------------------------------
bool vtkModelBCGridRepresentation::IsModelFaceConsistent(
  vtkDiscreteModel* /*model*/, vtkIdType /*modelFaceId*/)
{
  return true;
}

//----------------------------------------------------------------------------
bool vtkModelBCGridRepresentation::GetModelFaceAnalysisFacets(
  vtkDiscreteModel* model, vtkIdType modelFaceId, vtkIdList* cellIds,
  vtkIdList* cellSides)
{
  cellIds->Reset();
  cellSides->Reset();
  if(this->IsModelFaceConsistent(model, modelFaceId) == false)
    {
    vtkErrorMacro("A boundary group has changed since loading the bc information."
                  << " The Omicron information will be reset.");
    this->Reset();
    return false;
    }
  vtkDiscreteModelFace* face = vtkDiscreteModelFace::SafeDownCast(
    model->GetModelEntity(vtkModelFaceType, modelFaceId));
  if(face == NULL || face->GetNumberOfCells()==0)
    {
    vtkErrorMacro("Could not find model face.");
    this->Reset();
    return false;
    }

  vtkIdTypeArray* MasterCellIds = face->GetReverseClassificationArray();
  vtkIdType numFaceCells = face->GetNumberOfCells();
  vtkIdType masterCellId;
  std::map<vtkIdType, std::set<std::pair<vtkIdType, int> > >::iterator it;
  for(vtkIdType i=0; i<numFaceCells; i++)
    {
    masterCellId = MasterCellIds->GetValue(i);
    it = this->MasterCellToMeshCellInfo.find(masterCellId);
    if(it!=this->MasterCellToMeshCellInfo.end())
      {
      for(std::set<std::pair<vtkIdType, int> >::iterator sit=it->second.begin();
        sit!=it->second.end();sit++)
        {
        cellIds->InsertNextId(sit->first);
        cellSides->InsertNextId(sit->second);
        }
      }
    else
      {
      this->Reset();
      vtkErrorMacro("Could not find model face cell id: " << masterCellId);
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
void vtkModelBCGridRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

