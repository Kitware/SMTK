/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
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

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkCmbModelEntityMesh.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkCmbMeshServer.h"
#include "vtkCMBParserBase.h"
#include "vtkMath.h"
#include "vtkModelItemIterator.h"
#include "vtkModelMaterial.h"

#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include "vtkNew.h"
#include <vtkObjectFactory.h>
#include <vtkTrivialProducer.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkTriangle.h>
#include <vtkVector.h>
#include <vtksys/SystemTools.hxx>

#include "vtkERDCMeshWriter.h"
#include "CmbFaceMeshHelper.h"

using namespace CmbFaceMesherClasses;

vtkStandardNewMacro(vtkCmbMeshGridRepresentationServer);

//----------------------------------------------------------------------------
vtkCmbMeshGridRepresentationServer::vtkCmbMeshGridRepresentationServer():
  RepresentationBuilt(false),
  Representation(NULL),
  Model(NULL)
{
}

//----------------------------------------------------------------------------
vtkCmbMeshGridRepresentationServer::~vtkCmbMeshGridRepresentationServer()
{
  this->SetRepresentation(NULL);
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetBCSNodalAnalysisGridPointIds(
  vtkDiscreteModel* model, vtkIdType bcsGroupId,
  int bcGroupType, vtkIdList* pointIds)
{
  pointIds->Reset();
  if(this->IsModelConsistent(model) == false)
    {
    this->Reset();
    return false;
    }

  if(model->HasInValidMesh())
    {  // we're on the client and don't know this info
    return false;
    }

  vtkIdTypeArray *ids = this->GetPointIdMapArray();
  if (!ids )
    {
    return false;
    }

  if(vtkDiscreteModelEntityGroup* bcsNodalGroup =
    vtkDiscreteModelEntityGroup::SafeDownCast(
    model->GetModelEntity(vtkDiscreteModelEntityGroupType, bcsGroupId)))
    {
    vtkNew<vtkIdList> vertsIdList;
    vtkNew<vtkIdList> edgesIdList;

    vtkModelItemIterator* iterEdge=bcsNodalGroup->NewIterator(vtkModelEdgeType);
    for(iterEdge->Begin();!iterEdge->IsAtEnd();iterEdge->Next())
      {
      vtkDiscreteModelEdge* entity =
        vtkDiscreteModelEdge::SafeDownCast(iterEdge->GetCurrentItem());
      if(entity)
        {
        edgesIdList->InsertUniqueId(entity->GetUniquePersistentId());
        for(int i=0; i<2; i++)
          {
          vtkDiscreteModelVertex* cmbModelVertex =
            vtkDiscreteModelVertex::SafeDownCast(entity->GetAdjacentModelVertex(i));
          if(cmbModelVertex)
            {
            vertsIdList->InsertUniqueId(cmbModelVertex->GetUniquePersistentId());
            }
          }
        }
      }
    iterEdge->Delete();
    vtkIdType entId;
    bool bAdd;
    for ( vtkIdType i=0; i < ids->GetNumberOfTuples(); ++i)
      {
      entId = ids->GetValue(i);
      if(bcGroupType == 1)// vtkSBBCInstance::enBCModelEntityAllNodesType)
        {
        bAdd = (vertsIdList->IsId(entId)>=0 || edgesIdList->IsId(entId)>=0);
        }
      else if(bcGroupType == 2)//vtkSBBCInstance::enBCModelEntityBoundaryNodesType)
        {
        bAdd = (vertsIdList->IsId(entId)>=0);
        }
      else if(bcGroupType == 3)//vtkSBBCInstance::enBCModelEntityInteriorNodesType)
        {
        bAdd = (edgesIdList->IsId(entId)>=0);
        }
      if(bAdd)
        {
        pointIds->InsertNextId(i);
        }
      }

    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetFloatingEdgeAnalysisGridPointIds(
  vtkDiscreteModel* model, vtkIdType floatingEdgeId, vtkIdList* pointIds)
{
  return false;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetModelEdgeAnalysisPoints(
  vtkDiscreteModel* model, vtkIdType edgeId, vtkIdTypeArray* edgePoints)
{
  edgePoints->Reset();
  if(model->HasInValidMesh())
    {  // we're on the client and don't know this info
    return false;
    }
  edgePoints->SetNumberOfComponents(2);

  vtkIdTypeArray *ids = this->GetCellIdMapArray();
  if (!ids )
    {
    return false;
    }

  const int indices[4] = {0,1,2,0}; //used for cell point indexes
  //we need to find organize the edge ids by connection. So in that
  //case we need to go through the cell model use ids while also iterating
  //the cell structure to find all the viable edges
  vtkIdType npts,*pts,modelIds[3],edge[2],i=0;
  if(this->Representation->GetNumberOfCells()>0)
    {
    vtkCellArray *polys = this->Representation->GetPolys();
    polys->InitTraversal();
    while(polys->GetNextCell(npts,pts))
      {
      ids->GetTupleValue(i++,modelIds);
      for (vtkIdType j=0; j < 3; j++)
        {
        if (modelIds[j] == edgeId)
          {
          edge[0] = pts[indices[j]];
          edge[1] = pts[indices[j+1]];
          edgePoints->InsertNextTupleValue(edge);
          }
        }
      }
    }
  else
    {
    vtkIdTypeArray *cellptsids = this->GetCellPointIdsArray();
    if (!cellptsids || cellptsids->GetNumberOfTuples() !=
       ids->GetNumberOfTuples())
      {
      return false;
      }
    vtkIdType pointIds[3];
    for(vtkIdType id=0; id<cellptsids->GetNumberOfTuples(); id++)
      {
      ids->GetTupleValue(id, modelIds);
      for (vtkIdType j=0; j < 3; j++)
        {
        if (modelIds[j] == edgeId)
          {
          cellptsids->GetTupleValue(id,pointIds);
          edge[0] = pointIds[indices[j]];
          edge[1] = pointIds[indices[j+1]];
          edgePoints->InsertNextTupleValue(edge);
          }
        }
      }
    }

  edgePoints->Squeeze();
  return true;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetBoundaryGroupAnalysisFacets(
  vtkDiscreteModel* model, vtkIdType boundaryGroupId,
  vtkIdList* cellIds, vtkIdList* cellSides)
{
  cellIds->Reset();
  cellSides->Reset();
  if(this->IsModelConsistent(model) == false)
    {
    this->Reset();
    return false;
    }
  if(model->HasInValidMesh())
    {  // we're on the client and don't know this info
    return false;
    }

  vtkIdTypeArray *ids = this->GetCellIdMapArray();
  if (!ids )
    {
    return false;
    }

  const int indices[4] = {0,1,2,0}; //used for cell point indexes
  vtkIdType npts,*pts,modelIds[3],i=0;
  vtkCellArray *polys = this->Representation->GetPolys();

  if(vtkDiscreteModelEntityGroup* boundaryGroup =
    vtkDiscreteModelEntityGroup::SafeDownCast(
    model->GetModelEntity(vtkDiscreteModelEntityGroupType, boundaryGroupId)))
    {
    vtkModelItemIterator* entities = boundaryGroup->NewModelEntityIterator();
    for(entities->Begin();!entities->IsAtEnd();entities->Next())
      {
      vtkModelEntity *entity = vtkModelEntity::SafeDownCast(entities->GetCurrentItem());
      vtkIdType id = entity->GetUniquePersistentId();
      polys->InitTraversal();
      while(polys->GetNextCell(npts,pts))
        {
        ids->GetTupleValue(i++,modelIds);
        for (vtkIdType j=0; j < 3; j++)
          {
          if (modelIds[j] == id)
            {
            cellIds->InsertNextId(i);
            cellSides->InsertNextId(j);
            }
          }
        }
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::IsModelConsistent(vtkDiscreteModel* model)
{
  return (this->Model != NULL  &&
          this->Model == model &&
          this->RepresentationBuilt);
}

//----------------------------------------------------------------------------
void vtkCmbMeshGridRepresentationServer::Reset()
{
  this->Superclass::Reset();
  if ( this->Representation )
    {
    this->Representation->Delete();
    this->Representation = NULL;
    }
  this->RepresentationBuilt = false;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::Initialize(
  vtkCmbMeshServer *meshServer)
{
  //we build the mesh on init time as we need to save the current mesh
  // if we wait for when a query is executed the mesh could have been
  //regenerated and become invalid.
  this->Reset();
  return this->BuildRepresentation(meshServer);
}
//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::Initialize(
vtkPolyData* meshRepresentation, vtkDiscreteModel* model)
{
  this->SetRepresentation(meshRepresentation);
  this->Model = model;
  return this->RepresentationBuilt;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::BuildRepresentation(
  vtkCmbMeshServer *meshServer)
{
  //TODO: we need to look at how we do our storage of model relationships on the mesh.
  //current it is really inefficent for lookup.
  if ( this->RepresentationBuilt )
    {
    return true;
    }
  this->Model = vtkDiscreteModel::SafeDownCast(meshServer->GetModel());
  if (!this->Model)
    {
    return false;
    }

  //generate a single polydata that is the combintation of all the
  //face meshes
  std::vector<vtkPolyData*> faceMeshes;
  vtkSmartPointer<vtkModelItemIterator> faces;
  faces.TakeReference(this->Model->NewIterator(vtkModelFaceType));

  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
    vtkCmbModelEntityMesh *faceEntityMesh = meshServer->GetModelEntityMesh(face);
    if ( faceEntityMesh )
      {
      vtkPolyData *faceMesh = faceEntityMesh->GetModelEntityMesh();
      if ( faceMesh )
        {
        //append this mesh together
        faceMeshes.push_back(faceMesh);
        }
      }
    }

  if ( faceMeshes.size() == 0 )
    {
    return false;
    }

  //create the single polydata now
  vtkNew<vtkAppendPolyData> appender;

  for(std::vector<vtkPolyData*>::iterator it = faceMeshes.begin();
      it != faceMeshes.end();
      it++)
    {
    appender->AddInputData(*it);
    }
  //now remove duplicate points
  vtkNew<vtkCleanPolyData> clean;
  clean->SetInputConnection(appender->GetOutputPort());
  clean->ToleranceIsAbsoluteOn();
  clean->SetTolerance(0.0);
  clean->SetAbsoluteTolerance(0.0);
  clean->PointMergingOn();
  clean->ConvertLinesToPointsOff();
  clean->ConvertPolysToLinesOff();
  clean->ConvertStripsToPolysOff();

  clean->Update();
  this->SetRepresentation(clean->GetOutput());
  return true;
}

//----------------------------------------------------------------------------
void vtkCmbMeshGridRepresentationServer::WriteMeshToFile()
{
  if (!this->RepresentationBuilt)
    {
    return;
    }

  if (this->GetGridFileName() == NULL)
    {
    return;
    }

  vtkNew<vtkTrivialProducer> tvp;
  tvp->SetOutput(this->Representation);

  vtkNew<vtkERDCMeshWriter> writer;
  writer->SetInputConnection(tvp->GetOutputPort());
  writer->SetFileName(this->GetGridFileName());

  //if the representation has a cell array that
  //identifies the model id for each cell
  if (this->Representation->GetCellData()->HasArray("ModelId") )
    {
    writer->SetInputArrayToProcess(0,0,0,
      vtkDataObject::FIELD_ASSOCIATION_CELLS,"ModelId");
    }

  writer->SetFileFormat(vtkERDCMeshWriter::XMS);
  writer->SetMeshDimension(vtkERDCMeshWriter::MESH2D);
  writer->SetValidateDimension(true);
  writer->SetWriteMetaInfo(true);
  writer->SetFloatPrecision(6);
  writer->SetUseScientificNotation(true);

  writer->Write();
}
//----------------------------------------------------------------------------
void vtkCmbMeshGridRepresentationServer::SetRepresentation (vtkPolyData* mesh)
{
  if(this->Representation == mesh)
    {
    return;
    }
  if(this->Representation)
    {
    this->Representation->UnRegister(this);
    this->Representation = NULL;
    this->RepresentationBuilt = false;
    }
  this->Representation = mesh;
  if(this->Representation)
    {
    this->Representation->Register(this);
    this->RepresentationBuilt = true;
    }
  this->Modified();
}

//----------------------------------------------------------------------------
vtkIdTypeArray* vtkCmbMeshGridRepresentationServer::GetCellIdMapArray()
{
  vtkIdTypeArray *maparray = vtkIdTypeArray::SafeDownCast(
    this->Representation->GetCellData()->GetArray(
    ModelFaceRep::Get2DAnalysisCellModelIdsString()));
  if (!maparray )
    {
    maparray = vtkIdTypeArray::SafeDownCast(
      this->Representation->GetFieldData()->GetArray(
      ModelFaceRep::Get2DAnalysisCellModelIdsString()));
    }
  return maparray;
}
//----------------------------------------------------------------------------
vtkIntArray* vtkCmbMeshGridRepresentationServer::GetCellTypeMapArray()
{
  vtkIntArray *maparray = vtkIntArray::SafeDownCast(
    this->Representation->GetCellData()->GetArray(
    ModelFaceRep::Get2DAnalysisCellModelTypesString()));
  if (!maparray )
    {
    maparray = vtkIntArray::SafeDownCast(
      this->Representation->GetFieldData()->GetArray(
      ModelFaceRep::Get2DAnalysisCellModelTypesString()));
    }
  return maparray;

}
//----------------------------------------------------------------------------
vtkIdTypeArray* vtkCmbMeshGridRepresentationServer::GetPointIdMapArray()
{
  vtkIdTypeArray *maparray = vtkIdTypeArray::SafeDownCast(
    this->Representation->GetPointData()->GetArray(
    ModelFaceRep::Get2DAnalysisPointModelIdsString()));
  if (!maparray )
    {
    maparray = vtkIdTypeArray::SafeDownCast(
      this->Representation->GetFieldData()->GetArray(
      ModelFaceRep::Get2DAnalysisPointModelIdsString()));
    }
  return maparray;

}
//----------------------------------------------------------------------------
vtkIntArray* vtkCmbMeshGridRepresentationServer::GetPointTypeMapArray()
{
  vtkIntArray *maparray = vtkIntArray::SafeDownCast(
    this->Representation->GetPointData()->GetArray(
    ModelFaceRep::Get2DAnalysisPointModelTypesString()));
  if (!maparray )
    {
    maparray = vtkIntArray::SafeDownCast(
      this->Representation->GetFieldData()->GetArray(
      ModelFaceRep::Get2DAnalysisPointModelTypesString()));
    }
  return maparray;
}
//----------------------------------------------------------------------------
vtkIdTypeArray* vtkCmbMeshGridRepresentationServer::GetCellPointIdsArray()
{
  vtkIdTypeArray *cellptsarray = vtkIdTypeArray::SafeDownCast(
    this->Representation->GetFieldData()->GetArray(
    ModelFaceRep::Get2DAnalysisCellPointIdsString()));
  return cellptsarray;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::CanProcessModelGroup(
  vtkDiscreteModel* model, int groupId, vtkIdTypeArray *maparray,
  std::set<vtkIdType>& faceIdList)
{
  if(this->IsModelConsistent(model) == false)
    {
    this->Reset();
    return false;
    }

  if(model->HasInValidMesh())
    {  // we're on the client and don't know this info
    return false;
    }
  // this method is meant for 2d models
  if(model->GetModelDimension() == 3)
    {
    return false;
    }

  // if the representation has a cell array that
  // identifies the model id for each cell. Normally this array should
  // be there when we have a generated 2d mesh.
  // See ModelFaceRep::RelateMeshToModel()
  if (this->Representation->GetCellData()->HasArray("ModelId") )
    {
    maparray = vtkIdTypeArray::SafeDownCast(
      this->Representation->GetCellData()->GetArray("ModelId"));
    }
  if (!maparray )
    {
    return false;
    }
  vtkModelEntity* entityGroup = model->GetModelEntity(groupId);
  if(!entityGroup)
    {
    return false;
    }

  // we are expecting only model faces
  vtkModelItemIterator* iterFace=entityGroup->NewIterator(vtkModelFaceType);
  for(iterFace->Begin();!iterFace->IsAtEnd();iterFace->Next())
    {
    vtkDiscreteModelFace* entity =
      vtkDiscreteModelFace::SafeDownCast(iterFace->GetCurrentItem());
    if(entity)
      {
      faceIdList.insert(entity->GetUniquePersistentId());
      }
    }
  iterFace->Delete();
  if(faceIdList.size() == 0)
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetGroupFacetIds(
  vtkDiscreteModel* model, int groupId, std::vector<int>& cellIds)
{
  vtkIdTypeArray* maparray = NULL;
  std::set<vtkIdType> faceIdList;
  if(!this->CanProcessModelGroup(model, groupId, maparray, faceIdList))
    {
    return false;
    }

  vtkIdType length = maparray->GetNumberOfComponents() * maparray->GetNumberOfTuples();
  vtkIdType *idBuffer = reinterpret_cast<vtkIdType *>(
    maparray->GetVoidPointer(0));
  cellIds.clear();
  vtkIdType fid;
  for(vtkIdType i = 0; i < length; ++i, ++ idBuffer)
    {
    fid = *idBuffer;
    if(faceIdList.find(fid) != faceIdList.end())
      {
      cellIds.push_back(i);
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetGroupFacetsArea(
  vtkDiscreteModel* model, int groupId, double& area)
{
  vtkIdTypeArray* maparray = NULL;
  std::set<vtkIdType> faceIdList;
  if(!this->CanProcessModelGroup(model, groupId, maparray, faceIdList))
    {
    return false;
    }

  area = 0.0;
  vtkIdType length = maparray->GetNumberOfComponents() * maparray->GetNumberOfTuples();
  vtkIdType *idBuffer = reinterpret_cast<vtkIdType *>(
    maparray->GetVoidPointer(0));
  vtkIdType fid;
  for(vtkIdType tId = 0; tId < length; ++tId, ++ idBuffer)
    {
    fid = *idBuffer;
    if(faceIdList.find(fid) != faceIdList.end())
      {
      vtkTriangle *t = vtkTriangle::SafeDownCast(
        this->Representation->GetCell(tId));
      area += t ? t->ComputeArea() : 0;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetAgentsInGroupDomain(
  vtkDiscreteModel* model,int groupId, int numberOfAgents,
  std::vector<std::pair<int, std::pair<double, double> > >& locations)
{
  vtkIdTypeArray* maparray = NULL;
  std::set<vtkIdType> faceIdList;
  if(!this->CanProcessModelGroup(model, groupId, maparray, faceIdList))
    {
    return false;
    }

  vtkIdType numCells = this->Representation->GetNumberOfCells();
  int count = 0;
  vtkIdType cellId, fid;
  double p0[3], p1[3], p2[3];
  double r, s, ab[3], ac[3];
  locations.clear();

  while(count < numberOfAgents)
    {
    cellId = (vtkIdType)vtkMath::Random(0, (double)(numCells-1));
    fid = maparray->GetValue(cellId);
    if(faceIdList.find(fid) != faceIdList.end())
      {
      vtkTriangle *t = vtkTriangle::SafeDownCast(
        this->Representation->GetCell(cellId));
      if(t)
        {
        // I am using triangle center for performance, but we can certainly
        // do some random generation with the three points to get a point.
        t->GetPoints()->GetPoint(0, p0);
        t->GetPoints()->GetPoint(1, p1);
        t->GetPoints()->GetPoint(2, p2);

        vtkMath::Subtract(p1, p0, ab);
        vtkMath::Subtract(p2, p0, ac);
        r = vtkMath::Random(0.0, 1.0);       //  % along ab
        s = vtkMath::Random(0.0, 1.0);       //  % along ac

        if (r + s >= 1)
          {
          r = 1 - r;
          s = 1 - s;
          }

        std::pair<double, double> posXY;
        //  Now add the two weighted vectors to p0
        posXY.first = p0[0] + ((ab[0] * r) + (ac[1] * s));
        posXY.second = p0[1] + ((ab[1] * r) + (ac[1] * s));
/*
        vtkTriangle::TriangleCenter(p0, p1, p2, tcenter);
        posXY.first = tcenter[0];
        posXY.second = tcenter[1];
*/

        count++;
        locations.push_back(
          std::make_pair<int, std::pair<double, double> >(cellId, posXY));
        }
      }
    }

  return true;
}
//----------------------------------------------------------------------------
void vtkCmbMeshGridRepresentationServer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
