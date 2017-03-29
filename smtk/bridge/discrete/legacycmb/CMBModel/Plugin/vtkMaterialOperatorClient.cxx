//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkMaterialOperatorClient.h"

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
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkMaterialOperatorClient);

vtkMaterialOperatorClient::vtkMaterialOperatorClient()
{
}

vtkMaterialOperatorClient::~vtkMaterialOperatorClient()
{
}

bool vtkMaterialOperatorClient::Operate(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if(!this->AbleToOperate(Model))
    {
    return 0;
    }

  // first try to do the operation on the server
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "MaterialOperator"));
  if(!OperatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
    }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkSMIdTypeVectorProperty* idproperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("Id"));
  idproperty->SetElement(0, this->GetId());

  if(this->GetIsVisibilitySet())
    {
    vtkSMIntVectorProperty* visibilityproperty =
      vtkSMIntVectorProperty::SafeDownCast(
        OperatorProxy->GetProperty("Visibility"));
    visibilityproperty->SetElement(0, this->GetVisibility());
    }

  if(this->GetIsRGBASet())
    {
    double * rgba = this->GetRGBA();
    vtkSMDoubleVectorProperty* rgbaproperty =
      vtkSMDoubleVectorProperty::SafeDownCast(
        OperatorProxy->GetProperty("RGBA"));
    for(int j=0;j<4;j++)
      {
      rgbaproperty->SetElement(j, rgba[j]);
      }
    }
  if(this->GetUserName())
    {
    vtkSMStringVectorProperty* strproperty =
      vtkSMStringVectorProperty::SafeDownCast(
        OperatorProxy->GetProperty("UserName"));
    strproperty->SetElement(0, this->GetUserName());
    strproperty->SetElementType(0, vtkSMStringVectorProperty::STRING);
    }
  unsigned int NumberOfGeometricEntities =
    this->GetGeometricEntitiesToAdd()->GetNumberOfIds();
  if(NumberOfGeometricEntities)
    {
    vtkSMIdTypeVectorProperty* addedentitiesproperty =
      vtkSMIdTypeVectorProperty::SafeDownCast(
        OperatorProxy->GetProperty("AddModelGeometricEntity"));
    for(unsigned int ui=0;ui<NumberOfGeometricEntities;ui++)
      {
      addedentitiesproperty->SetElement(
        ui, this->GetGeometricEntitiesToAdd()->GetId(ui));
      }
    }

  OperatorProxy->Operate(Model, ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("OperateSucceeded"));

  OperatorProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  OperatorProxy->Delete();
  OperatorProxy = 0;
  if(!Succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    return 0;
    }

  return this->Superclass::Operate(Model);
}

vtkIdType vtkMaterialOperatorClient::Build(vtkDiscreteModel* Model,
                                           vtkSMProxy* ServerModelProxy)
{
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "MaterialOperator"));
  if(!OperatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return -1;
    }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkIdType ServerMaterialId =
    OperatorProxy->Build(Model, ServerModelProxy);
  vtkIdType ClientMaterialId = this->Superclass::Build(Model);
  OperatorProxy->Delete();
  if(ServerMaterialId != ClientMaterialId)
    {
    vtkErrorMacro("Created material does not have matching Ids on the server and client.");
    return -1;
    }
  this->SetId(ClientMaterialId);
  return ClientMaterialId;
}

bool vtkMaterialOperatorClient::Destroy(vtkDiscreteModel* Model,
                                        vtkSMProxy* ServerModelProxy)
{
  if(this->GetIsIdSet() == 0)
    {
    vtkWarningMacro("Must specify Id of material to destroy.");
    return 0;
    }
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "MaterialOperator"));
  if(!OperatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
    }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  // now set the id of the material to be destroyed on the server
  vtkSMIdTypeVectorProperty* idproperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("Id"));
  idproperty->SetElement(0, this->GetId());

  bool Success = OperatorProxy->Destroy(Model, ServerModelProxy);
  OperatorProxy->Delete();
  if(!Success)
    {
    vtkWarningMacro("Could not delete material on server.");
    return 0;
    }

  return this->Superclass::Destroy(Model);
}

void vtkMaterialOperatorClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
