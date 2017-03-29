//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkMergeOperatorClient.h"

#include "vtkDiscreteModel.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSmartPointer.h"
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
