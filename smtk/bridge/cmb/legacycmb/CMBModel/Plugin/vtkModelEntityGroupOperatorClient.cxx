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

#include "vtkModelEntityGroupOperatorClient.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEntity.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkIdList.h"
#
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"

vtkStandardNewMacro(vtkModelEntityGroupOperatorClient);

vtkModelEntityGroupOperatorClient::vtkModelEntityGroupOperatorClient()
{
}

vtkModelEntityGroupOperatorClient::~vtkModelEntityGroupOperatorClient()
{
}

bool vtkModelEntityGroupOperatorClient::Operate(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if(!this->AbleToOperate(Model))
    {
    return 0;
    }

  // first try to do the operation on the server
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "ModelEntityGroupOperator"));
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
  unsigned int numberOfEntities =
    this->GetEntitiesToAdd()->GetNumberOfIds();
  if(numberOfEntities)
    {
    vtkSMIdTypeVectorProperty* addedentitiesproperty =
      vtkSMIdTypeVectorProperty::SafeDownCast(
        OperatorProxy->GetProperty("AddModelEntity"));
    for(unsigned int ui=0;ui<numberOfEntities;ui++)
      {
      addedentitiesproperty->SetElement(
        ui, this->GetEntitiesToAdd()->GetId(ui));
      }
    }

  numberOfEntities = this->GetEntitiesToRemove()->GetNumberOfIds();
  if(numberOfEntities)
    {
    vtkSMIdTypeVectorProperty* removedentitiesproperty =
      vtkSMIdTypeVectorProperty::SafeDownCast(
        OperatorProxy->GetProperty("RemoveModelEntity"));
    for(unsigned int ui=0;ui<numberOfEntities;ui++)
      {
      removedentitiesproperty->SetElement(
        ui, this->GetEntitiesToRemove()->GetId(ui));
      }
    }

  vtkSMIntVectorProperty* buildtypeproperty =
     vtkSMIntVectorProperty::SafeDownCast(
     OperatorProxy->GetProperty("BuildEnityType"));
  buildtypeproperty->SetElement(0, this->GetBuildEnityType());

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

vtkIdType vtkModelEntityGroupOperatorClient::Build(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "ModelEntityGroupOperator"));
  if(!OperatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return -1;
    }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkIdType ServerEntityGroupId =
    OperatorProxy->Build(Model, ServerModelProxy);
  vtkIdType ClientEntityGroupId = this->Superclass::Build(Model);
  OperatorProxy->Delete();
  if(ServerEntityGroupId != ClientEntityGroupId)
    {
    vtkErrorMacro("Created entity group does not have matching Ids on the server and client.");
    return -1;
    }
  this->SetId(ClientEntityGroupId);
  return ClientEntityGroupId;
}

bool vtkModelEntityGroupOperatorClient::Destroy(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if(this->GetIsIdSet() == 0)
    {
    vtkWarningMacro("Must specify Id of entity group to destroy.");
    return 0;
    }
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "ModelEntityGroupOperator"));
  if(!OperatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
    }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  // now set the id of the entity group to be destroyed on the server
  vtkSMIdTypeVectorProperty* idproperty =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("Id"));
  idproperty->SetElement(0, this->GetId());

  bool Success = OperatorProxy->Destroy(Model, ServerModelProxy);
  OperatorProxy->Delete();
  if(!Success)
    {
    vtkWarningMacro("Could not delete entity group on server.");
    return 0;
    }

  return this->Superclass::Destroy(Model);
}

void vtkModelEntityGroupOperatorClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
