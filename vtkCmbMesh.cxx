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
#include <vtkCMBModelGeometricEntity.h>
#include "vtkCmbModelVertexMesh.h"
#include <vtkIdList.h>
#include <vtkMath.h>
#include <vtkMergeEventData.h>
#include <vtkModel.h>
#include <vtkModelEdge.cxx>
#include <vtkModelEntity.h>
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
  std::map<vtkModelGeometricEntity*, vtkSmartPointer<vtkCmbModelEntityMesh> > ModelEntities;
  vtkWeakPointer<vtkModel> Model;
  vtkCmbMeshInternals() : Model(NULL)
    {};
};

//----------------------------------------------------------------------------
vtkCmbMesh::vtkCmbMesh()
{
  this->Visible = true;
  this->GlobalLength = 0;
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
    meshRepresentation->SetModelEntityMeshSize(this->GlobalLength);
    this->Internal->ModelEntities[edge] =
      meshRepresentation;
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
    return;
    }
  this->GlobalLength = globalLength;
  for(std::map<vtkModelGeometricEntity*,
        vtkSmartPointer<vtkCmbModelEntityMesh> >::iterator it=
        this->Internal->ModelEntities.begin();it!=this->Internal->ModelEntities.end();
      it++)
    {
    it->second->SetModelEntityMeshSize(globalLength);
    it->second->BuildModelEntityMesh();
    }
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
  std::map<vtkModelGeometricEntity*,
    vtkSmartPointer<vtkCmbModelEntityMesh> >::iterator it=
    this->Internal->ModelEntities.find(entity);
  if(it == this->Internal->ModelEntities.end())
    {
    return NULL;
    }
  return it->second;
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
  vtkCmbModelEntityMesh* sourceMesh = this->GetModelEntityMesh(sourceEdge);
  sourceMesh->BuildModelEntityMesh();
  double size = sourceMesh->GetModelEntityMeshSize();

  vtkSmartPointer<vtkCmbModelEdgeMesh> createdMesh =
    vtkSmartPointer<vtkCmbModelEdgeMesh>::New();
  createdMesh->Initialize(this, createdEdge);
  createdMesh->SetModelEntityMeshSize(size);
  createdMesh->BuildModelEntityMesh();
  this->Internal->ModelEntities[createdEdge] = createdMesh;

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMesh::ModelEdgeMerge(vtkMergeEventData* mergeEventData)
{
  vtkModelEdge* sourceEdge = vtkModelEdge::SafeDownCast(
    mergeEventData->GetSourceEntity()->GetThisModelEntity());
  vtkModelEdge* targetEdge = vtkModelEdge::SafeDownCast(
    mergeEventData->GetTargetEntity()->GetThisModelEntity());
  vtkCmbModelEntityMesh* sourceMesh = this->GetModelEntityMesh(sourceEdge);
  double sourceSize = sourceMesh->GetModelEntityMeshSize();
  this->Internal->ModelEntities.erase(sourceEdge);
  vtkCmbModelEntityMesh* targetEdgeMesh = this->GetModelEntityMesh(targetEdge);
  if(sourceSize > 0)
    {
    double targetSize = targetEdgeMesh->GetModelEntityMeshSize();
    if(targetSize > sourceSize)
      {
      targetEdgeMesh->SetModelEntityMeshSize(sourceSize);
      }
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
  if(entity->IsA("vtkModelEdge") == true)
    {
    if(vtkCmbModelEntityMesh* entityMesh = this->GetModelEntityMesh(entity))
      {
      entityMesh->BuildModelEntityMesh();
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
        entityMesh->BuildModelEntityMesh();
        }
      }
    cout << "still need to update the model face mesh!!!!!!!!!!\n";
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

