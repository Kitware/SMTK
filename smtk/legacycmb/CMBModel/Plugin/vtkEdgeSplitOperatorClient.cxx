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

#include "vtkEdgeSplitOperatorClient.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelVertex.h"
#include <vtkIdList.h>
#include "vtkModelFace.h"
#include "vtkModelItemIterator.h"
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSMIdTypeVectorProperty.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMOperatorProxy.h>
#include <vtkSmartPointer.h>
#include <vtkSMProxyManager.h>
#include "vtkSplitEventData.h"

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
