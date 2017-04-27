//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelReadOperatorClient.h"

#include "vtkDiscreteModel.h"
#include "vtkObjectFactory.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSmartPointer.h"

#include "vtkCMBModelBuilderClient.h"

vtkStandardNewMacro(vtkCMBModelReadOperatorClient);

vtkCMBModelReadOperatorClient::vtkCMBModelReadOperatorClient()
{
  this->FileName = 0;
}

vtkCMBModelReadOperatorClient::~vtkCMBModelReadOperatorClient()
{
  if (this->FileName)
  {
    this->SetFileName(0);
  }
}

bool vtkCMBModelReadOperatorClient::Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if (!this->AbleToOperate(Model))
  {
    return 0;
  }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy =
    vtkSMOperatorProxy::SafeDownCast(manager->NewProxy("CMBModelGroup", "CMBModelReadOperator"));
  if (!OperatorProxy)
  {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
  }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkSMStringVectorProperty* strproperty =
    vtkSMStringVectorProperty::SafeDownCast(OperatorProxy->GetProperty("FileName"));
  strproperty->SetElement(0, this->GetFileName());
  strproperty->SetElementType(0, vtkSMStringVectorProperty::STRING);

  OperatorProxy->Operate(Model, ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(OperatorProxy->GetProperty("OperateSucceeded"));

  OperatorProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  OperatorProxy->Delete();
  OperatorProxy = 0;
  if (!Succeeded)
  {
    vtkErrorMacro("Server side operator failed.");
    return 0;
  }

  return vtkCMBModelBuilderClient::UpdateClientModel(Model, ServerModelProxy);
}

bool vtkCMBModelReadOperatorClient::AbleToOperate(vtkDiscreteModel* Model)
{
  if (!Model)
  {
    vtkErrorMacro("Passed in a null model.");
    return 0;
  }
  if (this->GetFileName() == 0)
  {
    vtkErrorMacro("Must specify a FileName.");
    return 0;
  }

  return 1;
}

void vtkCMBModelReadOperatorClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << endl;
}
