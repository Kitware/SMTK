//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkADHExporterOperatorClient.h"

#include "vtkDiscreteModel.h"
#include <vtkSMIdTypeVectorProperty.h>
#include <vtkSMIntVectorProperty.h>
#include "vtkSMOperatorProxy.h"
#include <vtkSMProxy.h>
#include <vtkSMProxyManager.h>
#include <vtkSMStringVectorProperty.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkADHExporterOperatorClient);

vtkADHExporterOperatorClient::vtkADHExporterOperatorClient()
{
  this->ClientText = 0;
}

vtkADHExporterOperatorClient::~vtkADHExporterOperatorClient()
{
  this->SetClientText(0);
}

bool vtkADHExporterOperatorClient::Operate(
  vtkDiscreteModel* model, vtkSMProxy* serverModelProxy)
{
  if(this->ClientText == NULL)
    {
    vtkErrorMacro("No text set on the client.");
    return 0;
    }

  if(this->AbleToOperate(model) == 0)
    {
    return 0;
    }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* operatorProxy =
    vtkSMOperatorProxy::SafeDownCast(
      manager->NewProxy("CMBModelGroup", "ADHExporter"));
  if(!operatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
    }
  operatorProxy->SetLocation(serverModelProxy->GetLocation());

  vtkSMStringVectorProperty* strproperty =
    vtkSMStringVectorProperty::SafeDownCast(operatorProxy->GetProperty("FileName"));
  strproperty->SetElement(0, this->GetFileName());
  strproperty->SetElementType(0, vtkSMStringVectorProperty::STRING);

  strproperty = vtkSMStringVectorProperty::
    SafeDownCast(operatorProxy->GetProperty("ClientText"));
  strproperty->SetElement(0, this->GetClientText());
  strproperty->SetElementType(0, vtkSMStringVectorProperty::STRING);

  vtkSMIdTypeVectorProperty* idProperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(operatorProxy->GetProperty("ApplyNodalBC"));

  int counter = 0;  //first the nodal bcs
  for(int i=0;i<this->GetNumberOfAppliedNodalBCs();i++)
    {
    int bcIndex, bcNodalGroupType;
    vtkIdType bcsGroupId;
    if(this->GetAppliedNodalBC(i, bcIndex, bcsGroupId, bcNodalGroupType))
      {
      idProperty->SetElement(counter++, bcIndex);
      idProperty->SetElement(counter++, bcsGroupId);
      idProperty->SetElement(counter++, bcNodalGroupType);
      }
    }

  idProperty = vtkSMIdTypeVectorProperty::SafeDownCast(operatorProxy->GetProperty("ApplyElementBC"));
  counter = 0; // now the face bcs
  for(int i=0;i<this->GetNumberOfAppliedElementBCs();i++)
    {
    int bcIndex;
    vtkIdType faceGroupId;
    if(this->GetAppliedElementBC(i, bcIndex, faceGroupId))
      {
      idProperty->SetElement(counter++, bcIndex);
      idProperty->SetElement(counter++, faceGroupId);
      }
    }

  operatorProxy->Operate(model, serverModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* operateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(operatorProxy->GetProperty("OperateSucceeded"));

  operatorProxy->UpdatePropertyInformation();

  int succeeded = operateSucceeded->GetElement(0);
  operatorProxy->Delete();
  if(!succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    return 0;
    }

  return true;
}

void vtkADHExporterOperatorClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
