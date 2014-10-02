//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCreateModelEdgesOperatorClient.h"

#include "vtkCMBModelBuilderClient.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModelUserName.h"
#include "vtkIdTypeArray.h"
#include "vtkIdTypeArray.h"
#include "vtkModelFaceUse.h"
#include "vtkModelShellUse.h"
#include "vtkObjectFactory.h"

#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"

vtkStandardNewMacro(vtkCreateModelEdgesOperatorClient);

vtkCreateModelEdgesOperatorClient::vtkCreateModelEdgesOperatorClient()
{
}

vtkCreateModelEdgesOperatorClient::~vtkCreateModelEdgesOperatorClient()
{
}

bool vtkCreateModelEdgesOperatorClient::Operate(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy, const char* strSerializedModel)
{
  if(!this->AbleToOperate(Model))
    {
    return 0;
    }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy =
    vtkSMOperatorProxy::SafeDownCast(
      manager->NewProxy("CMBModelGroup", "CreateModelEdges"));
  if(!OperatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
    }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  OperatorProxy->Operate(Model, ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("OperateSucceeded"));

  OperatorProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  if(!Succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    OperatorProxy->Delete();
    return 0;
    }

  OperatorProxy->Delete();
  // The following call will re-serialize the model, so we need to
  // set the serialized model back to before
  bool res = vtkCMBModelBuilderClient::UpdateClientModel(Model, ServerModelProxy);
  if(res)
    {
     vtkSMStringVectorProperty* smSerializedModel =
        vtkSMStringVectorProperty::SafeDownCast(
          ServerModelProxy->GetProperty("SerializedModel"));
    if(smSerializedModel && strSerializedModel)
      {
      smSerializedModel->SetElement(0, strSerializedModel);
      smSerializedModel->SetElementType(0, vtkSMStringVectorProperty::STRING);
      ServerModelProxy->UpdateVTKObjects();
      }
    else
      {
      vtkWarningMacro("Can't reset the serialized model that was changed by updating client model.");
      }
    }
  return res;
}

void vtkCreateModelEdgesOperatorClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
