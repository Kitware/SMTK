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
#include "vtkCmbMesh.h"

#include <vtkCallbackCommand.h>
#include "vtkCmbModelEdgeMesh.cxx"
#include "vtkCmbModelFaceMesh.h"
#include <vtkCMBModelGeometricEntity.h>
#include "vtkCmbModelVertexMesh.h"
#include <vtkIdList.h>
#include <vtkMath.h>
#include <vtkMergeEventData.h>
#include <vtkModel.h>
#include <vtkModelEdge.cxx>
#include <vtkModelEntity.h>
#include <vtkModelFace.h>
#include <vtkModelGeometricEntity.h>
#include <vtkModelItemIterator.h>
#include <vtkModelVertex.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkSplitEventData.h>
#include <vtkWeakPointer.h>

#include <map>

vtkStandardNewMacro(vtkCmbMesh);
vtkCxxRevisionMacro(vtkCmbMesh, "");

class vtkCmbMeshInternals
{
public:
  std::map<vtkModelEdge*, vtkSmartPointer<vtkCmbModelEdgeMesh> > ModelEdges;
  std::map<vtkModelFace*, vtkSmartPointer<vtkCmbModelFaceMesh> > ModelFaces;
  vtkWeakPointer<vtkModel> Model;
  vtkCmbMeshInternals() : Model(NULL)
    {};
};

//----------------------------------------------------------------------------
vtkCmbMesh::vtkCmbMesh()
{
  this->Visible = true;
  this->GlobalLength = 0;
  this->GlobalMaximumArea = 0;
  this->GlobalMinimumAngle = 0;
  this->Internal = new vtkCmbMeshInternals;
}

//----------------------------------------------------------------------------
vtkCmbMesh::~vtkCmbMesh()
{
  if(this->Internal)
    {
    delete this->Internal;
    this->Internal = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkCmbMesh::Initialize(vtkModel* model)
{
  if(model == NULL)
    {
    vtkErrorMacro("Passed in NULL model.");
    return;
    }
  if(this->Internal->Model != model)
    {
    this->Reset();
    this->Internal->Model = model;
    }
  // register model modification events that we want
  // this may not be correct yet
  vtkSmartPointer<vtkCallbackCommand> callbackCommand =
    vtkSmartPointer<vtkCallbackCommand>::New();
  callbackCommand->SetCallback(vtkCmbMesh::ModelGeometricEntityChanged);
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
    vtkSmartPointer<vtkCmbModelEdgeMesh> meshRepresentation =
      vtkSmartPointer<vtkCmbModelEdgeMesh>::New();
    meshRepresentation->Initialize(this, edge);
    this->Internal->ModelEdges[edge] = meshRepresentation;
    }
  iter->Delete();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMesh::SetGlobalLength(double globalLength)
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
  for(std::map<vtkModelEdge*, vtkSmartPointer<vtkCmbModelEdgeMesh> >::iterator it=
        this->Internal->ModelEdges.begin();it!=this->Internal->ModelEdges.end();
      it++)
    {
    it->second->BuildModelEntityMesh(false);
    }
  // now remesh the model faces
  for(std::map<vtkModelFace*, vtkSmartPointer<vtkCmbModelFaceMesh> >::iterator it=
        this->Internal->ModelFaces.begin();it!=this->Internal->ModelFaces.end();
      it++)
    {
    it->second->BuildModelEntityMesh(false);
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMesh::SetGlobalMaximumArea(double maxArea)
{
  if(this->GlobalMaximumArea == maxArea)
    {
    return;
    }
  this->GlobalMaximumArea = maxArea > 0. ? maxArea : 0.;
  for(std::map<vtkModelFace*, vtkSmartPointer<vtkCmbModelFaceMesh> >::iterator it=
        this->Internal->ModelFaces.begin();it!=this->Internal->ModelFaces.end();
      it++)
    {
    it->second->BuildModelEntityMesh(false);
    }
  // when we implement volume meshing we'll have to trigger a remesh here
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMesh::SetGlobalMinimumAngle(double minAngle)
{
  if(this->GlobalMinimumAngle == minAngle)
    {
    return;
    }
  this->GlobalMinimumAngle = minAngle > 0. ? minAngle : 0.;
  for(std::map<vtkModelFace*, vtkSmartPointer<vtkCmbModelFaceMesh> >::iterator it=
        this->Internal->ModelFaces.begin();it!=this->Internal->ModelFaces.end();
      it++)
    {
    it->second->BuildModelEntityMesh(false);
    }
  // when we implement volume meshing we'll have to trigger a remesh here
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMesh::Reset()
{
  this->Modified();
}

//----------------------------------------------------------------------------
vtkCmbModelEntityMesh* vtkCmbMesh::GetModelEntityMesh(
  vtkModelGeometricEntity* entity)
{
  if(vtkModelEdge* modelEdge = vtkModelEdge::SafeDownCast(entity))
    {
    std::map<vtkModelEdge*,
      vtkSmartPointer<vtkCmbModelEdgeMesh> >::iterator it=
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
      vtkSmartPointer<vtkCmbModelFaceMesh> >::iterator it=
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
void vtkCmbMesh::ModelGeometricEntityChanged(
  vtkObject *caller, unsigned long event, void *cData, void *callData)
{
  vtkCmbMesh* cmbMesh = (vtkCmbMesh*) cData;
  vtkModel* model = cmbMesh->Internal->Model;
  if(event == ModelGeometricEntitySplit)
    {
    vtkSplitEventData* splitEventData = (vtkSplitEventData*) callData;
    if(model->GetModelDimension() == 2)
      {
      cmbMesh->ModelEdgeSplit(splitEventData);
      }
    else
      {
      vtkGenericWarningMacro("Model face split not implemented yet.")
      }
    }
  else if(event == ModelGeometricEntitiesAboutToMerge)
    {
    vtkMergeEventData* mergeEventData = (vtkMergeEventData*) callData;
    if(model->GetModelDimension() == 2)
      {
      cmbMesh->ModelEdgeMerge(mergeEventData);
      }
    else
      {
      vtkGenericWarningMacro("Model face merge not implemented yet.")
      }
    }
  else if(event == ModelGeometricEntityBoundaryModified)
    {
    cmbMesh->ModelEntityBoundaryModified((vtkModelGeometricEntity*)callData);
    }
}

//----------------------------------------------------------------------------
void vtkCmbMesh::ModelEdgeSplit(vtkSplitEventData* splitEventData)
{
  vtkModelEdge* sourceEdge = vtkModelEdge::SafeDownCast(
    splitEventData->GetSourceEntity()->GetThisModelEntity());
  if(splitEventData->GetCreatedModelEntityIds()->GetNumberOfIds() != 2)
    {
    vtkGenericWarningMacro("Problem with split event.");
    return;
    }
  vtkModelEdge* createdEdge = vtkModelEdge::SafeDownCast(
    this->Internal->Model->GetModelEntity(
      vtkModelEdgeType, splitEventData->GetCreatedModelEntityIds()->GetId(0)));
  if(createdEdge == NULL)
    {
    createdEdge = vtkModelEdge::SafeDownCast(
      this->Internal->Model->GetModelEntity(
        vtkModelEdgeType, splitEventData->GetCreatedModelEntityIds()->GetId(1)));
    }
  vtkCmbModelEdgeMesh* sourceMesh = vtkCmbModelEdgeMesh::SafeDownCast(
    this->GetModelEntityMesh(sourceEdge));
  sourceMesh->BuildModelEntityMesh(false);

  vtkSmartPointer<vtkCmbModelEdgeMesh> createdMesh =
    vtkSmartPointer<vtkCmbModelEdgeMesh>::New();
  createdMesh->SetLength(sourceMesh->GetLength());
  createdMesh->Initialize(this, createdEdge);
  createdMesh->BuildModelEntityMesh(true);
  this->Internal->ModelEdges[createdEdge] = createdMesh;

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMesh::ModelEdgeMerge(vtkMergeEventData* mergeEventData)
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
void vtkCmbMesh::ModelEntityBoundaryModified(vtkModelGeometricEntity* entity)
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
void vtkCmbMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Visible: " << this->Visible << "\n";
  os << indent << "GlobalLength: " << this->GlobalLength << "\n";
}

