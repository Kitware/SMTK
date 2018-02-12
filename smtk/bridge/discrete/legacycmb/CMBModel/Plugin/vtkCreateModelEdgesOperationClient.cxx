//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCreateModelEdgesOperationClient.h"

#include "vtkCMBModelBuilderClient.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdTypeArray.h"
#include "vtkIdTypeArray.h"
#include "vtkModelFaceUse.h"
#include "vtkModelShellUse.h"
#include "vtkModelUserName.h"
#include "vtkObjectFactory.h"

#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperationProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"

vtkStandardNewMacro(vtkCreateModelEdgesOperationClient);

vtkCreateModelEdgesOperationClient::vtkCreateModelEdgesOperationClient()
{
}

vtkCreateModelEdgesOperationClient::~vtkCreateModelEdgesOperationClient()
{
}

bool vtkCreateModelEdgesOperationClient::Operate(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy, const char* strSerializedModel)
{
  if (!this->AbleToOperate(Model))
  {
    return 0;
  }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperationProxy* OperationProxy =
    vtkSMOperationProxy::SafeDownCast(manager->NewProxy("CMBModelGroup", "CreateModelEdges"));
  if (!OperationProxy)
  {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
  }
  OperationProxy->SetLocation(ServerModelProxy->GetLocation());

  OperationProxy->Operate(Model, ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(OperationProxy->GetProperty("OperateSucceeded"));

  OperationProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  if (!Succeeded)
  {
    vtkErrorMacro("Server side operator failed.");
    OperationProxy->Delete();
    return 0;
  }

  OperationProxy->Delete();
  // The following call will re-serialize the model, so we need to
  // set the serialized model back to before
  bool res = vtkCMBModelBuilderClient::UpdateClientModel(Model, ServerModelProxy);
  if (res)
  {
    vtkSMStringVectorProperty* smSerializedModel =
      vtkSMStringVectorProperty::SafeDownCast(ServerModelProxy->GetProperty("SerializedModel"));
    if (smSerializedModel && strSerializedModel)
    {
      smSerializedModel->SetElement(0, strSerializedModel);
      smSerializedModel->SetElementType(0, vtkSMStringVectorProperty::STRING);
      ServerModelProxy->UpdateVTKObjects();
    }
    else
    {
      vtkWarningMacro(
        "Can't reset the serialized model that was changed by updating client model.");
    }
  }
  return res;
}

void vtkCreateModelEdgesOperationClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
