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

#include "vtkMaterialOperatorClient.h"

#include "vtkModelMaterial.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#
#include "vtkDiscreteModelRegion.h"
#include "vtkIdList.h"
#
#
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"

vtkStandardNewMacro(vtkMaterialOperatorClient);

vtkMaterialOperatorClient::vtkMaterialOperatorClient()
{
}

vtkMaterialOperatorClient::~vtkMaterialOperatorClient()
{
}

bool vtkMaterialOperatorClient::Operate(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if(!this->AbleToOperate(Model))
    {
    return 0;
    }

  // first try to do the operation on the server
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "MaterialOperator"));
  if(!OperatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
    }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkSMIdTypeVectorProperty* idproperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("Id"));
  idproperty->SetElement(0, this->GetId());

  if(this->GetIsVisibilitySet())
    {
    vtkSMIntVectorProperty* visibilityproperty =
      vtkSMIntVectorProperty::SafeDownCast(
        OperatorProxy->GetProperty("Visibility"));
    visibilityproperty->SetElement(0, this->GetVisibility());
    }

  if(this->GetIsRGBASet())
    {
    double * rgba = this->GetRGBA();
    vtkSMDoubleVectorProperty* rgbaproperty =
      vtkSMDoubleVectorProperty::SafeDownCast(
        OperatorProxy->GetProperty("RGBA"));
    for(int j=0;j<4;j++)
      {
      rgbaproperty->SetElement(j, rgba[j]);
      }
    }
  if(this->GetUserName())
    {
    vtkSMStringVectorProperty* strproperty =
      vtkSMStringVectorProperty::SafeDownCast(
        OperatorProxy->GetProperty("UserName"));
    strproperty->SetElement(0, this->GetUserName());
    strproperty->SetElementType(0, vtkSMStringVectorProperty::STRING);
    }
  unsigned int NumberOfGeometricEntities =
    this->GetGeometricEntitiesToAdd()->GetNumberOfIds();
  if(NumberOfGeometricEntities)
    {
    vtkSMIdTypeVectorProperty* addedentitiesproperty =
      vtkSMIdTypeVectorProperty::SafeDownCast(
        OperatorProxy->GetProperty("AddModelGeometricEntity"));
    for(unsigned int ui=0;ui<NumberOfGeometricEntities;ui++)
      {
      addedentitiesproperty->SetElement(
        ui, this->GetGeometricEntitiesToAdd()->GetId(ui));
      }
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

vtkIdType vtkMaterialOperatorClient::Build(vtkDiscreteModel* Model,
                                           vtkSMProxy* ServerModelProxy)
{
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "MaterialOperator"));
  if(!OperatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return -1;
    }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkIdType ServerMaterialId =
    OperatorProxy->Build(Model, ServerModelProxy);
  vtkIdType ClientMaterialId = this->Superclass::Build(Model);
  OperatorProxy->Delete();
  if(ServerMaterialId != ClientMaterialId)
    {
    vtkErrorMacro("Created material does not have matching Ids on the server and client.");
    return -1;
    }
  this->SetId(ClientMaterialId);
  return ClientMaterialId;
}

bool vtkMaterialOperatorClient::Destroy(vtkDiscreteModel* Model,
                                        vtkSMProxy* ServerModelProxy)
{
  if(this->GetIsIdSet() == 0)
    {
    vtkWarningMacro("Must specify Id of material to destroy.");
    return 0;
    }
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "MaterialOperator"));
  if(!OperatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
    }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  // now set the id of the material to be destroyed on the server
  vtkSMIdTypeVectorProperty* idproperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("Id"));
  idproperty->SetElement(0, this->GetId());

  bool Success = OperatorProxy->Destroy(Model, ServerModelProxy);
  OperatorProxy->Delete();
  if(!Success)
    {
    vtkWarningMacro("Could not delete material on server.");
    return 0;
    }

  return this->Superclass::Destroy(Model);
}

void vtkMaterialOperatorClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
