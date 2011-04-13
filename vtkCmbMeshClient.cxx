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
#include <vtkSMProxy.h>
#include <vtkSMProxyManager.h>
#include "vtkSMProxyProperty.h"
#include "vtkClientServerStream.h"
#include "vtkProcessModule.h"
#include "vtkCollection.h"

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
  this->ServerMeshProxy = NULL;
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
  if(this->ServerMeshProxy)
    {
    this->ServerMeshProxy->Delete();
    this->ServerMeshProxy = NULL;
    }
  this->SetServerModelProxy(NULL);
}

//----------------------------------------------------------------------------
void vtkCmbMeshClient::Initialize(vtkModel* model, vtkSMProxy* smModelProxy)
{
  if(model == NULL)
    {
    vtkErrorMacro("Passed in NULL model.");
    return;
    }
  if(smModelProxy == NULL)
    {
    vtkErrorMacro("Passed in NULL server side proxy.");
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
  if(this->ServerModelProxy != smModelProxy)
    {
    this->SetServerModelProxy(smModelProxy);
    }
  if(this->ServerMeshProxy)
    {
    this->ServerMeshProxy->Delete();
    this->ServerMeshProxy = NULL;
    }
  // register model modification events that we want
  // this may not be correct yet
  vtkSmartPointer<vtkCallbackCommand> callbackCommand =
    vtkSmartPointer<vtkCallbackCommand>::New();
  callbackCommand->SetCallback(vtkCmbMeshClient::ModelGeometricEntityChanged);
  callbackCommand->SetClientData((void*) this);
  model->AddObserver(ModelGeometricEntityBoundaryModified, callbackCommand);
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
  this->ServerMeshProxy = manager->NewProxy("CMBSimBuilderMeshGroup", "CMBMeshWrapper");
  if(!this->ServerMeshProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return;
    }
  this->ServerMeshProxy->SetConnectionID(this->ServerModelProxy->GetConnectionID());
  this->ServerMeshProxy->SetServers(this->ServerModelProxy->GetServers());

  vtkSMProxyProperty* proxyproperty =
    vtkSMProxyProperty::SafeDownCast(
    this->ServerMeshProxy->GetProperty("ModelWrapper"));
  proxyproperty->AddProxy(this->ServerModelProxy);
  this->ServerMeshProxy->UpdateVTKObjects();
}

//----------------------------------------------------------------------------
bool vtkCmbMeshClient::SetGlobalLength(double globalLength)
{
  if(this->GlobalLength == globalLength || !this->ServerMeshProxy)
    {
    return false;
    }
  this->GlobalLength = globalLength > 0. ? globalLength : 0.;

  vtkSMDoubleVectorProperty* lengthProperty =
    vtkSMDoubleVectorProperty::SafeDownCast(
      this->ServerMeshProxy->GetProperty("GlobalLength"));
  lengthProperty->SetElement(0, this->GetGlobalLength());
  this->ServerMeshProxy->UpdateVTKObjects();
  return true;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshClient::SetGlobalMaximumArea(double maxArea)
{
  if(this->GlobalMaximumArea == maxArea)
    {
    return false;
    }
  this->GlobalMaximumArea = maxArea > 0. ? maxArea : 0.;

  vtkSMDoubleVectorProperty* areaProperty =
    vtkSMDoubleVectorProperty::SafeDownCast(
      this->ServerMeshProxy->GetProperty("GlobalMaximumArea"));
  areaProperty->SetElement(0, this->GetGlobalMaximumArea());
  this->ServerMeshProxy->UpdateVTKObjects();
  return true;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshClient::SetGlobalMinimumAngle(double minAngle)
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
  vtkSMDoubleVectorProperty* angleProperty =
    vtkSMDoubleVectorProperty::SafeDownCast(
      this->ServerMeshProxy->GetProperty("GlobalMinimumAngle"));
  angleProperty->SetElement(0, this->GetGlobalMinimumAngle());
  this->ServerMeshProxy->UpdateVTKObjects();
  return true;
}

//----------------------------------------------------------------------------
void vtkCmbMeshClient::Reset()
{
  this->Internal->ModelEdges.clear();
  this->Internal->ModelFaces.clear();
  if(this->ServerMeshProxy)
    {
    this->ServerMeshProxy->Delete();
    this->ServerMeshProxy = NULL;
    }
  this->SetServerModelProxy(NULL);
  this->Superclass::Reset();
}

//----------------------------------------------------------------------------
bool vtkCmbMeshClient::BuildModelEntityMeshes()
{
  vtkSmartPointer<vtkModelItemIterator> edges;
  edges.TakeReference(this->Model->NewIterator(vtkModelEdgeType));
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkModelEdge* edge = vtkModelEdge::SafeDownCast(edges->GetCurrentItem());
    vtkCmbModelEdgeMeshClient* edgeMesh =
      vtkCmbModelEdgeMeshClient::SafeDownCast(this->GetModelEntityMesh(edge));
    if(edgeMesh->BuildModelEntityMesh(false) == false)
      {
      vtkErrorMacro("Unable to build model edge mesh.");
      return false;
      }
    }
  vtkSmartPointer<vtkModelItemIterator> faces;
  faces.TakeReference(this->Model->NewIterator(vtkModelFaceType));
  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
    vtkCmbModelFaceMeshClient* faceMesh =
      vtkCmbModelFaceMeshClient::SafeDownCast(this->GetModelEntityMesh(face));
    if(faceMesh->BuildModelEntityMesh(false) == false)
      {
      vtkErrorMacro("Unable to build model face mesh.");
      return false;
      }
    }
  return true;
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
  createdMesh->Initialize(this, createdEdge);
  createdMesh->SetLength(sourceMesh->GetLength());
  createdMesh->SetMeshedLength(sourceMesh->GetMeshedLength());
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
bool vtkCmbMeshClient::SetLocalMeshLength(
  vtkCollection* selectedMeshEntities, double localLen)
{
  if(!selectedMeshEntities || selectedMeshEntities->GetNumberOfItems()==0)
    {
    return false;
    }
  bool res = true;
  for(int i=0; i<selectedMeshEntities->GetNumberOfItems(); i++)
    {
    vtkSmartPointer<vtkCmbModelEdgeMeshClient> edgeMesh =
      vtkCmbModelEdgeMeshClient::SafeDownCast(
      selectedMeshEntities->GetItemAsObject(i));
    if(edgeMesh)
      {
      res = res && edgeMesh->SetLocalLength(localLen);
      }
    }
  return res;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshClient::SetLocalMeshMinimumAngle(
  vtkCollection* selectedMeshEntities, double localMinAngle)
{
  if(!selectedMeshEntities || selectedMeshEntities->GetNumberOfItems()==0)
    {
    return false;
    }
  bool res = true;
  for(int i=0; i<selectedMeshEntities->GetNumberOfItems(); i++)
    {
    vtkSmartPointer<vtkCmbModelFaceMeshClient> faceMesh =
      vtkCmbModelFaceMeshClient::SafeDownCast(
      selectedMeshEntities->GetItemAsObject(i));
    if(faceMesh)
      {
      res = res && faceMesh->SetLocalMinimumAngle(localMinAngle);
      }
    }
  return res;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshClient::SetLocalMeshMaximumArea(
  vtkCollection* selectedMeshEntities, double localMaxArea)
{
  if(!selectedMeshEntities || selectedMeshEntities->GetNumberOfItems()==0)
    {
    return false;
    }

  bool res = true;
  for(int i=0; i<selectedMeshEntities->GetNumberOfItems(); i++)
    {
    vtkSmartPointer<vtkCmbModelFaceMeshClient> faceMesh =
      vtkCmbModelFaceMeshClient::SafeDownCast(
      selectedMeshEntities->GetItemAsObject(i));
    if(faceMesh)
      {
      res = res && faceMesh->SetLocalMaximumArea(localMaxArea);
      }
    }
  return res;
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
  if(this->ServerMeshProxy)
    {
    os << "ServerMeshProxy: " << this->ServerMeshProxy << "\n";
    }
  else
    {
    os << "ServerMeshProxy: (NULL)\n";
    }
}
