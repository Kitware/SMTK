//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBMeshServer.h"

#include <vtkCallbackCommand.h>
#include <vtkDiscreteModel.h>
#include "vtkCMBModelEdgeMeshServer.h"
#include "vtkCMBModelFaceMeshServer.h"
#include <vtkDiscreteModelGeometricEntity.h>
#include "vtkCMBModelVertexMesh.h"
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

vtkStandardNewMacro(vtkCMBMeshServer);

class vtkCMBMeshServerInternals
{
public:
  std::map<vtkModelEdge*, vtkSmartPointer<vtkCMBModelEdgeMeshServer> > ModelEdges;
  std::map<vtkModelFace*, vtkSmartPointer<vtkCMBModelFaceMeshServer> > ModelFaces;
};

//----------------------------------------------------------------------------
vtkCMBMeshServer::vtkCMBMeshServer()
{
  this->Internal = new vtkCMBMeshServerInternals;
}

//----------------------------------------------------------------------------
vtkCMBMeshServer::~vtkCMBMeshServer()
{
  if(this->CallbackCommand)
    {
    if(this->Model)
      {
      this->Model->RemoveObserver(this->CallbackCommand);
      }
    this->CallbackCommand = NULL;
    }
  if(this->Internal)
    {
    delete this->Internal;
    this->Internal = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkCMBMeshServer::Initialize(vtkModel* model)
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
  this->CallbackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->CallbackCommand->SetCallback(vtkCMBMeshServer::ModelGeometricEntityChanged);
  this->CallbackCommand->SetClientData(static_cast<void*>(this));
  model->AddObserver(ModelGeometricEntityBoundaryModified, this->CallbackCommand);
  model->AddObserver(ModelGeometricEntitiesAboutToMerge, this->CallbackCommand);
  model->AddObserver(ModelGeometricEntitySplit, this->CallbackCommand);

  // edges
  vtkModelItemIterator* iter = model->NewIterator(vtkModelEdgeType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelEdge* edge =
      vtkModelEdge::SafeDownCast(iter->GetCurrentItem());
    vtkSmartPointer<vtkCMBModelEdgeMeshServer> meshRepresentation =
      vtkSmartPointer<vtkCMBModelEdgeMeshServer>::New();
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
    vtkSmartPointer<vtkCMBModelFaceMeshServer> meshRepresentation =
      vtkSmartPointer<vtkCMBModelFaceMeshServer>::New();
    meshRepresentation->Initialize(this, face);
    this->Internal->ModelFaces[face] = meshRepresentation;
    }
  iter->Delete();
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkCMBMeshServer::SetGlobalLength(double globalLength)
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
bool vtkCMBMeshServer::SetGlobalMinimumAngle(double minAngle)
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
void vtkCMBMeshServer::Reset()
{
  this->Internal->ModelEdges.clear();
  this->Internal->ModelFaces.clear();
  if(this->CallbackCommand)
    {
    if(this->Model)
      {
      this->Model->RemoveObserver(this->CallbackCommand);
      }
    this->CallbackCommand = NULL;
    }
  this->Superclass::Reset();
  this->Modified();
}

//----------------------------------------------------------------------------
vtkCMBModelEntityMesh* vtkCMBMeshServer::GetModelEntityMesh(
  vtkModelGeometricEntity* entity)
{
  if(vtkModelEdge* modelEdge = vtkModelEdge::SafeDownCast(entity))
    {
    std::map<vtkModelEdge*,vtkSmartPointer<vtkCMBModelEdgeMeshServer> >::iterator it=
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
      vtkSmartPointer<vtkCMBModelFaceMeshServer> >::iterator it=
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
void vtkCMBMeshServer::ModelEdgeSplit(vtkSplitEventData* splitEventData)
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
  vtkCMBModelEdgeMesh* sourceMesh = vtkCMBModelEdgeMesh::SafeDownCast(
    this->GetModelEntityMesh(sourceEdge));
  sourceMesh->BuildModelEntityMesh(false);

  vtkSmartPointer<vtkCMBModelEdgeMeshServer> createdMesh =
    vtkSmartPointer<vtkCMBModelEdgeMeshServer>::New();
  createdMesh->SetLength(sourceMesh->GetLength());
  createdMesh->Initialize(this, createdEdge);
  this->Internal->ModelEdges[createdEdge] = createdMesh;
  createdMesh->BuildModelEntityMesh(true);

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCMBMeshServer::ModelEdgeMerge(vtkMergeEventData* mergeEventData)
{
  vtkModelEdge* sourceEdge = vtkModelEdge::SafeDownCast(
    mergeEventData->GetSourceEntity()->GetThisModelEntity());
  vtkModelEdge* targetEdge = vtkModelEdge::SafeDownCast(
    mergeEventData->GetTargetEntity()->GetThisModelEntity());
  vtkCMBModelEdgeMesh* sourceMesh = vtkCMBModelEdgeMesh::SafeDownCast(
    this->GetModelEntityMesh(sourceEdge));
  double sourceLength = sourceMesh->GetLength();
  this->Internal->ModelEdges.erase(sourceEdge);
  vtkCMBModelEdgeMesh* targetEdgeMesh = vtkCMBModelEdgeMesh::SafeDownCast(
    this->GetModelEntityMesh(targetEdge));
  if( (targetEdgeMesh->GetLength() > sourceLength && sourceLength > 0.) ||
      targetEdgeMesh->GetLength() <= 0.)
    {
    targetEdgeMesh->SetLength(sourceLength);
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCMBMeshServer::ModelEntityBoundaryModified(vtkModelGeometricEntity* entity)
{
  if(entity->IsA("vtkModelEdge") != 0)
    {
    if(vtkCMBModelEntityMesh* entityMesh = this->GetModelEntityMesh(entity))
      {
      entityMesh->BuildModelEntityMesh(false);
      }
    }
  else if(vtkModelFace* face = vtkModelFace::SafeDownCast(entity))
    {
    vtkModelItemIterator* edges = face->NewAdjacentModelEdgeIterator();
    for(edges->Begin();!edges->IsAtEnd();edges->Next())
      {
      if(vtkCMBModelEntityMesh* entityMesh =
         this->GetModelEntityMesh(vtkModelEdge::SafeDownCast(edges->GetCurrentItem())))
        {
        entityMesh->BuildModelEntityMesh(false);
        }
      }
    edges->Delete();
    if(vtkCMBModelEntityMesh* faceMesh = this->GetModelEntityMesh(face))
      {
      faceMesh->BuildModelEntityMesh(false);
      }
    }
}

//----------------------------------------------------------------------------
void vtkCMBMeshServer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
