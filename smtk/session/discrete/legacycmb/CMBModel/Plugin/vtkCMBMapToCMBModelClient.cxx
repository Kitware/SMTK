//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBMapToCMBModelClient.h"

#include "vtkDiscreteModel.h"
#include "vtkObjectFactory.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperationProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSmartPointer.h"

#include "vtkCMBModelBuilderClient.h"

vtkStandardNewMacro(vtkCMBMapToCMBModelClient);

vtkCMBMapToCMBModelClient::vtkCMBMapToCMBModelClient()
{
}

vtkCMBMapToCMBModelClient::~vtkCMBMapToCMBModelClient()
{
}

bool vtkCMBMapToCMBModelClient::Operate(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy, vtkSMProxy* PolySourceProxy)
{
  if (!this->AbleToOperate(Model) || PolySourceProxy == NULL || ServerModelProxy == NULL)
  {
    return 0;
  }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperationProxy* OperationProxy =
    vtkSMOperationProxy::SafeDownCast(manager->NewProxy("CMBModelGroup", "vtkCMBMapToCMBModel"));
  if (!OperationProxy)
  {
    vtkErrorMacro("Unable to create builder operator proxy.");
    return 0;
  }
  OperationProxy->SetLocation(ServerModelProxy->GetLocation());

  OperationProxy->Operate(Model, ServerModelProxy, PolySourceProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(OperationProxy->GetProperty("OperateSucceeded"));

  OperationProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  OperationProxy->Delete();
  OperationProxy = 0;
  if (!Succeeded)
  {
    return 0;
  }

  return vtkCMBModelBuilderClient::UpdateClientModel(Model, ServerModelProxy);
}

bool vtkCMBMapToCMBModelClient::AbleToOperate(vtkDiscreteModel* Model)
{
  if (!Model)
  {
    vtkErrorMacro("Passed in a null model.");
    return 0;
  }
  return 1;
}

void vtkCMBMapToCMBModelClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
