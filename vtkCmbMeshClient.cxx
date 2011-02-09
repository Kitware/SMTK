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
#include "vtkCmbMeshClient.h"

#include <vtkCallbackCommand.h>
#include <vtkCMBModel.h>
#include "vtkCmbModelEdgeMeshClient.h"
#include "vtkCmbModelFaceMeshClient.h"
#include <vtkCMBModelGeometricEntity.h>
#include "vtkCmbModelVertexMesh.h"
#include <vtkIdList.h>
#include <vtkMergeEventData.h>
#include <vtkModel.h>
#include <vtkModelEdge.h>
#include <vtkModelEntity.h>
#include <vtkModelFace.h>
#include <vtkModelGeometricEntity.h>
#include <vtkModelItemIterator.h>
#include <vtkModelVertex.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkSplitEventData.h>

#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMOperatorProxy.h>
#include <vtkSMProxyManager.h>

#include <map>

vtkStandardNewMacro(vtkCmbMeshClient);
vtkCxxRevisionMacro(vtkCmbMeshClient, "");
vtkCxxSetObjectMacro(vtkCmbMeshClient, ServerModelProxy, vtkSMProxy);

class vtkCmbMeshClientInternals
{
public:
  std::map<vtkModelEdge*, vtkSmartPointer<vtkCmbModelEdgeMeshClient> > ModelEdges;
  std::map<vtkModelFace*, vtkSmartPointer<vtkCmbModelFaceMeshClient> > ModelFaces;
};

//----------------------------------------------------------------------------
vtkCmbMeshClient::vtkCmbMeshClient()
{
  this->ServerModelProxy = NULL;
  this->Internal = new vtkCmbMeshClientInternals;
}

//----------------------------------------------------------------------------
vtkCmbMeshClient::~vtkCmbMeshClient()
{
  if(this->Internal)
    {
    delete this->Internal;
    this->Internal = NULL;
    }
  this->SetServerModelProxy(NULL);
}

//----------------------------------------------------------------------------
void vtkCmbMeshClient::Initialize(vtkModel* model, vtkSMProxy* serverModelProxy)
{
  if(model == NULL)
    {
    vtkErrorMacro("Passed in NULL model.");
    return;
    }
  if(serverModelProxy == NULL)
    {
    vtkErrorMacro("Passed in NULL server side proxy.");
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
  callbackCommand->SetCallback(vtkCmbMeshClient::ModelGeometricEntityChanged);
  callbackCommand->SetClientData((void*) this);
  model->AddObserver(ModelGeometricEntityBoundaryModified, callbackCommand);
  model->AddObserver(ModelGeometricEntityCreated, callbackCommand);
  model->AddObserver(ModelGeometricEntityAboutToDestroy, callbackCommand);
  model->AddObserver(ModelGeometricEntitiesAboutToMerge, callbackCommand);
  model->AddObserver(ModelGeometricEntitySplit, callbackCommand);

  // edges
  this->Internal->ModelEdges.clear();
  vtkModelItemIterator* iter = model->NewIterator(vtkModelEdgeType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelEdge* edge =
      vtkModelEdge::SafeDownCast(iter->GetCurrentItem());
    vtkSmartPointer<vtkCmbModelEdgeMeshClient> meshRepresentation =
      vtkSmartPointer<vtkCmbModelEdgeMeshClient>::New();
    meshRepresentation->Initialize(this, edge);
    this->Internal->ModelEdges[edge] = meshRepresentation;
    }
  iter->Delete();
  // faces
  this->Internal->ModelFaces.clear();
  iter = model->NewIterator(vtkModelFaceType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelFace* face =
      vtkModelFace::SafeDownCast(iter->GetCurrentItem());
    vtkSmartPointer<vtkCmbModelFaceMeshClient> meshRepresentation =
      vtkSmartPointer<vtkCmbModelFaceMeshClient>::New();
    meshRepresentation->Initialize(this, face);
    this->Internal->ModelFaces[face] = meshRepresentation;
    }
  iter->Delete();

  // now initiate the initialization of the server side mesh
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* operatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBSimBuilderMeshGroup", "MeshOperator"));
  if(!operatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return;
    }
  operatorProxy->SetConnectionID(this->ServerModelProxy->GetConnectionID());
  operatorProxy->SetServers(this->ServerModelProxy->GetServers());

  vtkErrorMacro("STILL NEED TO FIGURE OUT HOW TO CALL REQUEST!!!!!!!!!!");

  // vtkSMDoubleVectorProperty* resetProperty =
  //   vtkSMDoubleVectorProperty::SafeDownCast(
  //     operatorProxy->GetProperty("Reset"));

  operatorProxy->Operate(vtkCMBModel::SafeDownCast(this->Model), this->ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* operateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("OperateSucceeded"));

  operatorProxy->UpdatePropertyInformation();

  int succeeded = operateSucceeded->GetElement(0);
  operatorProxy->Delete();
  operatorProxy = NULL;
  if(!succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMeshClient::SetGlobalLength(double globalLength)
{
  if(this->GlobalLength == globalLength)
    {
    return;
    }
  this->GlobalLength = globalLength > 0. ? globalLength : 0.;

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* operatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBSimBuilderMeshGroup", "MeshOperator"));
  if(!operatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return;
    }
  operatorProxy->SetConnectionID(this->ServerModelProxy->GetConnectionID());
  operatorProxy->SetServers(this->ServerModelProxy->GetServers());

  vtkSMDoubleVectorProperty* lengthProperty =
    vtkSMDoubleVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("GlobalLength"));
  lengthProperty->SetElement(0, this->GetGlobalLength());

  operatorProxy->Operate(vtkCMBModel::SafeDownCast(this->Model), this->ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* operateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("OperateSucceeded"));

  operatorProxy->UpdatePropertyInformation();

  int succeeded = operateSucceeded->GetElement(0);
  operatorProxy->Delete();
  operatorProxy = NULL;
  if(!succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMeshClient::SetGlobalMaximumArea(double maxArea)
{
  if(this->GlobalMaximumArea == maxArea)
    {
    return;
    }
  this->GlobalMaximumArea = maxArea > 0. ? maxArea : 0.;
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* operatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBSimBuilderMeshGroup", "MeshOperator"));
  if(!operatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return;
    }
  operatorProxy->SetConnectionID(this->ServerModelProxy->GetConnectionID());
  operatorProxy->SetServers(this->ServerModelProxy->GetServers());

  vtkSMDoubleVectorProperty* areaProperty =
    vtkSMDoubleVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("GlobalMaximumArea"));
  areaProperty->SetElement(0, this->GetGlobalMaximumArea());

  operatorProxy->Operate(vtkCMBModel::SafeDownCast(this->Model), this->ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* operateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("OperateSucceeded"));

  operatorProxy->UpdatePropertyInformation();

  int succeeded = operateSucceeded->GetElement(0);
  operatorProxy->Delete();
  operatorProxy = NULL;
  if(!succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMeshClient::SetGlobalMinimumAngle(double minAngle)
{
  if(this->GlobalMinimumAngle == minAngle)
    {
    return;
    }
  this->GlobalMinimumAngle = minAngle > 0. ? minAngle : 0.;
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* operatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBSimBuilderMeshGroup", "MeshOperator"));
  if(!operatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return;
    }
  operatorProxy->SetConnectionID(this->ServerModelProxy->GetConnectionID());
  operatorProxy->SetServers(this->ServerModelProxy->GetServers());

  vtkSMDoubleVectorProperty* angleProperty =
    vtkSMDoubleVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("GlobalLength"));
  angleProperty->SetElement(0, this->GetGlobalMinimumAngle());

  operatorProxy->Operate(vtkCMBModel::SafeDownCast(this->Model), this->ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* operateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("OperateSucceeded"));

  operatorProxy->UpdatePropertyInformation();

  int succeeded = operateSucceeded->GetElement(0);
  operatorProxy->Delete();
  operatorProxy = NULL;
  if(!succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMeshClient::Reset()
{
  this->Internal->ModelEdges.clear();
  this->Internal->ModelFaces.clear();
  this->SetServerModelProxy(NULL);
  this->Superclass::Reset();
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* operatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBSimBuilderMeshGroup", "MeshOperator"));
  if(!operatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return;
    }
  operatorProxy->SetConnectionID(this->ServerModelProxy->GetConnectionID());
  operatorProxy->SetServers(this->ServerModelProxy->GetServers());

  vtkErrorMacro("STILL NEED TO FIGURE OUT HOW TO CALL REQUEST!!!!!!!!!!");

  // vtkSMDoubleVectorProperty* resetProperty =
  //   vtkSMDoubleVectorProperty::SafeDownCast(
  //     operatorProxy->GetProperty("Reset"));

  operatorProxy->Operate(vtkCMBModel::SafeDownCast(this->Model), this->ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* operateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("OperateSucceeded"));

  operatorProxy->UpdatePropertyInformation();

  int succeeded = operateSucceeded->GetElement(0);
  operatorProxy->Delete();
  operatorProxy = NULL;
  if(!succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    }
  this->Modified();
}


//----------------------------------------------------------------------------
vtkCmbModelEntityMesh* vtkCmbMeshClient::GetModelEntityMesh(
  vtkModelGeometricEntity* entity)
{
  if(vtkModelEdge* modelEdge = vtkModelEdge::SafeDownCast(entity))
    {
    std::map<vtkModelEdge*,vtkSmartPointer<vtkCmbModelEdgeMeshClient> >::iterator it=
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
      vtkSmartPointer<vtkCmbModelFaceMeshClient> >::iterator it=
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
void vtkCmbMeshClient::ModelEdgeSplit(vtkSplitEventData* splitEventData)
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

  vtkSmartPointer<vtkCmbModelEdgeMeshClient> createdMesh =
    vtkSmartPointer<vtkCmbModelEdgeMeshClient>::New();
  createdMesh->SetLength(sourceMesh->GetLength());
  createdMesh->Initialize(this, createdEdge);
  // don't need to trigger the mesh since it's already been done on the server
  this->Internal->ModelEdges[createdEdge] = createdMesh;

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMeshClient::ModelEdgeMerge(vtkMergeEventData* mergeEventData)
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
void vtkCmbMeshClient::ModelEntityBoundaryModified(vtkModelGeometricEntity* entity)
{
  // no op on the client
}

//----------------------------------------------------------------------------
void vtkCmbMeshClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  if(this->ServerModelProxy)
    {
    os << "ServerModelProxy: " << this->ServerModelProxy << "\n";
    }
  else
    {
    os << "ServerModelProxy: (NULL)\n";
    }
}
