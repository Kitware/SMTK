//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelEdgeOperationClient.h"

#include "vtkDiscreteModel.h"
#include "vtkModelUserName.h"
#
#include "vtkDiscreteModelEdge.h"
#include "vtkObjectFactory.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperationProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"

vtkStandardNewMacro(vtkModelEdgeOperationClient);

vtkModelEdgeOperationClient::vtkModelEdgeOperationClient()
{
}

vtkModelEdgeOperationClient::~vtkModelEdgeOperationClient()
{
}

bool vtkModelEdgeOperationClient::Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if (!this->AbleToOperate(Model))
  {
    return 0;
  }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperationProxy* OperationProxy =
    vtkSMOperationProxy::SafeDownCast(manager->NewProxy("CMBModelGroup", "ModelEdgeOperation"));
  if (!OperationProxy)
  {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
  }
  OperationProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkSMIdTypeVectorProperty* idproperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(OperationProxy->GetProperty("Id"));
  idproperty->SetElement(0, this->GetId());

  vtkSMIntVectorProperty* resproperty =
    vtkSMIntVectorProperty::SafeDownCast(OperationProxy->GetProperty("LineResolution"));
  resproperty->SetElement(0, this->GetLineResolution());

  if (this->GetIsItemTypeSet())
  {
    vtkSMIntVectorProperty* typeproperty =
      vtkSMIntVectorProperty::SafeDownCast(OperationProxy->GetProperty("ItemType"));
    typeproperty->SetElement(0, this->GetItemType());
  }
  if (this->GetIsVisibilitySet())
  {
    vtkSMIntVectorProperty* visibilityproperty =
      vtkSMIntVectorProperty::SafeDownCast(OperationProxy->GetProperty("Visibility"));
    visibilityproperty->SetElement(0, this->GetVisibility());
  }

  if (this->GetIsRGBASet())
  {
    double* rgba = this->GetRGBA();
    vtkSMDoubleVectorProperty* rgbaproperty =
      vtkSMDoubleVectorProperty::SafeDownCast(OperationProxy->GetProperty("RGBA"));
    for (int j = 0; j < 4; j++)
    {
      rgbaproperty->SetElement(j, rgba[j]);
    }
  }
  if (this->GetUserName())
  {
    vtkSMStringVectorProperty* strproperty =
      vtkSMStringVectorProperty::SafeDownCast(OperationProxy->GetProperty("UserName"));
    strproperty->SetElement(0, this->GetUserName());
    strproperty->SetElementType(0, vtkSMStringVectorProperty::STRING);
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

void vtkModelEdgeOperationClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
