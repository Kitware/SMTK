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

#include "vtkCMBModelWriterClient.h"

#include "vtkDiscreteModel.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"

vtkStandardNewMacro(vtkCMBModelWriterClient);

vtkCMBModelWriterClient::vtkCMBModelWriterClient()
{
  this->FileName = 0;
  this->Version = 5;
}

vtkCMBModelWriterClient:: ~vtkCMBModelWriterClient()
{
  this->SetFileName(0);
}

bool vtkCMBModelWriterClient::Operate(vtkDiscreteModel* Model,
                                      vtkSMProxy* ServerModelProxy)
{
  if(!this->GetFileName())
    {
    vtkWarningMacro("Must set file name.");
    return 0;
    }

  if(!Model)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }
  if(this->Version != 2 && this->Version != 3 &&
     this->Version != 4 && this->Version != 5)
    {
    vtkWarningMacro("Writing version " << this->Version << " not supported.");
    return 0;
    }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "CMBModelWriter"));
  if(!OperatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
    }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkSMIntVectorProperty* versionproperty =
    vtkSMIntVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("Version"));
  versionproperty->SetElement(0, this->GetVersion());

  vtkSMStringVectorProperty* strproperty =
    vtkSMStringVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("FileName"));
  strproperty->SetElement(0, this->GetFileName());
  strproperty->SetElementType(0, vtkSMStringVectorProperty::STRING);

  OperatorProxy->Operate(Model, ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("OperateSucceeded"));

  OperatorProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  OperatorProxy->Delete();
  if(!Succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    return 0;
    }

  return 1;
}

void vtkCMBModelWriterClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " << this->FileName << endl;
  os << indent << "Version: " << this->Version << endl;
}
