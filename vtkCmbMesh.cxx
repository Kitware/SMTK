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

  // vertices
  vtkModelItemIterator* iter = model->NewIterator(vtkModelVertexType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelVertex* vertex =
      vtkModelVertex::SafeDownCast(iter->GetCurrentItem());
    vtkSmartPointer<vtkCmbModelVertexMesh> meshRepresentation =
      vtkSmartPointer<vtkCmbModelVertexMesh>::New();
    meshRepresentation->SetModelEntityMeshSize(this->GlobalLength);
    this->Internal->ModelEntities[vertex] =
      meshRepresentation;
    }
  iter->Delete();
  // edges
  iter = model->NewIterator(vtkModelEdgeType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelEdge* edge =
      vtkModelEdge::SafeDownCast(iter->GetCurrentItem());
    vtkSmartPointer<vtkCmbModelEdgeMesh> meshRepresentation =
      vtkSmartPointer<vtkCmbModelEdgeMesh>::New();
    meshRepresentation->SetModelEntityMeshSize(this->GlobalLength);
    this->Internal->ModelEntities[edge] =
      meshRepresentation;
    }
  iter->Delete();
//   // faces
//   iter = model->NewIterator(vtkModelFaceType);
//   for(iter->Begin();!iter->IsAtEnd();iter->Next())
//     {
//     vtkModelEntity* entity =
//       vtkModelEntity::SafeDownCast(iter->GetCurrentItem());
//     vtkSmartPointer<vtkCmbModelEntityMesh> meshRepresentation =
//       vtkSmartPointer<vtkCmbModelEntityMesh>::New();
//     meshRepresentation->SetGridSize(this->GlobalLength);
//     this->Internal->ModelEntities[entity->GetUniquePersistentId()] =
//       meshRepresentation;
//     }
//   iter->Delete();
//   // regions
//   iter = model->NewIterator(vtkModelRegionType);
//   for(iter->Begin();!iter->IsAtEnd();iter->Next())
//     {
//     vtkModelEntity* entity =
//       vtkModelEntity::SafeDownCast(iter->GetCurrentItem());
//     vtkSmartPointer<vtkCmbModelEntityMesh> meshRepresentation =
//       vtkSmartPointer<vtkCmbModelEntityMesh>::New();
//     meshRepresentation->SetGridSize(this->GlobalLength);
//     this->Internal->ModelEntities[entity->GetUniquePersistentId()] =
//       meshRepresentation;
//     }
//   iter->Delete();

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
    vtkWarningMacro("Trying to set GlobalLength to invalid value of " << globalLength);
    return;
    }
  this->GlobalLength = globalLength;
  for(std::map<vtkModelGeometricEntity*,
        vtkSmartPointer<vtkCmbModelEntityMesh> >::iterator it=
        this->Internal->ModelEntities.begin();it!=this->Internal->ModelEntities.end();
      it++)
    {
    it->second->SetModelEntityMeshSize(globalLength);
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
    vtkModelEdge* sourceEdge = vtkModelEdge::SafeDownCast(
      splitEventData->GetSourceEntity()->GetThisModelEntity());
    if(splitEventData->GetCreatedModelEntityIds()->GetNumberOfIds() != 2)
      {
      vtkGenericWarningMacro("Problem with split event.");
      return;
      }
    vtkModelVertex* createdVertex = NULL;
    vtkModelEdge* createdEdge = vtkModelEdge::SafeDownCast(
      model->GetModelEntity(vtkModelEdgeType,
                            splitEventData->GetCreatedModelEntityIds()->GetId(0)));
    if(createdEdge == NULL)
      {
      createdEdge = vtkModelEdge::SafeDownCast(
        model->GetModelEntity(vtkModelEdgeType,
                              splitEventData->GetCreatedModelEntityIds()->GetId(1)));
      createdVertex = vtkModelVertex::SafeDownCast(
        model->GetModelEntity(vtkModelVertexType,
                              splitEventData->GetCreatedModelEntityIds()->GetId(0)));
      }
    else
      {
      createdVertex = vtkModelVertex::SafeDownCast(
        model->GetModelEntity(vtkModelVertexType,
                              splitEventData->GetCreatedModelEntityIds()->GetId(1)));

      }
    vtkModelVertex* sourceVertex1 =
      vtkModelVertex::SafeDownCast(sourceEdge->GetAdjacentModelVertex(0));
    if(sourceVertex1 == createdVertex)
      {
      sourceVertex1 = vtkModelVertex::SafeDownCast(
        sourceEdge->GetAdjacentModelVertex(1));
      }
    vtkModelVertex* sourceVertex2 =
      vtkModelVertex::SafeDownCast(createdEdge->GetAdjacentModelVertex(0));
    if(sourceVertex2 == createdVertex)
      {
      sourceVertex2 = vtkModelVertex::SafeDownCast(
        createdEdge->GetAdjacentModelVertex(1));
      }
    double size1 = cmbMesh->GetModelEntityMesh(sourceVertex1)->GetModelEntityMeshSize();
    double size2 = cmbMesh->GetModelEntityMesh(sourceVertex2)->GetModelEntityMeshSize();
    double newSize = size1;
    if(size1 != size2)
      {
      double coords1[3], coords2[3], createdCoords[3];
      sourceVertex1->GetPoint(coords1);
      sourceVertex2->GetPoint(coords2);
      createdVertex->GetPoint(createdCoords);
      double length1 = sqrt(vtkMath::Distance2BetweenPoints(coords1, createdCoords));
      double length2 = sqrt(vtkMath::Distance2BetweenPoints(coords2, createdCoords));
      newSize = size1+(size2-size1)*length1/(length1+length2);
      }

    vtkSmartPointer<vtkCmbModelVertexMesh> meshRepresentation =
      vtkSmartPointer<vtkCmbModelVertexMesh>::New();
    meshRepresentation->SetModelEntityMeshSize(newSize);
    cmbMesh->Internal->ModelEntities[createdVertex] =
      meshRepresentation;
    }


  cmbMesh->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Visible: " << this->Visible << "\n";
  os << indent << "GlobalLength: " << this->GlobalLength << "\n";
}

