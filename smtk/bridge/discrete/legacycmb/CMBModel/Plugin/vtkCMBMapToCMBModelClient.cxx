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
#include "vtkSMOperatorProxy.h"
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

bool vtkCMBMapToCMBModelClient::Operate(vtkDiscreteModel* Model,
    vtkSMProxy* ServerModelProxy, vtkSMProxy* PolySourceProxy)
{
  if(!this->AbleToOperate(Model)|| PolySourceProxy == NULL ||
      ServerModelProxy == NULL)
    {
    return 0;
    }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy = vtkSMOperatorProxy::SafeDownCast(
      manager->NewProxy("CMBModelGroup", "vtkCMBMapToCMBModel"));
  if(!OperatorProxy)
    {
    vtkErrorMacro("Unable to create builder operator proxy.");
    return 0;
    }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  OperatorProxy->Operate(Model, ServerModelProxy, PolySourceProxy);

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
    return 0;
    }

  return vtkCMBModelBuilderClient::UpdateClientModel(Model, ServerModelProxy);
}


bool vtkCMBMapToCMBModelClient::AbleToOperate(vtkDiscreteModel* Model)
{
  if(!Model)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }
  return 1;
}


void vtkCMBMapToCMBModelClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
