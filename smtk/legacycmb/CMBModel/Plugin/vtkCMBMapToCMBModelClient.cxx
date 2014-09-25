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

#include "vtkCMBMapToCMBModelClient.h"

#include "vtkDiscreteModel.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"

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
