/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

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
#include "vtkCmbMeshServer.h"

#include <vtkCallbackCommand.h>
#include <vtkCMBModel.h>
#include "vtkCmbModelEdgeMeshServer.h"
#include "vtkCmbModelFaceMeshServer.h"
#include <vtkCMBModelGeometricEntity.h>
#include "vtkCmbModelVertexMesh.h"
#include <vtkIdList.h>
#include <vtkMath.h>
#include <vtkMergeEventData.h>
#include <vtkModel.h>
#include <vtkModelEdge.h>
#include <vtkModelEntity.h>
#include <vtkModelFace.h>
#include <vtkModelGeometricEntity.h>
#include <vtkModelItemIterator.h>
#include <vtkModelVertex.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkSplitEventData.h>

#include <map>

vtkStandardNewMacro(vtkCmbMeshServer);
vtkCxxRevisionMacro(vtkCmbMeshServer, "");

class vtkCmbMeshServerInternals
{
public:
  std::map<vtkModelEdge*, vtkSmartPointer<vtkCmbModelEdgeMeshServer> > ModelEdges;
  std::map<vtkModelFace*, vtkSmartPointer<vtkCmbModelFaceMeshServer> > ModelFaces;
};

//----------------------------------------------------------------------------
vtkCmbMeshServer::vtkCmbMeshServer()
{
  this->Internal = new vtkCmbMeshServerInternals;
}

//----------------------------------------------------------------------------
vtkCmbMeshServer::~vtkCmbMeshServer()
{
  if(this->Internal)
    {
    delete this->Internal;
    this->Internal = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkCmbMeshServer::Initialize(vtkModel* model)
{
  if(model == NULL)
    {
    vtkErrorMacro("Passed in NULL model.");
    return;
    }
  if(this->Model != model)
    {
    this->Reset();
    this->Model = model;
    }

  // register model modification events that we want
  // this may not be correct yet
  vtkSmartPointer<vtkCallbackCommand> callbackCommand =
    vtkSmartPointer<vtkCallbackCommand>::New();
  callbackCommand->SetCallback(vtkCmbMeshServer::ModelGeometricEntityChanged);
  callbackCommand->SetClientData((void*) this);
  model->AddObserver(ModelGeometricEntityBoundaryModified, callbackCommand);
  model->AddObserver(ModelGeometricEntityCreated, callbackCommand);
  model->AddObserver(ModelGeometricEntityAboutToDestroy, callbackCommand);
  model->AddObserver(ModelGeometricEntitiesAboutToMerge, callbackCommand);
  model->AddObserver(ModelGeometricEntitySplit, callbackCommand);

  // edges
  vtkModelItemIterator* iter = model->NewIterator(vtkModelEdgeType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelEdge* edge =
      vtkModelEdge::SafeDownCast(iter->GetCurrentItem());
    vtkSmartPointer<vtkCmbModelEdgeMeshServer> meshRepresentation =
      vtkSmartPointer<vtkCmbModelEdgeMeshServer>::New();
    meshRepresentation->Initialize(this, edge);
    this->Internal->ModelEdges[edge] = meshRepresentation;
    }
  iter->Delete();
  // faces
  iter = model->NewIterator(vtkModelFaceType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelFace* face =
      vtkModelFace::SafeDownCast(iter->GetCurrentItem());
    vtkSmartPointer<vtkCmbModelFaceMeshServer> meshRepresentation =
      vtkSmartPointer<vtkCmbModelFaceMeshServer>::New();
    meshRepresentation->Initialize(this, face);
    this->Internal->ModelFaces[face] = meshRepresentation;
    }
  iter->Delete();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMeshServer::SetGlobalLength(double globalLength)
{
  if(this->GlobalLength == globalLength)
    {
    return;
    }
  if(globalLength <= 0)
    {
    this->GlobalLength = 0.;
    return;
    }
  this->GlobalLength = globalLength > 0. ? globalLength : 0.;
  for(std::map<vtkModelEdge*, vtkSmartPointer<vtkCmbModelEdgeMeshServer> >::iterator it=
        this->Internal->ModelEdges.begin();it!=this->Internal->ModelEdges.end();
      it++)
    {
    it->second->BuildModelEntityMesh(false);
    }
  // now remesh the model faces
  for(std::map<vtkModelFace*, vtkSmartPointer<vtkCmbModelFaceMeshServer> >::iterator it=
        this->Internal->ModelFaces.begin();it!=this->Internal->ModelFaces.end();
      it++)
    {
    it->second->BuildModelEntityMesh(false);
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMeshServer::SetGlobalMaximumArea(double maxArea)
{
  if(this->GlobalMaximumArea == maxArea)
    {
    return;
    }
  this->GlobalMaximumArea = maxArea > 0. ? maxArea : 0.;
  for(std::map<vtkModelFace*, vtkSmartPointer<vtkCmbModelFaceMeshServer> >::iterator it=
        this->Internal->ModelFaces.begin();it!=this->Internal->ModelFaces.end();
      it++)
    {
    it->second->BuildModelEntityMesh(false);
    }
  // when we implement volume meshing we'll have to trigger a remesh here
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMeshServer::SetGlobalMinimumAngle(double minAngle)
{
  if(this->GlobalMinimumAngle == minAngle)
    {
    return;
    }
  this->GlobalMinimumAngle = minAngle > 0. ? minAngle : 0.;
  for(std::map<vtkModelFace*, vtkSmartPointer<vtkCmbModelFaceMeshServer> >::iterator it=
        this->Internal->ModelFaces.begin();it!=this->Internal->ModelFaces.end();
      it++)
    {
    it->second->BuildModelEntityMesh(false);
    }
  // when we implement volume meshing we'll have to trigger a remesh here
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMeshServer::Reset()
{
  this->Internal->ModelEdges.clear();
  this->Internal->ModelFaces.clear();
  this->Superclass::Reset();
  this->Modified();
}

//----------------------------------------------------------------------------
vtkCmbModelEntityMesh* vtkCmbMeshServer::GetModelEntityMesh(
  vtkModelGeometricEntity* entity)
{
  if(vtkModelEdge* modelEdge = vtkModelEdge::SafeDownCast(entity))
    {
    std::map<vtkModelEdge*,vtkSmartPointer<vtkCmbModelEdgeMeshServer> >::iterator it=
      this->Internal->ModelEdges.find(modelEdge);
    if(it == this->Internal->ModelEdges.end())
      {
      return NULL;
      }
    return it->second;
    }
  if(vtkModelFace* modelFace = vtkModelFace::SafeDownCast(entity))
    {
    std::map<vtkModelFace*,
      vtkSmartPointer<vtkCmbModelFaceMeshServer> >::iterator it=
      this->Internal->ModelFaces.find(modelFace);
    if(it == this->Internal->ModelFaces.end())
      {
      return NULL;
      }
    return it->second;
    }
  vtkErrorMacro("Incorrect type.");
  return NULL;
}

//----------------------------------------------------------------------------
void vtkCmbMeshServer::ModelEdgeSplit(vtkSplitEventData* splitEventData)
{
  vtkModelEdge* sourceEdge = vtkModelEdge::SafeDownCast(
    splitEventData->GetSourceEntity()->GetThisModelEntity());
  if(splitEventData->GetCreatedModelEntityIds()->GetNumberOfIds() != 2)
    {
    vtkGenericWarningMacro("Problem with split event.");
    return;
    }
  vtkModelEdge* createdEdge = vtkModelEdge::SafeDownCast(
    this->Model->GetModelEntity(
      vtkModelEdgeType, splitEventData->GetCreatedModelEntityIds()->GetId(0)));
  if(createdEdge == NULL)
    {
    createdEdge = vtkModelEdge::SafeDownCast(
      this->Model->GetModelEntity(
        vtkModelEdgeType, splitEventData->GetCreatedModelEntityIds()->GetId(1)));
    }
  vtkCmbModelEdgeMesh* sourceMesh = vtkCmbModelEdgeMesh::SafeDownCast(
    this->GetModelEntityMesh(sourceEdge));
  sourceMesh->BuildModelEntityMesh(false);

  vtkSmartPointer<vtkCmbModelEdgeMeshServer> createdMesh =
    vtkSmartPointer<vtkCmbModelEdgeMeshServer>::New();
  createdMesh->SetLength(sourceMesh->GetLength());
  createdMesh->Initialize(this, createdEdge);
  createdMesh->BuildModelEntityMesh(true);
  this->Internal->ModelEdges[createdEdge] = createdMesh;

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMeshServer::ModelEdgeMerge(vtkMergeEventData* mergeEventData)
{
  vtkModelEdge* sourceEdge = vtkModelEdge::SafeDownCast(
    mergeEventData->GetSourceEntity()->GetThisModelEntity());
  vtkModelEdge* targetEdge = vtkModelEdge::SafeDownCast(
    mergeEventData->GetTargetEntity()->GetThisModelEntity());
  vtkCmbModelEdgeMesh* sourceMesh = vtkCmbModelEdgeMesh::SafeDownCast(
    this->GetModelEntityMesh(sourceEdge));
  double sourceLength = sourceMesh->GetLength();
  this->Internal->ModelEdges.erase(sourceEdge);
  vtkCmbModelEdgeMesh* targetEdgeMesh = vtkCmbModelEdgeMesh::SafeDownCast(
    this->GetModelEntityMesh(targetEdge));
  if( (targetEdgeMesh->GetLength() > sourceLength && sourceLength > 0.) ||
      targetEdgeMesh->GetLength() <= 0.)
    {
    targetEdgeMesh->SetLength(sourceLength);
    }

  // we can't remesh the target edge yet since the topology hasn't changed
  // yet.  we mark it as modified so that when we see the boundary modified
  // event we will trigger the remeshing then.
  targetEdgeMesh->Modified();

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMeshServer::ModelEntityBoundaryModified(vtkModelGeometricEntity* entity)
{
  if(entity->IsA("vtkModelEdge") != 0)
    {
    if(vtkCmbModelEntityMesh* entityMesh = this->GetModelEntityMesh(entity))
      {
      entityMesh->BuildModelEntityMesh(false);
      }
    }
  else if(vtkModelFace* face = vtkModelFace::SafeDownCast(entity))
    {
    vtkModelItemIterator* edges = face->NewAdjacentModelEdgeIterator();
    for(edges->Begin();!edges->IsAtEnd();edges->Next())
      {
      if(vtkCmbModelEntityMesh* entityMesh =
         this->GetModelEntityMesh(vtkModelEdge::SafeDownCast(edges->GetCurrentItem())))
        {
        entityMesh->BuildModelEntityMesh(false);
        }
      }
    edges->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkCmbMeshServer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
