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
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkCmbMeshGridRepresentationServer);
vtkCxxRevisionMacro(vtkCmbMeshGridRepresentationServer, "");

//----------------------------------------------------------------------------
vtkCmbMeshGridRepresentationServer::vtkCmbMeshGridRepresentationServer():
  RepresentationBuilt(false),
  MeshServer(NULL),
  Representation(NULL)
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
bool vtkCmbMeshGridRepresentationServer::GetNodalGroupAnalysisGridPointIds(
  vtkCMBModel* model, vtkIdType nodalGroupId, vtkIdList* pointIds)
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

  this->BuildRepresentation(model);

  vtkIdTypeArray *ids = vtkIdTypeArray::SafeDownCast(
    this->Representation->GetPointData()->GetArray("ModelUseId"));
  if (!ids )
    {
    return false;
    }

  for ( vtkIdType i=0; i < ids->GetNumberOfTuples(); ++i)
    {
    if ( ids->GetValue(i) == nodalGroupId )
      {
      pointIds->InsertNextId(i);
      }
    }

  return true;
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
  this->BuildRepresentation(model);
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
  return true;
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
  //instead of doing this logic now, instead store a weak pointer to the model and mesh server
  //and than call BuildRepresentation when we are asked about any information
  this->Reset();

  this->MeshServer = meshServer;
  this->RepresentationBuilt = false;
  return this->MeshServer != NULL;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::BuildRepresentation(
  vtkCMBModel* model)
{
  if ( this->RepresentationBuilt )
    {
    return true;
    }
  //generate a single polydata that is the combintation of all the
  //face meshes
  std::vector<vtkPolyData*> faceMeshes;
  vtkSmartPointer<vtkModelItemIterator> faces;
  faces.TakeReference(model->NewIterator(vtkModelFaceType));

  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
    vtkCmbModelEntityMesh *faceEntityMesh = this->MeshServer->GetModelEntityMesh(face);
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
  clean->PointMergingOn();
  clean->ConvertLinesToPointsOff();
  clean->ConvertPolysToLinesOff();
  clean->ConvertStripsToPolysOff();

  clean->Update();
  this->Representation = vtkPolyData::New();
  this->Representation->ShallowCopy(clean->GetOutput());

  //TODO: we need to look at how we do our storage of model relationships on the mesh.
  //current it is really inefficent for lookup.

  clean->Delete();
  appender->Delete();

  this->RepresentationBuilt = true;
  return true;
}

//----------------------------------------------------------------------------
void vtkCmbMeshGridRepresentationServer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
