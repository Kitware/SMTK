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
#include "vtkCmbModelEntityMesh.h"
#include "vtkCMBModelEntityGroup.h"
#include "vtkCMBModelFace.h"
#include "vtkCMBModelGeometricEntity.h"
#include "vtkCMBModelVertex.h"
#include "vtkCMBNodalGroup.h"
#include "vtkCmbMeshServer.h"
#include "vtkModelItemIterator.h"

#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>
#include <vtkTrivialProducer.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>
#include "vtkNew.h"

#include "vtkGMSMesh2DWriter.h"

vtkStandardNewMacro(vtkCmbMeshGridRepresentationServer);
vtkCxxRevisionMacro(vtkCmbMeshGridRepresentationServer, "");

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
  if ( this->Representation )
    {
    this->Representation->Delete();
    }
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetBCSNodalAnalysisGridPointIds(
  vtkCMBModel* model, vtkIdType bcsGroupId,
  int bcGroupType, vtkIdList* pointIds)
{
  pointIds->Reset();
  if(this->IsModelConsistent(model) == false)
    {
    this->Reset();
    return false;
    }

  if(vtkPolyData::SafeDownCast(model->GetGeometry()) == NULL)
    {  // we're on the client and don't know this info
    return false;
    }

  vtkIdTypeArray *ids = vtkIdTypeArray::SafeDownCast(
    this->Representation->GetPointData()->GetArray("ModelUseId"));
  if (!ids )
    {
    return false;
    }

  if(vtkCMBModelEntityGroup* bcsNodalGroup =
    vtkCMBModelEntityGroup::SafeDownCast(
    model->GetModelEntity(vtkCMBModelEntityGroupType, bcsGroupId)))
    {
    vtkNew<vtkIdList> vertsIdList;
    vtkNew<vtkIdList> edgesIdList;

    vtkModelItemIterator* iterEdge=bcsNodalGroup->NewIterator(vtkModelEdgeType);
    for(iterEdge->Begin();!iterEdge->IsAtEnd();iterEdge->Next())
      {
      vtkCMBModelEdge* entity =
        vtkCMBModelEdge::SafeDownCast(iterEdge->GetCurrentItem());
      if(entity)
        {
        edgesIdList->InsertUniqueId(entity->GetUniquePersistentId());
        for(int i=0; i<2; i++)
          {
          vtkCMBModelVertex* cmbModelVertex =
            vtkCMBModelVertex::SafeDownCast(entity->GetAdjacentModelVertex(i));
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
  vtkCMBModel* model, vtkIdType floatingEdgeId, vtkIdList* pointIds)
{
  return false;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetModelEdgeAnalysisPoints(
  vtkCMBModel* model, vtkIdType edgeId, vtkIdTypeArray* edgePoints)
{

  edgePoints->Reset();
  if(vtkPolyData::SafeDownCast(model->GetGeometry()) == NULL)
    {  // we're on the client and don't know this info
    return false;
    }
  edgePoints->SetNumberOfComponents(2);

  vtkIdTypeArray *ids = vtkIdTypeArray::SafeDownCast(
    this->Representation->GetCellData()->GetArray("ModelUseId"));
  if (!ids )
    {
    return false;
    }

  const int indices[4] = {0,1,2,0}; //used for cell point indexes
  //we need to find organize the edge ids by connection. So in that
  //case we need to go through the cell model use ids while also iterating
  //the cell structure to find all the viable edges
  vtkIdType npts,*pts,modelIds[3],edge[2],i=0;
  vtkCellArray *polys = this->Representation->GetPolys();
  polys->InitTraversal();
  while(polys->GetNextCell(npts,pts) != NULL)
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

  edgePoints->Squeeze();
  return true;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetBoundaryGroupAnalysisFacets(
  vtkCMBModel* model, vtkIdType boundaryGroupId,
  vtkIdList* cellIds, vtkIdList* cellSides)
{
  cellIds->Reset();
  cellSides->Reset();
  if(this->IsModelConsistent(model) == false)
    {
    this->Reset();
    return false;
    }
  if(vtkPolyData::SafeDownCast(model->GetGeometry()) == NULL)
    {  // we're on the client and don't know this info
    return false;
    }

  vtkIdTypeArray *ids = vtkIdTypeArray::SafeDownCast(
    this->Representation->GetCellData()->GetArray("ModelUseId"));
  if (!ids )
    {
    return false;
    }

  const int indices[4] = {0,1,2,0}; //used for cell point indexes
  vtkIdType npts,*pts,modelIds[3],i=0;
  vtkCellArray *polys = this->Representation->GetPolys();

  if(vtkCMBModelEntityGroup* boundaryGroup =
    vtkCMBModelEntityGroup::SafeDownCast(
    model->GetModelEntity(vtkCMBModelEntityGroupType, boundaryGroupId)))
    {
    vtkModelItemIterator* entities = boundaryGroup->NewModelEntityIterator();
    for(entities->Begin();!entities->IsAtEnd();entities->Next())
      {
      vtkModelEntity *entity = vtkModelEntity::SafeDownCast(entities->GetCurrentItem());
      vtkIdType id = entity->GetUniquePersistentId();
      polys->InitTraversal();
      while(polys->GetNextCell(npts,pts) != NULL)
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
bool vtkCmbMeshGridRepresentationServer::IsModelConsistent(vtkCMBModel* model)
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
bool vtkCmbMeshGridRepresentationServer::BuildRepresentation(
  vtkCmbMeshServer *meshServer)
{

  //TODO: we need to look at how we do our storage of model relationships on the mesh.
  //current it is really inefficent for lookup.
  if ( this->RepresentationBuilt )
    {
    return true;
    }
  this->Model = vtkCMBModel::SafeDownCast(meshServer->GetModel());
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
  vtkAppendPolyData *appender = vtkAppendPolyData::New();

  for(std::vector<vtkPolyData*>::iterator it = faceMeshes.begin();
      it != faceMeshes.end();
      it++)
    {
    appender->AddInput(*it);
    }
  //now remove duplicate points
  vtkCleanPolyData *clean = vtkCleanPolyData::New();
  clean->SetInputConnection(appender->GetOutputPort());
  clean->ToleranceIsAbsoluteOn();
  clean->SetTolerance(0.0);
  clean->SetAbsoluteTolerance(0.0);
  clean->PointMergingOn();
  clean->ConvertLinesToPointsOff();
  clean->ConvertPolysToLinesOff();
  clean->ConvertStripsToPolysOff();

  clean->Update();
  this->Representation = vtkPolyData::New();
  this->Representation->ShallowCopy(clean->GetOutput());

  clean->Delete();
  appender->Delete();

  this->RepresentationBuilt = true;
  return true;
}

//----------------------------------------------------------------------------
void vtkCmbMeshGridRepresentationServer::WriteToFile()
{
  if (!this->RepresentationBuilt)
    {
    return;
    }

  if (this->GetGridFileName() == NULL)
    {
    return;
    }

  vtkTrivialProducer *tvp = vtkTrivialProducer::New();
  tvp->SetOutput(this->Representation);

  vtkGMSMesh2DWriter *writer = vtkGMSMesh2DWriter::New();
  writer->SetInputConnection(tvp->GetOutputPort());
  writer->SetFileName(this->GetGridFileName());

  //if the representation has a cell array that
  //identifies the model id for each cell
  if (this->Representation->GetCellData()->HasArray("ModelId") )
    {
    writer->SetInputArrayToProcess(0,0,0,
      vtkDataObject::FIELD_ASSOCIATION_CELLS,"ModelId");
    }
  writer->Write();
  writer->Delete();
  tvp->Delete();

}
//----------------------------------------------------------------------------
void vtkCmbMeshGridRepresentationServer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
