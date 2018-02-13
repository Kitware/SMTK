//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkMergeOperationClient.h"

#include "vtkDiscreteModel.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperationProxy.h"
#include "vtkSmartPointer.h"
#
#include "vtkSMProxyManager.h"

vtkStandardNewMacro(vtkMergeOperationClient);

vtkMergeOperationClient::vtkMergeOperationClient()
{
}

vtkMergeOperationClient::~vtkMergeOperationClient()
{
}

bool vtkMergeOperationClient::Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if (!this->AbleToOperate(Model))
  {
    return 0;
  }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperationProxy* OperationProxy =
    vtkSMOperationProxy::SafeDownCast(manager->NewProxy("CMBModelGroup", "MergeOperation"));
  if (!OperationProxy)
  {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
  }
  OperationProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkSMIdTypeVectorProperty* targetIdProperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(OperationProxy->GetProperty("TargetId"));
  targetIdProperty->SetElement(0, this->GetTargetId());

  vtkSMIdTypeVectorProperty* sourceIdProperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(OperationProxy->GetProperty("SourceId"));
  sourceIdProperty->SetElement(0, this->GetSourceId());

  vtkIdTypeArray* lowerDimensionalIds = this->GetLowerDimensionalIds();
  vtkSMIdTypeVectorProperty* lowerDimensionalIdsProperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(OperationProxy->GetProperty("LowerDimensionalIds"));
  for (vtkIdType i = 0; i < lowerDimensionalIds->GetNumberOfTuples(); i++)
  {
    lowerDimensionalIdsProperty->SetElement(i, lowerDimensionalIds->GetValue(i));
  }

  OperationProxy->Operate(Model, ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(OperationProxy->GetProperty("OperateSucceeded"));

  OperationProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  OperationProxy->Delete();
  OperationProxy = 0;
  if (!Succeeded)
  {
    vtkErrorMacro("Server side operator failed.");
    return 0;
  }

  return this->Superclass::Operate(Model);
}

void vtkMergeOperationClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
