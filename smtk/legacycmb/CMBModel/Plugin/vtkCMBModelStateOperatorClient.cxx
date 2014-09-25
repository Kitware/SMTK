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
