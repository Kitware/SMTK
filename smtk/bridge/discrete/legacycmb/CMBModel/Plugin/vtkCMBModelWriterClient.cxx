//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelWriterClient.h"

#include "vtkDiscreteModel.h"
#include "vtkObjectFactory.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkCMBModelWriterClient);

vtkCMBModelWriterClient::vtkCMBModelWriterClient()
{
  this->FileName = 0;
  this->Version = 5;
}

vtkCMBModelWriterClient::~vtkCMBModelWriterClient()
{
  this->SetFileName(0);
}

bool vtkCMBModelWriterClient::Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if (!this->GetFileName())
  {
    vtkWarningMacro("Must set file name.");
    return 0;
  }

  if (!Model)
  {
    vtkErrorMacro("Passed in a null model.");
    return 0;
  }
  if (this->Version != 2 && this->Version != 3 && this->Version != 4 && this->Version != 5)
  {
    vtkWarningMacro("Writing version " << this->Version << " not supported.");
    return 0;
  }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy =
    vtkSMOperatorProxy::SafeDownCast(manager->NewProxy("CMBModelGroup", "CMBModelWriter"));
  if (!OperatorProxy)
  {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
  }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkSMIntVectorProperty* versionproperty =
    vtkSMIntVectorProperty::SafeDownCast(OperatorProxy->GetProperty("Version"));
  versionproperty->SetElement(0, this->GetVersion());

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
  if (!Succeeded)
  {
    vtkErrorMacro("Server side operator failed.");
    return 0;
  }

  return 1;
}

void vtkCMBModelWriterClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << endl;
  os << indent << "Version: " << this->Version << endl;
}
