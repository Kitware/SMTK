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
#include "vtkCmbMeshGridRepresentationServer.h"

#include <iostream>
#include <string>
#include "vtkCMBModel.h"
#include "vtkCMBModelEdge.h"
#include "vtkCMBModelEntityGroup.h"
#include "vtkCMBModelFace.h"
#include "vtkCMBModelGeometricEntity.h"
#include "vtkCMBNodalGroup.h"
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include "vtkModelItemIterator.h"
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkCmbMeshGridRepresentationServer);
vtkCxxRevisionMacro(vtkCmbMeshGridRepresentationServer, "");

//----------------------------------------------------------------------------
vtkCmbMeshGridRepresentationServer::vtkCmbMeshGridRepresentationServer()
{
}

//----------------------------------------------------------------------------
vtkCmbMeshGridRepresentationServer::~vtkCmbMeshGridRepresentationServer()
{
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetNodalGroupAnalysisGridPointIds(
  vtkCMBModel* model, vtkIdType nodalGroupId, vtkIdList* pointIds)
{
  pointIds->Reset();
  if(vtkCMBNodalGroup* nodalGroup =
     vtkCMBNodalGroup::SafeDownCast(model->GetModelEntity(vtkCMBNodalGroupType, nodalGroupId)))
    {
    std::map<vtkIdType, std::set<vtkIdType> >::iterator it=
      this->NodalGroupToPointIds.find(nodalGroupId);
    if(it!=this->NodalGroupToPointIds.end())
      {
      if(static_cast<size_t>(nodalGroup->GetNumberOfPointIds()) == it->second.size())
        {
        pointIds->SetNumberOfIds(nodalGroup->GetNumberOfPointIds());
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
    vtkErrorMacro("A nodal group has changed since loading the bc information."
                  << " The Omicron information will be reset.");
    this->Reset();
    }

  pointIds->Reset();
  return false;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetFloatingEdgeAnalysisGridPointIds(
  vtkCMBModel* model, vtkIdType floatingEdgeId, vtkIdList* pointIds)
{
  pointIds->Reset();
  if(vtkCMBModelEdge* floatingEdge =
     vtkCMBModelEdge::SafeDownCast(model->GetModelEntity(vtkModelEdgeType, floatingEdgeId)))
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
bool vtkCmbMeshGridRepresentationServer::GetModelEdgeAnalysisPoints(
  vtkCMBModel* model, vtkIdType boundaryGroupId, vtkIdTypeArray* edgePoints)
{
  vtkErrorMacro("2D models not supported.");
  return false;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetBoundaryGroupAnalysisFacets(
  vtkCMBModel* model, vtkIdType boundaryGroupId,
  vtkIdList* cellIds, vtkIdList* cellSides)
{
  cellIds->Reset();
  cellSides->Reset();
  vtkCMBModelEntityGroup* boundaryGroup =
    vtkCMBModelEntityGroup::SafeDownCast(
      model->GetModelEntity(vtkCMBModelEntityGroupType, boundaryGroupId));
  if(!boundaryGroup)
    {
    vtkWarningMacro("Bad boundary group id.");
    return false;
    }
  vtkSmartPointer<vtkModelItemIterator> faces;
  faces.TakeReference(boundaryGroup->NewIterator(vtkModelFaceType));
  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkCMBModelFace* face =
      vtkCMBModelFace::SafeDownCast(faces->GetCurrentItem());
    if(this->IsModelFaceConsistent(model, face->GetUniquePersistentId()) == false)
      {
      vtkErrorMacro("A boundary group has changed since loading the bc information."
                    << " The Omicron information will be reset.");
      this->Reset();
      cellIds->Reset();
      cellSides->Reset();
      return false;
      }
    std::map<vtkIdType, std::set<std::pair<vtkIdType, int> > >::iterator it=
      this->ModelFaceToFacetInfo.find(face->GetUniquePersistentId());
    if(it!=this->ModelFaceToFacetInfo.end())
      {
      for(std::set<std::pair<vtkIdType, int> >::iterator sit=it->second.begin();
          sit!=it->second.end();sit++)
        {
        cellIds->InsertNextId(sit->first);
        cellSides->InsertNextId(sit->second);
        }
      }
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::IsModelConsistent(vtkCMBModel* model)
{
// now go through and make sure the BC file information matches with model
  size_t number = model->GetNumberOfAssociations(vtkCMBNodalGroupType);
  if(number != this->NodalGroupToPointIds.size())
    {
    vtkErrorMacro("There are " << number << " nodal groups but the bc file has "
                  << this->NodalGroupToPointIds.size());
    this->Reset();
    return false;
    }
  // get the floating edges of the 3D model
  std::set<vtkIdType> floatingEdgeIds;
  vtkModelItemIterator* edges = model->NewIterator(vtkModelEdgeType);
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkCMBModelEdge* edge = vtkCMBModelEdge::SafeDownCast(
      edges->GetCurrentItem());
    if(edge->GetModelRegion())
      {
      floatingEdgeIds.insert(edge->GetUniquePersistentId());
      }
    }
  edges->Delete();
  number = floatingEdgeIds.size();
  if(number != this->FloatingEdgeToPointIds.size())
    {
    vtkErrorMacro("There are " << number << " floating edges but the bc file has "
                  << this->FloatingEdgeToPointIds.size());
    this->Reset();
    return false;
    }
  number = model->GetNumberOfAssociations(vtkModelFaceType);
  if(number != this->ModelFaceToFacetInfo.size())
    {
    vtkErrorMacro("There are " << number << " model faces but the bc file has "
                  << this->ModelFaceToFacetInfo.size() << " boundary model faces.");
    this->Reset();
    return false;
    }

  for(std::map<vtkIdType, std::set<vtkIdType> >::iterator it=this->NodalGroupToPointIds.begin();
      it!=this->NodalGroupToPointIds.end();it++)
    {
    if(this->IsNodalGroupConsistent(model, it->first) == false)
      {
      this->Reset();
      return false;
      }
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
        this->ModelFaceToFacetInfo.begin();
      it!=this->ModelFaceToFacetInfo.end();it++)
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
bool vtkCmbMeshGridRepresentationServer::Initialize(
  const char* bcFileName, vtkCMBModel* model)
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
    vtkCMBModelEdge* edge = vtkCMBModelEdge::SafeDownCast(
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
      if(vtkCMBNodalGroup* nodalGroup =
         vtkCMBNodalGroup::SafeDownCast(model->GetModelEntity(vtkCMBNodalGroupType, entityId)))
        {
        this->NodalGroupToPointIds[entityId].insert(pointId);
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
    else if(vtksys::SystemTools::Strucmp(values[0].c_str(), "FCS") == 0 &&
            values.size() == 7)
      {
      vtkIdType cellId = atoi(values[1].c_str())-1; // analysis grid cell Id in C++ ordering
      int cellSide = atoi(values[2].c_str())-1; // analysis grid cell side (0-3)
      vtkIdType modelFaceId = atoi(values[3].c_str());
      this->ModelFaceToFacetInfo[modelFaceId].insert(std::make_pair<vtkIdType, int>(cellId, cellSide));
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
  std::string gridName = vtksys::SystemTools::GetFilenameName(bcFileName);
  gridName = vtksys::SystemTools::GetFilenameWithoutExtension(gridName);
  gridName.append(".3dm");
  this->SetGridFileName(gridName.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::AddNodalGroup(
  vtkIdType nodalGroupId, vtkIdList* pointIds, vtkCMBModel* model)
{
  std::map<vtkIdType, std::set<vtkIdType> >::iterator it=
    this->NodalGroupToPointIds.find(nodalGroupId);
  if(it==this->NodalGroupToPointIds.end())
    {
    it = this->NodalGroupToPointIds.insert(
      this->NodalGroupToPointIds.begin(),
      std::make_pair(nodalGroupId, std::set<vtkIdType>()));
    }

  for(vtkIdType i=0;i<pointIds->GetNumberOfIds();i++)
    {
    it->second.insert(pointIds->GetId(i));
    }

  if(IsNodalGroupConsistent(model, nodalGroupId) == true)
    {
    return true;
    }
  this->Reset();
  return false;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::AddFloatingEdge(
  vtkIdType floatingEdgeId, vtkIdList* pointIds, vtkCMBModel* model)
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
bool vtkCmbMeshGridRepresentationServer::AddModelFace(
  vtkIdType modelFaceId, vtkIdList* cellIds,
  vtkIdList* cellSides, vtkCMBModel* model)
{
  std::map<vtkIdType, std::set<std::pair<vtkIdType, int> > >::iterator it=
    this->ModelFaceToFacetInfo.find(modelFaceId);
  if(it==this->ModelFaceToFacetInfo.end())
    {
    it = this->ModelFaceToFacetInfo.insert(
      this->ModelFaceToFacetInfo.begin(),
      std::make_pair(modelFaceId, std::set<std::pair<vtkIdType, int> >()));
    }

  for(vtkIdType i=0;i<cellIds->GetNumberOfIds();i++)
    {
    it->second.insert(std::make_pair(cellIds->GetId(i), static_cast<int>(cellSides->GetId(i))));
    }

  if(IsModelFaceConsistent(model, modelFaceId) == true)
    {
    return true;
    }
  this->Reset();
  return false;
}

//----------------------------------------------------------------------------
void vtkCmbMeshGridRepresentationServer::Reset()
{
  this->Superclass::Reset();
  this->NodalGroupToPointIds.clear();
  this->FloatingEdgeToPointIds.clear();
  this->ModelFaceToFacetInfo.clear();
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::IsNodalGroupConsistent(
  vtkCMBModel* model, vtkIdType nodalGroupId)
{
  std::map<vtkIdType, std::set<vtkIdType> >::iterator it=
    this->NodalGroupToPointIds.find(nodalGroupId);
  if(it!=this->NodalGroupToPointIds.end())
    {
    if(vtkCMBNodalGroup* nodalGroup =
       vtkCMBNodalGroup::SafeDownCast(model->GetModelEntity(vtkCMBNodalGroupType, nodalGroupId)))
      {
      if(static_cast<size_t>(nodalGroup->GetNumberOfPointIds()) == it->second.size())
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
bool vtkCmbMeshGridRepresentationServer::IsFloatingEdgeConsistent(
  vtkCMBModel* model, vtkIdType floatingEdgeId)
{
  std::map<vtkIdType, std::set<vtkIdType> >::iterator it=
    this->FloatingEdgeToPointIds.find(floatingEdgeId);
  if(it!=this->FloatingEdgeToPointIds.end())
    {
    if(vtkCMBModelEdge* floatingEdge =
       vtkCMBModelEdge::SafeDownCast(model->GetModelEntity(vtkModelEdgeType, floatingEdgeId)))
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
bool vtkCmbMeshGridRepresentationServer::IsModelFaceConsistent(
  vtkCMBModel* model, vtkIdType modelFaceId)
{
  std::map<vtkIdType, std::set<std::pair<vtkIdType, int> > >::iterator it=
    this->ModelFaceToFacetInfo.find(modelFaceId);
  if(it!=this->ModelFaceToFacetInfo.end())
    {
    if(vtkCMBModelFace* modelFace =
       vtkCMBModelFace::SafeDownCast(model->GetModelEntity(vtkModelFaceType, modelFaceId)))
      {
      int numberOfRegions = modelFace->GetNumberOfModelRegions();
      if(static_cast<size_t>(modelFace->GetNumberOfCells()) ==
         it->second.size() && numberOfRegions < 2)
        {
        return true;
        }
      else if(numberOfRegions == 2 && it->second.size() ==
              static_cast<size_t>(modelFace->GetNumberOfCells() * 2) )
        {
        return true;
        }
      }
    }

  this->Reset();
  vtkErrorMacro("A boundary group has changed since loading the bc information."
                << " The Omicron output will be reset.");
  return false;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetModelFaceAnalysisFacets(
  vtkCMBModel* model, vtkIdType modelFaceId, vtkIdList* cellIds,
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
  vtkCMBModelFace* face = vtkCMBModelFace::SafeDownCast(
    model->GetModelEntity(vtkModelFaceType, modelFaceId));
  if(face == NULL)
    {
    vtkErrorMacro("Could not find model face.");
    return false;
    }
  std::map<vtkIdType, std::set<std::pair<vtkIdType, int> > >::iterator it=
    this->ModelFaceToFacetInfo.find(modelFaceId);
  if(it!=this->ModelFaceToFacetInfo.end())
    {
    for(std::set<std::pair<vtkIdType, int> >::iterator sit=it->second.begin();
        sit!=it->second.end();sit++)
      {
      cellIds->InsertNextId(sit->first);
      cellSides->InsertNextId(sit->second);
      }
    return true;
    }
  this->Reset();
  vtkErrorMacro("Could not find model face boundary info.");
  return false;
}

//----------------------------------------------------------------------------
void vtkCmbMeshGridRepresentationServer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
