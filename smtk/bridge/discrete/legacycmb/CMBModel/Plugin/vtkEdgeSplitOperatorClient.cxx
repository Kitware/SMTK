//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkEdgeSplitOperatorClient.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkModelFace.h"
#include "vtkModelItemIterator.h"
#include "vtkSplitEventData.h"
#include <vtkIdList.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSMIdTypeVectorProperty.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMOperatorProxy.h>
#include <vtkSMProxyManager.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro(vtkEdgeSplitOperatorClient);

vtkEdgeSplitOperatorClient::vtkEdgeSplitOperatorClient()
{
}

vtkEdgeSplitOperatorClient::~vtkEdgeSplitOperatorClient()
{
}

bool vtkEdgeSplitOperatorClient::Operate(vtkDiscreteModel* model,
                                         vtkSMProxy* serverModelProxy)
{
  if(!this->AbleToOperate(model))
    {
    return 0;
    }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSmartPointer<vtkSMOperatorProxy> operatorProxy;
  operatorProxy.TakeReference(
    vtkSMOperatorProxy::SafeDownCast(
      manager->NewProxy("CMBModelGroup", "EdgeSplitOperator")));

  if(!operatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
    }
  operatorProxy->SetLocation(serverModelProxy->GetLocation());

  vtkSMIdTypeVectorProperty* pointIdProperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("PointId"));
  pointIdProperty->SetElement(0, this->GetPointId());
  operatorProxy->UpdateVTKObjects();

  vtkSMIdTypeVectorProperty* edgeIdProperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("EdgeId"));
  edgeIdProperty->SetElement(0, this->GetEdgeId());
  operatorProxy->UpdateVTKObjects();

  operatorProxy->Operate(model, serverModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* operateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("OperateSucceeded"));

  operatorProxy->UpdatePropertyInformation();

  if(!operateSucceeded->GetElement(0))
    {
    vtkErrorMacro("Server side operator failed.");
    return 0;
    }

  // now update the information on the client
  vtkSMIdTypeVectorProperty* createdModelEdgeId =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("CreatedModelEdgeID"));
  vtkSMIdTypeVectorProperty* createdModelVertexId =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("CreatedModelVertexID"));

  operatorProxy->UpdateVTKObjects();
  operatorProxy->UpdatePropertyInformation();

  vtkDiscreteModelEdge* edge =
    vtkDiscreteModelEdge::SafeDownCast(this->GetModelEntity(model));

  vtkIdType newModelVertexId = createdModelVertexId->GetElement(0);

  vtkIdType newModelEdgeId = createdModelEdgeId->GetElement(0);
  if(newModelEdgeId >= 0)
    {
    vtkDiscreteModelVertex* newVertex =
      vtkDiscreteModelVertex::SafeDownCast(model->BuildModelVertex(-1, newModelVertexId));
    vtkDiscreteModelEdge* newEdge =
      vtkDiscreteModelEdge::SafeDownCast(model->BuildModelEdge(0, 0, newModelEdgeId));
    model->InvokeModelGeometricEntityEvent(ModelGeometricEntityCreated, newEdge);
    edge->SplitModelEdge(newVertex, newEdge);
    this->SetCreatedModelEdgeId(newModelEdgeId);
    this->SetCreatedModelVertexId(newModelVertexId);

    vtkNew<vtkSplitEventData> splitEventData;
    splitEventData->SetSourceEntity(edge);
    vtkNew<vtkIdList> createdEntityIds;
    createdEntityIds->InsertNextId(newModelVertexId);
    createdEntityIds->InsertNextId(newModelEdgeId);
    splitEventData->SetCreatedModelEntityIds(createdEntityIds.GetPointer());
    model->InvokeModelGeometricEntityEvent(
      ModelGeometricEntityBoundaryModified, edge);
    model->InvokeModelGeometricEntityEvent(ModelGeometricEntitySplit,
                                           splitEventData.GetPointer());

    vtkModelItemIterator* faces = newEdge->NewAdjacentModelFaceIterator();
    for(faces->Begin();!faces->IsAtEnd();faces->Next())
      {
      vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
      model->InvokeModelGeometricEntityEvent(
        ModelGeometricEntityBoundaryModified, face);
      }
    faces->Delete();
    }
  else
    { // a loop without a vertex in it is getting split
    bool returnVal = edge->SplitModelEdgeLoop(-1);
    if(vtkModelVertex* newVertex = edge->GetAdjacentModelVertex(0))
      {
      if(newModelVertexId != newVertex->GetUniquePersistentId())
        {
        vtkErrorMacro("Inconsistent information between server and client model.");
        return 0;
        }
      this->SetCreatedModelEdgeId(-1);
      this->SetCreatedModelVertexId(newModelVertexId);
      vtkNew<vtkIdList> createdEntityIds;
      createdEntityIds->InsertNextId(newModelVertexId);
      vtkNew<vtkSplitEventData> splitEventData;
      splitEventData->SetSourceEntity(edge);
      splitEventData->SetCreatedModelEntityIds(createdEntityIds.GetPointer());
      model->InvokeModelGeometricEntityEvent(ModelGeometricEntitySplit,
                                             splitEventData.GetPointer());
      }
    return returnVal;
    }

  return 1;
}

void vtkEdgeSplitOperatorClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
