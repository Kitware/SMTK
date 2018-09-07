//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelEntityGroupOperationClient.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEntity.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkIdList.h"
#
#include "vtkObjectFactory.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperationProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkModelEntityGroupOperationClient);

vtkModelEntityGroupOperationClient::vtkModelEntityGroupOperationClient()
{
}

vtkModelEntityGroupOperationClient::~vtkModelEntityGroupOperationClient()
{
}

bool vtkModelEntityGroupOperationClient::Operate(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if (!this->AbleToOperate(Model))
  {
    return 0;
  }

  // first try to do the operation on the server
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperationProxy* OperationProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "ModelEntityGroupOperation"));
  if (!OperationProxy)
  {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
  }
  OperationProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkSMIdTypeVectorProperty* idproperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(OperationProxy->GetProperty("Id"));
  idproperty->SetElement(0, this->GetId());

  if (this->GetIsVisibilitySet())
  {
    vtkSMIntVectorProperty* visibilityproperty =
      vtkSMIntVectorProperty::SafeDownCast(OperationProxy->GetProperty("Visibility"));
    visibilityproperty->SetElement(0, this->GetVisibility());
  }

  if (this->GetIsRGBASet())
  {
    double* rgba = this->GetRGBA();
    vtkSMDoubleVectorProperty* rgbaproperty =
      vtkSMDoubleVectorProperty::SafeDownCast(OperationProxy->GetProperty("RGBA"));
    for (int j = 0; j < 4; j++)
    {
      rgbaproperty->SetElement(j, rgba[j]);
    }
  }
  if (this->GetUserName())
  {
    vtkSMStringVectorProperty* strproperty =
      vtkSMStringVectorProperty::SafeDownCast(OperationProxy->GetProperty("UserName"));
    strproperty->SetElement(0, this->GetUserName());
    strproperty->SetElementType(0, vtkSMStringVectorProperty::STRING);
  }
  unsigned int numberOfEntities = this->GetEntitiesToAdd()->GetNumberOfIds();
  if (numberOfEntities)
  {
    vtkSMIdTypeVectorProperty* addedentitiesproperty =
      vtkSMIdTypeVectorProperty::SafeDownCast(OperationProxy->GetProperty("AddModelEntity"));
    for (unsigned int ui = 0; ui < numberOfEntities; ui++)
    {
      addedentitiesproperty->SetElement(ui, this->GetEntitiesToAdd()->GetId(ui));
    }
  }

  numberOfEntities = this->GetEntitiesToRemove()->GetNumberOfIds();
  if (numberOfEntities)
  {
    vtkSMIdTypeVectorProperty* removedentitiesproperty =
      vtkSMIdTypeVectorProperty::SafeDownCast(OperationProxy->GetProperty("RemoveModelEntity"));
    for (unsigned int ui = 0; ui < numberOfEntities; ui++)
    {
      removedentitiesproperty->SetElement(ui, this->GetEntitiesToRemove()->GetId(ui));
    }
  }

  vtkSMIntVectorProperty* buildtypeproperty =
    vtkSMIntVectorProperty::SafeDownCast(OperationProxy->GetProperty("BuildEnityType"));
  buildtypeproperty->SetElement(0, this->GetBuildEnityType());

  OperationProxy->Operate(Model, ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(OperationProxy->GetProperty("OperateSucceeded"));

  OperationProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  OperationProxy->Delete();
  OperationProxy = 0;
  if (!Succeeded)
  {
    vtkErrorMacro("Server side operator failed.");
    return 0;
  }

  return this->Superclass::Operate(Model);
}

vtkIdType vtkModelEntityGroupOperationClient::Build(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperationProxy* OperationProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "ModelEntityGroupOperation"));
  if (!OperationProxy)
  {
    vtkErrorMacro("Unable to create operator proxy.");
    return -1;
  }
  OperationProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkIdType ServerEntityGroupId = OperationProxy->Build(Model, ServerModelProxy);
  vtkIdType ClientEntityGroupId = this->Superclass::Build(Model);
  OperationProxy->Delete();
  if (ServerEntityGroupId != ClientEntityGroupId)
  {
    vtkErrorMacro("Created entity group does not have matching Ids on the server and client.");
    return -1;
  }
  this->SetId(ClientEntityGroupId);
  return ClientEntityGroupId;
}

bool vtkModelEntityGroupOperationClient::Destroy(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if (this->GetIsIdSet() == 0)
  {
    vtkWarningMacro("Must specify Id of entity group to destroy.");
    return 0;
  }
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperationProxy* OperationProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "ModelEntityGroupOperation"));
  if (!OperationProxy)
  {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
  }
  OperationProxy->SetLocation(ServerModelProxy->GetLocation());

  // now set the id of the entity group to be destroyed on the server
  vtkSMIdTypeVectorProperty* idproperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(OperationProxy->GetProperty("Id"));
  idproperty->SetElement(0, this->GetId());

  bool Success = OperationProxy->Destroy(Model, ServerModelProxy);
  OperationProxy->Delete();
  if (!Success)
  {
    vtkWarningMacro("Could not delete entity group on server.");
    return 0;
  }

  return this->Superclass::Destroy(Model);
}

void vtkModelEntityGroupOperationClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
