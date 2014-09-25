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

#include "vtkMergeOperatorClient.h"

#include "vtkDiscreteModel.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#
#include "vtkSMProxyManager.h"

vtkStandardNewMacro(vtkMergeOperatorClient);

vtkMergeOperatorClient::vtkMergeOperatorClient()
{
}

vtkMergeOperatorClient::~vtkMergeOperatorClient()
{
}

bool vtkMergeOperatorClient::Operate(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if(!this->AbleToOperate(Model))
    {
    return 0;
    }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "MergeOperator"));
  if(!OperatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
    }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkSMIdTypeVectorProperty* targetIdProperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("TargetId"));
  targetIdProperty->SetElement(0, this->GetTargetId());

  vtkSMIdTypeVectorProperty* sourceIdProperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("SourceId"));
  sourceIdProperty->SetElement(0, this->GetSourceId());

  vtkIdTypeArray* lowerDimensionalIds = this->GetLowerDimensionalIds();
  vtkSMIdTypeVectorProperty* lowerDimensionalIdsProperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("LowerDimensionalIds"));
  for(vtkIdType i=0;i<lowerDimensionalIds->GetNumberOfTuples();i++)
    {
    lowerDimensionalIdsProperty->SetElement(i, lowerDimensionalIds->GetValue(i));
    }

  OperatorProxy->Operate(Model, ServerModelProxy);

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
    vtkErrorMacro("Server side operator failed.");
    return 0;
    }

  return this->Superclass::Operate(Model);
}


void vtkMergeOperatorClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
