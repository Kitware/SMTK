//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBMeshClient.h"

#include "vtkCMBMeshGridRepresentationClient.h"
#include "vtkCMBModelEdgeMeshClient.h"
#include "vtkCMBModelFaceMeshClient.h"
#include "vtkCMBModelVertexMesh.h"

#include <vtkCallbackCommand.h>
#include <vtkDiscreteModel.h>
#include <vtkDiscreteModelGeometricEntity.h>
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

#include "vtkClientServerStream.h"
#include "vtkProcessModule.h"
#include "vtkSMProxyProperty.h"
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMProxy.h>
#include <vtkSMProxyManager.h>

#include <map>

vtkStandardNewMacro(vtkCMBMeshClient);
vtkCxxSetObjectMacro(vtkCMBMeshClient, ServerModelProxy, vtkSMProxy);

class vtkCMBMeshClientInternals
{
public:
  std::map<vtkModelEdge*, vtkSmartPointer<vtkCMBModelEdgeMeshClient> > ModelEdges;
  std::map<vtkModelFace*, vtkSmartPointer<vtkCMBModelFaceMeshClient> > ModelFaces;
};

vtkCMBMeshClient::vtkCMBMeshClient()
{
  this->ServerModelProxy = NULL;
  this->ServerMeshProxy = NULL;
  this->Internal = new vtkCMBMeshClientInternals;
}

vtkCMBMeshClient::~vtkCMBMeshClient()
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
  if(this->ServerMeshProxy)
    {
    this->ServerMeshProxy->Delete();
    this->ServerMeshProxy = NULL;
    }
  this->SetServerModelProxy(NULL);
}

void vtkCMBMeshClient::Initialize(vtkModel* model, vtkSMProxy* smModelProxy)
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
  this->CallbackCommand =
    vtkSmartPointer<vtkCallbackCommand>::New();
  this->CallbackCommand->SetCallback(vtkCMBMeshClient::ModelGeometricEntityChanged);
  this->CallbackCommand->SetClientData(static_cast<void*>(this));
  model->AddObserver(ModelGeometricEntityBoundaryModified, this->CallbackCommand);
  model->AddObserver(ModelGeometricEntitiesAboutToMerge, this->CallbackCommand);
  model->AddObserver(ModelGeometricEntitySplit, this->CallbackCommand);

  // edges
  this->Internal->ModelEdges.clear();
  vtkModelItemIterator* iter = model->NewIterator(vtkModelEdgeType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelEdge* edge =
      vtkModelEdge::SafeDownCast(iter->GetCurrentItem());
    vtkSmartPointer<vtkCMBModelEdgeMeshClient> meshRepresentation =
      vtkSmartPointer<vtkCMBModelEdgeMeshClient>::New();
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
    vtkSmartPointer<vtkCMBModelFaceMeshClient> meshRepresentation =
      vtkSmartPointer<vtkCMBModelFaceMeshClient>::New();
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
  this->ServerMeshProxy->SetLocation(this->ServerModelProxy->GetLocation());

  vtkSMProxyProperty* proxyproperty =
    vtkSMProxyProperty::SafeDownCast(
    this->ServerMeshProxy->GetProperty("ModelWrapper"));
  proxyproperty->AddProxy(this->ServerModelProxy);
  this->ServerMeshProxy->UpdateVTKObjects();
}

bool vtkCMBMeshClient::SetGlobalLength(double globalLength)
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

bool vtkCMBMeshClient::SetGlobalMinimumAngle(double minAngle)
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

void vtkCMBMeshClient::Reset()
{
  this->Internal->ModelEdges.clear();
  this->Internal->ModelFaces.clear();
  if(this->ServerMeshProxy)
    {
    this->ServerMeshProxy->Delete();
    this->ServerMeshProxy = NULL;
    }
  this->SetServerModelProxy(NULL);
  if(this->CallbackCommand)
    {
    if(this->Model)
      {
      this->Model->RemoveObserver(this->CallbackCommand);
      }
    this->CallbackCommand = NULL;
    }
  this->Superclass::Reset();
}

bool vtkCMBMeshClient::BuildModelEntityMeshes()
{
  vtkSmartPointer<vtkModelItemIterator> edges;
  edges.TakeReference(this->Model->NewIterator(vtkModelEdgeType));
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkModelEdge* edge = vtkModelEdge::SafeDownCast(edges->GetCurrentItem());
    vtkCMBModelEdgeMeshClient* edgeMesh =
      vtkCMBModelEdgeMeshClient::SafeDownCast(this->GetModelEntityMesh(edge));
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
    vtkCMBModelFaceMeshClient* faceMesh =
      vtkCMBModelFaceMeshClient::SafeDownCast(this->GetModelEntityMesh(face));
    if(faceMesh->BuildModelEntityMesh(false) == false)
      {
      return false;
      }
    }
  return true;
}

bool vtkCMBMeshClient::BuildModelMeshRepresentation(
  const char* fileName, const bool &isAnalysisMesh,
  vtkSMProxy* meshRepresentionInput)
{
  vtkCMBMeshGridRepresentationClient *meshRep =
    vtkCMBMeshGridRepresentationClient::New();
  vtkDiscreteModel *mod = vtkDiscreteModel::SafeDownCast(this->Model);
  if ( mod )
    {
    meshRep->SetMeshIsAnalysisGrid(isAnalysisMesh);
    if (fileName)
      {
      meshRep->SetGridFileName(fileName);
      }
    meshRep->SetMeshRepresentationSource(meshRepresentionInput);
    bool result =  meshRep->Operate(mod,this->ServerMeshProxy);
    meshRep->Delete();
    return result;
    }
  else
    {
    return false;
    }
}

vtkCMBModelEntityMesh* vtkCMBMeshClient::GetModelEntityMesh(
  vtkModelGeometricEntity* entity)
{
  if(vtkModelEdge* modelEdge = vtkModelEdge::SafeDownCast(entity))
    {
    std::map<vtkModelEdge*,vtkSmartPointer<vtkCMBModelEdgeMeshClient> >::iterator it=
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
      vtkSmartPointer<vtkCMBModelFaceMeshClient> >::iterator it=
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

void vtkCMBMeshClient::ModelEdgeSplit(vtkSplitEventData* splitEventData)
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

  vtkSmartPointer<vtkCMBModelEdgeMeshClient> createdMesh =
    vtkSmartPointer<vtkCMBModelEdgeMeshClient>::New();
  createdMesh->Initialize(this, createdEdge);
  createdMesh->SetLength(sourceMesh->GetLength());
  createdMesh->SetMeshedLength(sourceMesh->GetMeshedLength());
  // don't need to trigger the mesh since it's already been done on the server
  this->Internal->ModelEdges[createdEdge] = createdMesh;

  this->Modified();
}

void vtkCMBMeshClient::ModelEdgeMerge(vtkMergeEventData* mergeEventData)
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

  // we can't remesh the target edge yet since the topology hasn't changed
  // yet.  we mark it as modified so that when we see the boundary modified
  // event we will trigger the remeshing then.
  targetEdgeMesh->Modified();

  this->Modified();
}

void vtkCMBMeshClient::ModelEntityBoundaryModified(vtkModelGeometricEntity* /*entity*/)
{
  // no op on the client
}

void vtkCMBMeshClient::PrintSelf(ostream& os, vtkIndent indent)
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
