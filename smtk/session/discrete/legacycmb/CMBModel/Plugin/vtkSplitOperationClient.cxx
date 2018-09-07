//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkSplitOperationClient.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdTypeArray.h"
#include "vtkModelUserName.h"
#include "vtkSelectionSplitOperationClient.h" // for cliet model update
#
#include "vtkModelFaceUse.h"
#include "vtkModelShellUse.h"
#include "vtkObjectFactory.h"
#
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperationProxy.h"
#include "vtkSMProxyManager.h"

vtkStandardNewMacro(vtkSplitOperationClient);

vtkSplitOperationClient::vtkSplitOperationClient()
{
}

vtkSplitOperationClient::~vtkSplitOperationClient()
{
}

bool vtkSplitOperationClient::Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if (!this->AbleToOperate(Model))
  {
    return 0;
  }

  vtkModelEntity* Entity = this->GetModelEntity(Model);
  vtkDiscreteModelFace* Face = vtkDiscreteModelFace::SafeDownCast(Entity);

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperationProxy* OperationProxy =
    vtkSMOperationProxy::SafeDownCast(manager->NewProxy("CMBModelGroup", "SplitOperation"));
  if (!OperationProxy)
  {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
  }
  OperationProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkSMDoubleVectorProperty* angleproperty =
    vtkSMDoubleVectorProperty::SafeDownCast(OperationProxy->GetProperty("FeatureAngle"));
  angleproperty->SetElement(0, this->GetFeatureAngle());

  vtkSMIdTypeVectorProperty* idproperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(OperationProxy->GetProperty("Id"));
  idproperty->SetElement(0, this->GetId());

  OperationProxy->Operate(Model, ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(OperationProxy->GetProperty("OperateSucceeded"));

  OperationProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  if (!Succeeded)
  {
    vtkErrorMacro("Server side operator failed.");
    OperationProxy->Delete();
    return 0;
  }

  // now update the information on the client
  vtkSMIdTypeVectorProperty* CreatedModelFaces =
    vtkSMIdTypeVectorProperty::SafeDownCast(OperationProxy->GetProperty("CreatedModelFaceIDs"));
  OperationProxy->UpdateVTKObjects();
  OperationProxy->UpdatePropertyInformation();

  vtkSMIdTypeVectorProperty* CurrentNewFaceId =
    vtkSMIdTypeVectorProperty::SafeDownCast(OperationProxy->GetProperty("CurrentNewFaceId"));
  vtkSMIdTypeVectorProperty* SplitEdgeVertIds =
    vtkSMIdTypeVectorProperty::SafeDownCast(OperationProxy->GetProperty("SplitEdgeVertIds"));
  vtkSMIdTypeVectorProperty* CreatedModelEdgeVertIDs =
    vtkSMIdTypeVectorProperty::SafeDownCast(OperationProxy->GetProperty("CreatedModelEdgeVertIDs"));
  vtkSMIdTypeVectorProperty* FaceEdgeLoopIDs =
    vtkSMIdTypeVectorProperty::SafeDownCast(OperationProxy->GetProperty("FaceEdgeLoopIDs"));

  // CreatedModelFaces should now have the actual ids of the
  // created model faces.
  unsigned int NumberOfElements = CreatedModelFaces->GetNumberOfElements();
  vtkModelShellUse* ShellUses[2];
  for (int i = 0; i < 2; i++)
  {
    vtkModelFaceUse* FaceUse = Face->GetModelFaceUse(i);
    ShellUses[i] = FaceUse->GetModelShellUse();
  }

  bool hasEdges = Face->GetNumberOfModelEdges() != 0;
  vtkIdTypeArray* NewModelFaces = this->GetCreatedModelFaceIDs();
  NewModelFaces->Reset();
  NewModelFaces->SetNumberOfComponents(1);
  NewModelFaces->SetNumberOfTuples(NumberOfElements);
  for (unsigned int ui = 0; ui < NumberOfElements; ui++)
  {
    NewModelFaces->SetValue(ui, CreatedModelFaces->GetElement(ui));
    vtkModelFace* newFace = Model->BuildModelFace(0, 0, 0);
    vtkIdType newFId = newFace->GetUniquePersistentId();
    if (newFId != CreatedModelFaces->GetElement(ui))
    {
      vtkErrorMacro("Created model face ids do not match on server and client.");
    }
    for (int i = 0; i < 2; i++)
    {
      vtkModelFaceUse* FaceUse = newFace->GetModelFaceUse(i);
      if (ShellUses[i])
      {
        ShellUses[i]->AddModelFaceUse(FaceUse);
      }
    }

    // update Edges due to split of faces. We need to follow exactly what's
    // happenning on server side so that the Ids will match between server and client.
    if (hasEdges)
    {
      CurrentNewFaceId->SetElement(0, newFId);
      OperationProxy->UpdateVTKObjects();
      OperationProxy->UpdatePropertyInformation();

      vtkSelectionSplitOperationClient::UpdateSplitEdgeVertIds(
        Model, SplitEdgeVertIds, this->SplitEdgeMap, this->SplitVertMap);
      vtkSelectionSplitOperationClient::UpdateCreatedModelEdgeVertIDs(
        Model, CreatedModelEdgeVertIDs, this->NewEdges, this->NewVerts);
      vtkSelectionSplitOperationClient::UpdateFaceEdgeLoopIDs(Model, FaceEdgeLoopIDs);
    }
  }

  OperationProxy->Delete();

  return 1;
}

void vtkSplitOperationClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
