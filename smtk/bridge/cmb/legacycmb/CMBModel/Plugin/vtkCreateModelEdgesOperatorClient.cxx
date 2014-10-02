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
