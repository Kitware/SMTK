//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkSplitOperatorClient.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdTypeArray.h"
#include "vtkModelUserName.h"
#include "vtkSelectionSplitOperatorClient.h" // for cliet model update
#
#include "vtkModelFaceUse.h"
#include "vtkModelShellUse.h"
#include "vtkObjectFactory.h"
#
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"

vtkStandardNewMacro(vtkSplitOperatorClient);

vtkSplitOperatorClient::vtkSplitOperatorClient()
{
}

vtkSplitOperatorClient::~vtkSplitOperatorClient()
{
}

bool vtkSplitOperatorClient::Operate(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if(!this->AbleToOperate(Model))
    {
    return 0;
    }

  vtkModelEntity* Entity = this->GetModelEntity(Model);
  vtkDiscreteModelFace* Face = vtkDiscreteModelFace::SafeDownCast(Entity);

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy =
    vtkSMOperatorProxy::SafeDownCast(
      manager->NewProxy("CMBModelGroup", "SplitOperator"));
  if(!OperatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
    }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkSMDoubleVectorProperty* angleproperty =
    vtkSMDoubleVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("FeatureAngle"));
  angleproperty->SetElement(0, this->GetFeatureAngle());

  vtkSMIdTypeVectorProperty* idproperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("Id"));
  idproperty->SetElement(0, this->GetId());

  OperatorProxy->Operate(Model, ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("OperateSucceeded"));

  OperatorProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  if(!Succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    OperatorProxy->Delete();
    return 0;
    }

  // now update the information on the client
  vtkSMIdTypeVectorProperty* CreatedModelFaces =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("CreatedModelFaceIDs"));
  OperatorProxy->UpdateVTKObjects();
  OperatorProxy->UpdatePropertyInformation();

  vtkSMIdTypeVectorProperty* CurrentNewFaceId =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("CurrentNewFaceId"));
  vtkSMIdTypeVectorProperty* SplitEdgeVertIds =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("SplitEdgeVertIds"));
  vtkSMIdTypeVectorProperty* CreatedModelEdgeVertIDs =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("CreatedModelEdgeVertIDs"));
  vtkSMIdTypeVectorProperty* FaceEdgeLoopIDs =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("FaceEdgeLoopIDs"));

  // CreatedModelFaces should now have the actual ids of the
  // created model faces.
  unsigned int NumberOfElements = CreatedModelFaces->GetNumberOfElements();
  vtkModelShellUse* ShellUses[2];
  for(int i=0;i<2;i++)
    {
    vtkModelFaceUse* FaceUse = Face->GetModelFaceUse(i);
    ShellUses[i] = FaceUse->GetModelShellUse();
    }

  bool hasEdges = Face->GetNumberOfModelEdges() != 0;
  vtkIdTypeArray* NewModelFaces = this->GetCreatedModelFaceIDs();
  NewModelFaces->Reset();
  NewModelFaces->SetNumberOfComponents(1);
  NewModelFaces->SetNumberOfTuples(NumberOfElements);
  for(unsigned int ui=0;ui<NumberOfElements;ui++)
    {
    NewModelFaces->SetValue(ui, CreatedModelFaces->GetElement(ui));
    vtkModelFace* newFace = Model->BuildModelFace(0, 0, 0);
    vtkIdType newFId = newFace->GetUniquePersistentId();
    if( newFId != CreatedModelFaces->GetElement(ui))
      {
      vtkErrorMacro("Created model face ids do not match on server and client.");
      }
    for(int i=0;i<2;i++)
      {
      vtkModelFaceUse* FaceUse = newFace->GetModelFaceUse(i);
      if(ShellUses[i])
        {
        ShellUses[i]->AddModelFaceUse(FaceUse);
        }
      }

    // update Edges due to split of faces. We need to follow exactly what's
    // happenning on server side so that the Ids will match between server and client.
    if(hasEdges)
      {
      CurrentNewFaceId->SetElement(0, newFId);
      OperatorProxy->UpdateVTKObjects();
      OperatorProxy->UpdatePropertyInformation();

      vtkSelectionSplitOperatorClient::UpdateSplitEdgeVertIds(
        Model, SplitEdgeVertIds, this->SplitEdgeMap, this->SplitVertMap);
      vtkSelectionSplitOperatorClient::UpdateCreatedModelEdgeVertIDs(
        Model, CreatedModelEdgeVertIDs, this->NewEdges, this->NewVerts);
      vtkSelectionSplitOperatorClient::UpdateFaceEdgeLoopIDs(
        Model, FaceEdgeLoopIDs);
      }
    }

  OperatorProxy->Delete();

  return 1;
}

void vtkSplitOperatorClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
