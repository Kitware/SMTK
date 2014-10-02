//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelStateOperatorClient.h"

#include "vtkDiscreteModel.h"
#include "vtkCMBModelBuilderClient.h"
#include "vtkIdList.h"
#include "vtkObjectFactory.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMProxyManager.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkStringArray.h"

vtkStandardNewMacro(vtkCMBModelStateOperatorClient);

//-----------------------------------------------------------------------------
vtkCMBModelStateOperatorClient::vtkCMBModelStateOperatorClient()
{
  this->OperatorProxy = NULL;
}

//-----------------------------------------------------------------------------
vtkCMBModelStateOperatorClient::~vtkCMBModelStateOperatorClient()
{
  if(this->OperatorProxy)
    {
    this->OperatorProxy->Delete();
    this->OperatorProxy = NULL;
    }
}

//-----------------------------------------------------------------------------
int vtkCMBModelStateOperatorClient::SaveModelState(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if(Model == NULL || ServerModelProxy == NULL)
    {
    return 0;
    }

  if(!this->OperatorProxy)
    {
    vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
    this->OperatorProxy =
      vtkSMOperatorProxy::SafeDownCast(
        manager->NewProxy("CMBModelGroup", "CMBModelStateOperator"));
    }
  if(!this->OperatorProxy)
    {
    vtkErrorMacro("Unable to CMBModelStateOperator proxy.");
    return 0;
    }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkSMIntVectorProperty* modeproperty =
    vtkSMIntVectorProperty::SafeDownCast(
      this->OperatorProxy->GetProperty("OperatorMode"));
  modeproperty->SetElement(0, 0);

  this->OperatorProxy->Operate(Model, ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      this->OperatorProxy->GetProperty("OperateSucceeded"));

  this->OperatorProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  if(!Succeeded)
    {
    vtkErrorMacro("Server side operator failed to save state.");
    return 0;
    }

  return 1;
}

//-----------------------------------------------------------------------------
int vtkCMBModelStateOperatorClient::LoadModelState(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if(Model == NULL || ServerModelProxy == NULL || this->OperatorProxy == NULL)
    {
    return 0;
    }
  vtkSMIntVectorProperty* modeproperty =
    vtkSMIntVectorProperty::SafeDownCast(
      this->OperatorProxy->GetProperty("OperatorMode"));
  modeproperty->SetElement(0, 1);

  this->OperatorProxy->Operate(Model, ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      this->OperatorProxy->GetProperty("OperateSucceeded"));

  this->OperatorProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  if(!Succeeded)
    {
    vtkErrorMacro("Server side operator failed to load state.");
    return 0;
    }

  return vtkCMBModelBuilderClient::UpdateClientModel(Model, ServerModelProxy);
}

//-----------------------------------------------------------------------------
vtkStringArray* vtkCMBModelStateOperatorClient::GetSerializedModelString()
{
  // update the copy of serialized model on client
  vtkSMStringVectorProperty* smSerializedModel =
    vtkSMStringVectorProperty::SafeDownCast(
    this->OperatorProxy->GetProperty("SerializedModelString"));

  if(!smSerializedModel)
    {
    cerr << "Cannot get SerializedModelString property in wrapper proxy.\n";
    return NULL;
    }

  this->OperatorProxy->UpdatePropertyInformation(smSerializedModel);
  const char* data = smSerializedModel->GetElement(0);
  this->SerializedModelString->Reset();
  this->SerializedModelString->SetNumberOfTuples(1);
  this->SerializedModelString->SetValue(0, data);
  return this->SerializedModelString;
}

//-----------------------------------------------------------------------------
void vtkCMBModelStateOperatorClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
