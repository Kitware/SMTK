//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkMaterialOperationClient.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkModelMaterial.h"
#
#include "vtkDiscreteModelRegion.h"
#include "vtkIdList.h"
#
#
#include "vtkObjectFactory.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperationProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkMaterialOperationClient);

vtkMaterialOperationClient::vtkMaterialOperationClient()
{
}

vtkMaterialOperationClient::~vtkMaterialOperationClient()
{
}

bool vtkMaterialOperationClient::Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if (!this->AbleToOperate(Model))
  {
    return 0;
  }

  // first try to do the operation on the server
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperationProxy* OperationProxy =
    vtkSMOperationProxy::SafeDownCast(manager->NewProxy("CMBModelGroup", "MaterialOperation"));
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
  unsigned int NumberOfGeometricEntities = this->GetGeometricEntitiesToAdd()->GetNumberOfIds();
  if (NumberOfGeometricEntities)
  {
    vtkSMIdTypeVectorProperty* addedentitiesproperty = vtkSMIdTypeVectorProperty::SafeDownCast(
      OperationProxy->GetProperty("AddModelGeometricEntity"));
    for (unsigned int ui = 0; ui < NumberOfGeometricEntities; ui++)
    {
      addedentitiesproperty->SetElement(ui, this->GetGeometricEntitiesToAdd()->GetId(ui));
    }
  }

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

vtkIdType vtkMaterialOperationClient::Build(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperationProxy* OperationProxy =
    vtkSMOperationProxy::SafeDownCast(manager->NewProxy("CMBModelGroup", "MaterialOperation"));
  if (!OperationProxy)
  {
    vtkErrorMacro("Unable to create operator proxy.");
    return -1;
  }
  OperationProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkIdType ServerMaterialId = OperationProxy->Build(Model, ServerModelProxy);
  vtkIdType ClientMaterialId = this->Superclass::Build(Model);
  OperationProxy->Delete();
  if (ServerMaterialId != ClientMaterialId)
  {
    vtkErrorMacro("Created material does not have matching Ids on the server and client.");
    return -1;
  }
  this->SetId(ClientMaterialId);
  return ClientMaterialId;
}

bool vtkMaterialOperationClient::Destroy(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if (this->GetIsIdSet() == 0)
  {
    vtkWarningMacro("Must specify Id of material to destroy.");
    return 0;
  }
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperationProxy* OperationProxy =
    vtkSMOperationProxy::SafeDownCast(manager->NewProxy("CMBModelGroup", "MaterialOperation"));
  if (!OperationProxy)
  {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
  }
  OperationProxy->SetLocation(ServerModelProxy->GetLocation());

  // now set the id of the material to be destroyed on the server
  vtkSMIdTypeVectorProperty* idproperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(OperationProxy->GetProperty("Id"));
  idproperty->SetElement(0, this->GetId());

  bool Success = OperationProxy->Destroy(Model, ServerModelProxy);
  OperationProxy->Delete();
  if (!Success)
  {
    vtkWarningMacro("Could not delete material on server.");
    return 0;
  }

  return this->Superclass::Destroy(Model);
}

void vtkMaterialOperationClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
