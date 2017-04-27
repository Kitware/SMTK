//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBImportBCFileOperatorClient.h"

#include "vtkDiscreteModel.h"
#include "vtkObjectFactory.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSmartPointer.h"
#include "vtkXMLModelReader.h"

#include <sstream>

vtkStandardNewMacro(vtkCMBImportBCFileOperatorClient);

vtkCMBImportBCFileOperatorClient::vtkCMBImportBCFileOperatorClient()
{
  this->FileName = 0;
}

vtkCMBImportBCFileOperatorClient::~vtkCMBImportBCFileOperatorClient()
{
  this->SetFileName(0);
}

bool vtkCMBImportBCFileOperatorClient::Operate(
  vtkDiscreteModel* model, vtkSMProxy* serverModelProxy)
{
  if (!this->AbleToOperate(model))
  {
    return 0;
  }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* operatorProxy =
    vtkSMOperatorProxy::SafeDownCast(manager->NewProxy("CMBModelGroup", "CmbImportBCFileOperator"));
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

bool vtkCMBImportBCFileOperatorClient::AbleToOperate(vtkDiscreteModel* model)
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

void vtkCMBImportBCFileOperatorClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << endl;
}
