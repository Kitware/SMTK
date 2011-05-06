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
  if(model->GetModelDimension() != 2)
    {  // do nothing if it's not a 2d model
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
bool vtkCmbMeshServer::SetGlobalLength(double globalLength)
{
  if(this->GlobalLength == globalLength)
    {
    return false;
    }
  if(globalLength <= 0)
    {
    this->GlobalLength = 0.;
    return false;
    }
  this->GlobalLength = globalLength > 0. ? globalLength : 0.;
  this->Modified();
  return true;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshServer::SetGlobalMinimumAngle(double minAngle)
{
  if(this->GlobalMinimumAngle == minAngle)
    {
    return false;
    }
  if(minAngle < 0.)
    {
    this->GlobalMinimumAngle = 0;
    }
  else if(minAngle > 33.)
    {
    this->GlobalMinimumAngle = 33.;
    }
  else
    {
    this->GlobalMinimumAngle = minAngle;
    }
  this->Modified();
  return true;
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
  this->Internal->ModelEdges[createdEdge] = createdMesh;
  createdMesh->BuildModelEntityMesh(true);

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
    if(vtkCmbModelEntityMesh* faceMesh = this->GetModelEntityMesh(face))
      {
      faceMesh->BuildModelEntityMesh(false);
      }
    }
}

//----------------------------------------------------------------------------
void vtkCmbMeshServer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
