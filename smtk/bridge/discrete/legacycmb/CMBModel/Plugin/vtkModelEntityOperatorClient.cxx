//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkModelEntityOperatorClient.h"

#include "vtkDiscreteModel.h"
#include "vtkModelGeometricEntity.h"
#include "vtkModelUserName.h"
#include "vtkModelEntity.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"

vtkStandardNewMacro(vtkModelEntityOperatorClient);

vtkModelEntityOperatorClient::vtkModelEntityOperatorClient()
{
}

vtkModelEntityOperatorClient::~vtkModelEntityOperatorClient()
{
}

bool vtkModelEntityOperatorClient::Operate(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if(!this->AbleToOperate(Model))
    {
    return 0;
    }
  vtkModelEntity* modelEntity = this->GetModelEntity(Model);
  if(!modelEntity)
    {
    return 0;
    }
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "ModelEntityOperator"));
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
  int visible = this->GetIsVisibilitySet() ?
    this->GetVisibility() : modelEntity->GetVisibility();
  vtkSMIntVectorProperty* visibilityproperty =
    vtkSMIntVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("Visibility"));
  visibilityproperty->SetElement(0, visible);

  if(vtkModelGeometricEntity::SafeDownCast(modelEntity))
    {
    int pickable = this->GetIsPickableSet() ? this->GetPickable() :
      vtkModelGeometricEntity::SafeDownCast(modelEntity)->GetPickable();
    vtkSMIntVectorProperty* pickproperty =
      vtkSMIntVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("Pickable"));
    pickproperty->SetElement(0, pickable);
    }

  if(vtkModelGeometricEntity::SafeDownCast(modelEntity))
    {
    int texture = this->GetIsShowTextureSet() ? this->GetShowTexture() :
      vtkModelGeometricEntity::SafeDownCast(modelEntity)->GetShowTexture();
    vtkSMIntVectorProperty* pickproperty =
      vtkSMIntVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("ShowTexture"));
    pickproperty->SetElement(0, texture);
    }

  double * rgba = this->GetIsRGBASet() ? this->GetRGBA() : modelEntity->GetColor();
  vtkSMDoubleVectorProperty* rgbaproperty =
    vtkSMDoubleVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("RGBA"));
  for(int j=0;j<4;j++)
    {
    rgbaproperty->SetElement(j, rgba[j]);
    }

  double* reprgba= this->GetIsRepresentationRGBASet() ?
    this->GetRepresentationRGBA() : NULL;
  if(reprgba)
    {
    vtkSMDoubleVectorProperty* reprgbaproperty =
      vtkSMDoubleVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("RepresentationRGBA"));
    for(int j=0;j<4;j++)
      {
      reprgbaproperty->SetElement(j, reprgba[j]);
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
  ServerModelProxy->MarkModified(NULL);
  return this->Superclass::Operate(Model);
}

void vtkModelEntityOperatorClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
