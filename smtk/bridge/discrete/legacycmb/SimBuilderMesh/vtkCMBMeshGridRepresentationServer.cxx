//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBMeshGridRepresentationServer.h"

#include <iostream>
#include <string>

#include "vtkCMBMeshServer.h"
#include "vtkCMBModelEntityMesh.h"
#include "vtkCMBParserBase.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkMath.h"
#include "vtkModelItemIterator.h"
#include "vtkModelMaterial.h"

#include "vtkNew.h"
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTriangle.h>
#include <vtkTrivialProducer.h>
#include <vtkVector.h>
#include <vtksys/SystemTools.hxx>

#include "cmbFaceMeshHelper.h"
#include "vtkCMBMeshWriter.h"

using namespace discreteFaceMesherClasses;

vtkStandardNewMacro(vtkCMBMeshGridRepresentationServer);

//----------------------------------------------------------------------------
vtkCMBMeshGridRepresentationServer::vtkCMBMeshGridRepresentationServer():
  RepresentationBuilt(false),
  Representation(NULL),
  Model(NULL)
{
}

//----------------------------------------------------------------------------
vtkCMBMeshGridRepresentationServer::~vtkCMBMeshGridRepresentationServer()
{
  this->SetRepresentation(NULL);
}

//----------------------------------------------------------------------------
bool vtkCMBMeshGridRepresentationServer::GetBCSNodalAnalysisGridPointIds(
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
    bool bAdd = false;
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
bool vtkCMBMeshGridRepresentationServer::GetFloatingEdgeAnalysisGridPointIds(
  vtkDiscreteModel* /* model */, vtkIdType /* floatingEdgeId */, vtkIdList* /* pointIds */)
{
  return false;
}

//----------------------------------------------------------------------------
bool vtkCMBMeshGridRepresentationServer::GetModelEdgeAnalysisPoints(
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
      ids->GetTypedTuple(i++,modelIds);
      for (vtkIdType j=0; j < 3; j++)
        {
        if (modelIds[j] == edgeId)
          {
          edge[0] = pts[indices[j]];
          edge[1] = pts[indices[j+1]];
          edgePoints->InsertNextTypedTuple(edge);
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
      ids->GetTypedTuple(id, modelIds);
      for (vtkIdType j=0; j < 3; j++)
        {
        if (modelIds[j] == edgeId)
          {
          cellptsids->GetTypedTuple(id,pointIds);
          edge[0] = pointIds[indices[j]];
          edge[1] = pointIds[indices[j+1]];
          edgePoints->InsertNextTypedTuple(edge);
          }
        }
      }
    }

  edgePoints->Squeeze();
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBMeshGridRepresentationServer::GetBoundaryGroupAnalysisFacets(
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

  vtkIdType npts,*pts,modelIds[3];
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
      polys->InitTraversal(); //needs to be reset for each entities
      for(vtkIdType i=0; polys->GetNextCell(npts,pts); ++i)
        {
        ids->GetTypedTuple(i,modelIds);
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
bool vtkCMBMeshGridRepresentationServer::IsModelConsistent(vtkDiscreteModel* model)
{
  return (this->Model != NULL  &&
          this->Model == model &&
          this->RepresentationBuilt);
}

//----------------------------------------------------------------------------
void vtkCMBMeshGridRepresentationServer::Reset()
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
bool vtkCMBMeshGridRepresentationServer::Initialize(
  vtkCMBMeshServer *meshServer)
{
  //we build the mesh on init time as we need to save the current mesh
  // if we wait for when a query is executed the mesh could have been
  //regenerated and become invalid.
  this->Reset();
  return this->BuildRepresentation(meshServer);
}
//----------------------------------------------------------------------------
bool vtkCMBMeshGridRepresentationServer::Initialize(
vtkPolyData* meshRepresentation, vtkDiscreteModel* model)
{
  this->SetRepresentation(meshRepresentation);
  this->Model = model;
  return this->RepresentationBuilt;
}

//----------------------------------------------------------------------------
bool vtkCMBMeshGridRepresentationServer::BuildRepresentation(
  vtkCMBMeshServer *meshServer)
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
    vtkCMBModelEntityMesh *faceEntityMesh = meshServer->GetModelEntityMesh(face);
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
void vtkCMBMeshGridRepresentationServer::WriteMeshToFile()
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

  vtkNew<vtkCMBMeshWriter> writer;
  writer->SetInputConnection(tvp->GetOutputPort());
  writer->SetFileName(this->GetGridFileName());

  //if the representation has a cell array that
  //identifies the model id for each cell
  if (this->Representation->GetCellData()->HasArray("ModelId") )
    {
    writer->SetInputArrayToProcess(0,0,0,
      vtkDataObject::FIELD_ASSOCIATION_CELLS,"ModelId");
    }

  writer->SetFileFormat(vtkCMBMeshWriter::XMS);
  writer->SetMeshDimension(vtkCMBMeshWriter::MESH2D);
  writer->SetValidateDimension(true);
  writer->SetWriteMetaInfo(true);
  writer->SetFloatPrecision(6);
  writer->SetUseScientificNotation(true);

  writer->Write();
}
//----------------------------------------------------------------------------
void vtkCMBMeshGridRepresentationServer::SetRepresentation (vtkPolyData* mesh)
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
vtkIdTypeArray* vtkCMBMeshGridRepresentationServer::GetCellIdMapArray()
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
vtkIntArray* vtkCMBMeshGridRepresentationServer::GetCellTypeMapArray()
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
vtkIdTypeArray* vtkCMBMeshGridRepresentationServer::GetPointIdMapArray()
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
vtkIntArray* vtkCMBMeshGridRepresentationServer::GetPointTypeMapArray()
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
vtkIdTypeArray* vtkCMBMeshGridRepresentationServer::GetCellPointIdsArray()
{
  vtkIdTypeArray *cellptsarray = vtkIdTypeArray::SafeDownCast(
    this->Representation->GetFieldData()->GetArray(
    ModelFaceRep::Get2DAnalysisCellPointIdsString()));
  return cellptsarray;
}

//----------------------------------------------------------------------------
bool vtkCMBMeshGridRepresentationServer::CanProcessModelGroup(
  vtkDiscreteModel* model, int groupId, std::set<vtkIdType>& faceIdList)
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
  if (!this->Representation->GetCellData()->HasArray("ModelId") ||
     !vtkIdTypeArray::SafeDownCast(this->Representation->GetCellData()->GetArray("ModelId")))
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
bool vtkCMBMeshGridRepresentationServer::GetGroupFacetIds(
  vtkDiscreteModel* model,int groupId, std::vector<int>& cellIds)
{
  std::set<vtkIdType> faceIdList;
  if(!this->CanProcessModelGroup(model, groupId, faceIdList))
    {
    return false;
    }
  vtkIdTypeArray* maparray = vtkIdTypeArray::SafeDownCast(
      this->Representation->GetCellData()->GetArray("ModelId"));

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
bool vtkCMBMeshGridRepresentationServer::GetGroupFacetsArea(
  vtkDiscreteModel* model,int groupId, double& area)
{
  std::set<vtkIdType> faceIdList;
  if(!this->CanProcessModelGroup(model, groupId, faceIdList))
    {
    return false;
    }
  vtkIdTypeArray* maparray = vtkIdTypeArray::SafeDownCast(
      this->Representation->GetCellData()->GetArray("ModelId"));

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
bool vtkCMBMeshGridRepresentationServer::GetCellPointIds(
  int cellId, std::vector<int>& pointIds)
{
  vtkIdType cId = static_cast<vtkIdType>(cellId);
  if(cId < 0 || cId >= this->Representation->GetNumberOfCells())
    {
    pointIds.clear();
    vtkWarningMacro("Bad cell id");
    return false;
    }
  vtkNew<vtkIdList> ids;
  this->Representation->GetCellPoints(cId, ids.GetPointer());
  pointIds.resize(ids->GetNumberOfIds());
  for(vtkIdType i=0;i<ids->GetNumberOfIds();i++)
    {
    pointIds[i] = static_cast<int>(ids->GetId(i));
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBMeshGridRepresentationServer::GetPointLocation(
  int pointId, std::vector<double>& coords)
{
  vtkIdType pId = static_cast<vtkIdType>(pointId);
  if(pId < 0 || pId >= this->Representation->GetNumberOfPoints())
    {
    coords.clear();
    vtkWarningMacro("Bad point id");
    return false;
    }
  coords.resize(3);
  this->Representation->GetPoint(pId, &coords[0]);
  return true;
}

//----------------------------------------------------------------------------
void vtkCMBMeshGridRepresentationServer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
