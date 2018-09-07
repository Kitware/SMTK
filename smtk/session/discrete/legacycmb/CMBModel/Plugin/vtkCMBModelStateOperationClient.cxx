//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelStateOperationClient.h"

#include "vtkCMBModelBuilderClient.h"
#include "vtkDiscreteModel.h"
#include "vtkIdList.h"
#include "vtkObjectFactory.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperationProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkStringArray.h"

vtkStandardNewMacro(vtkCMBModelStateOperationClient);

vtkCMBModelStateOperationClient::vtkCMBModelStateOperationClient()
{
  this->OperationProxy = NULL;
}

vtkCMBModelStateOperationClient::~vtkCMBModelStateOperationClient()
{
  if (this->OperationProxy)
  {
    this->OperationProxy->Delete();
    this->OperationProxy = NULL;
  }
}

int vtkCMBModelStateOperationClient::SaveModelState(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if (Model == NULL || ServerModelProxy == NULL)
  {
    return 0;
  }

  if (!this->OperationProxy)
  {
    vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
    this->OperationProxy = vtkSMOperationProxy::SafeDownCast(
      manager->NewProxy("CMBModelGroup", "CMBModelStateOperation"));
  }
  if (!this->OperationProxy)
  {
    vtkErrorMacro("Unable to CMBModelStateOperation proxy.");
    return 0;
  }
  OperationProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkSMIntVectorProperty* modeproperty =
    vtkSMIntVectorProperty::SafeDownCast(this->OperationProxy->GetProperty("OperationMode"));
  modeproperty->SetElement(0, 0);

  this->OperationProxy->Operate(Model, ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(this->OperationProxy->GetProperty("OperateSucceeded"));

  this->OperationProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  if (!Succeeded)
  {
    vtkErrorMacro("Server side operator failed to save state.");
    return 0;
  }

  return 1;
}

int vtkCMBModelStateOperationClient::LoadModelState(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if (Model == NULL || ServerModelProxy == NULL || this->OperationProxy == NULL)
  {
    return 0;
  }
  vtkSMIntVectorProperty* modeproperty =
    vtkSMIntVectorProperty::SafeDownCast(this->OperationProxy->GetProperty("OperationMode"));
  modeproperty->SetElement(0, 1);

  this->OperationProxy->Operate(Model, ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(this->OperationProxy->GetProperty("OperateSucceeded"));

  this->OperationProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  if (!Succeeded)
  {
    vtkErrorMacro("Server side operator failed to load state.");
    return 0;
  }

  return vtkCMBModelBuilderClient::UpdateClientModel(Model, ServerModelProxy);
}

vtkStringArray* vtkCMBModelStateOperationClient::GetSerializedModelString()
{
  // update the copy of serialized model on client
  vtkSMStringVectorProperty* smSerializedModel = vtkSMStringVectorProperty::SafeDownCast(
    this->OperationProxy->GetProperty("SerializedModelString"));

  if (!smSerializedModel)
  {
    cerr << "Cannot get SerializedModelString property in wrapper proxy.\n";
    return NULL;
  }

  this->OperationProxy->UpdatePropertyInformation(smSerializedModel);
  const char* data = smSerializedModel->GetElement(0);
  this->SerializedModelString->Reset();
  this->SerializedModelString->SetNumberOfTuples(1);
  this->SerializedModelString->SetValue(0, data);
  return this->SerializedModelString;
}

void vtkCMBModelStateOperationClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
