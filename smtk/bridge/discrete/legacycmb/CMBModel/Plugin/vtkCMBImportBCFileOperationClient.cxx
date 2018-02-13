//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBImportBCFileOperationClient.h"

#include "vtkDiscreteModel.h"
#include "vtkObjectFactory.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperationProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSmartPointer.h"
#include "vtkXMLModelReader.h"

#include <sstream>

vtkStandardNewMacro(vtkCMBImportBCFileOperationClient);

vtkCMBImportBCFileOperationClient::vtkCMBImportBCFileOperationClient()
{
  this->FileName = 0;
}

vtkCMBImportBCFileOperationClient::~vtkCMBImportBCFileOperationClient()
{
  this->SetFileName(0);
}

bool vtkCMBImportBCFileOperationClient::Operate(
  vtkDiscreteModel* model, vtkSMProxy* serverModelProxy)
{
  if (!this->AbleToOperate(model))
  {
    return 0;
  }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperationProxy* operatorProxy = vtkSMOperationProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "CmbImportBCFileOperation"));
  if (!operatorProxy)
  {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
  }
  operatorProxy->SetLocation(serverModelProxy->GetLocation());

  vtkSMStringVectorProperty* strproperty =
    vtkSMStringVectorProperty::SafeDownCast(operatorProxy->GetProperty("FileName"));
  strproperty->SetElement(0, this->GetFileName());
  strproperty->SetElementType(0, vtkSMStringVectorProperty::STRING);

  operatorProxy->Operate(model, serverModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* operateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(operatorProxy->GetProperty("OperateSucceeded"));

  operatorProxy->UpdatePropertyInformation();

  int succeeded = operateSucceeded->GetElement(0);
  operatorProxy->Delete();
  operatorProxy = 0;
  if (!succeeded)
  {
    vtkErrorMacro("Server side operator failed.");
    return 0;
  }

  return 1;
}

bool vtkCMBImportBCFileOperationClient::AbleToOperate(vtkDiscreteModel* model)
{
  if (!model)
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

void vtkCMBImportBCFileOperationClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << endl;
}
