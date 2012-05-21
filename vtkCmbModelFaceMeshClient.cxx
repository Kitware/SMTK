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
#include "vtkCmbModelFaceMeshClient.h"

#include "vtkCmbMeshClient.h"
#include <vtkDiscreteModel.h>
#include <vtkModelFace.h>
#include <vtkObjectFactory.h>
#include <vtkSMOperatorProxy.h>
#include <vtkPolyData.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMProxyManager.h>
#include "vtkSMPropertyHelper.h"

vtkStandardNewMacro(vtkCmbModelFaceMeshClient);
vtkCxxRevisionMacro(vtkCmbModelFaceMeshClient, "");

//----------------------------------------------------------------------------
vtkCmbModelFaceMeshClient::vtkCmbModelFaceMeshClient()
{
}

//----------------------------------------------------------------------------
vtkCmbModelFaceMeshClient::~vtkCmbModelFaceMeshClient()
{
}

//----------------------------------------------------------------------------
bool vtkCmbModelFaceMeshClient::SendLengthAndAngleToServer()
{
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* operatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBSimBuilderMeshGroup", "ModelFaceMeshOperator"));
  if(!operatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return false;
    }
  vtkSMProxy* serverModelProxy =
    vtkCmbMeshClient::SafeDownCast(this->GetMasterMesh())->GetServerModelProxy();
  operatorProxy->SetLocation(serverModelProxy->GetLocation());

  vtkSMPropertyHelper(operatorProxy, "Id").Set(
    this->GetModelGeometricEntity()->GetUniquePersistentId());
  vtkSMPropertyHelper(operatorProxy, "Length").Set(this->GetLength());
  vtkSMPropertyHelper(operatorProxy, "MinimumAngle").Set(this->GetMinimumAngle());
  vtkSMPropertyHelper(operatorProxy, "BuildModelEntityMesh").Set(false);
  vtkSMPropertyHelper(operatorProxy, "MeshHigherDimensionalEntities").Set(false);

  vtkDiscreteModel* model =
    vtkDiscreteModel::SafeDownCast(this->GetModelGeometricEntity()->GetModel());
  operatorProxy->Operate(model, vtkCmbMeshClient::SafeDownCast(
                           this->GetMasterMesh())->GetServerMeshProxy());

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* operateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("OperateSucceeded"));

  operatorProxy->UpdatePropertyInformation();

  int succeeded = operateSucceeded->GetElement(0);
  operatorProxy->Delete();
  operatorProxy = NULL;
  if(!succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkCmbModelFaceMeshClient::BuildMesh(bool meshHigherDimensionalEntities)
{
  this->SetMeshedLength(0);
  this->SetMeshedMinimumAngle(0);
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* operatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBSimBuilderMeshGroup", "ModelFaceMeshOperator"));
  if(!operatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return false;
    }
  vtkSMProxy* serverModelProxy =
    vtkCmbMeshClient::SafeDownCast(this->GetMasterMesh())->GetServerModelProxy();
  operatorProxy->SetLocation(serverModelProxy->GetLocation());

  vtkSMPropertyHelper(operatorProxy, "Id").Set(
    this->GetModelGeometricEntity()->GetUniquePersistentId());
  vtkSMPropertyHelper(operatorProxy, "Length").Set(this->GetLength());
  vtkSMPropertyHelper(operatorProxy, "MinimumAngle").Set(this->GetMinimumAngle());
  vtkSMPropertyHelper(operatorProxy, "BuildModelEntityMesh").Set(true);
  vtkSMPropertyHelper(operatorProxy, "MeshHigherDimensionalEntities").Set(false);

  vtkDiscreteModel* model =
    vtkDiscreteModel::SafeDownCast(this->GetModelGeometricEntity()->GetModel());
  operatorProxy->Operate(model, vtkCmbMeshClient::SafeDownCast(
                           this->GetMasterMesh())->GetServerMeshProxy());

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* operateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("OperateSucceeded"));

  operatorProxy->UpdatePropertyInformation();

  int succeeded = operateSucceeded->GetElement(0);
  operatorProxy->Delete();
  operatorProxy = NULL;
  if(!succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    return false;
    }
  this->SetMeshedLength(this->GetActualLength());
  this->SetMeshedMinimumAngle(this->GetActualMinimumAngle());

  // when we do volume meshing we'll have to fill this in if
  // meshhigherdimensionalentities is true

  return true;
}

//----------------------------------------------------------------------------
bool vtkCmbModelFaceMeshClient::SetLocalLength(double length)
{
  if(length == this->GetLength())
    {
    return true;
    }
  this->SetLength(length);
  return this->SendLengthAndAngleToServer();
}

//----------------------------------------------------------------------------
bool vtkCmbModelFaceMeshClient::SetLocalMinimumAngle(double minAngle)
{
  if(minAngle == this->GetMinimumAngle())
    {
    return true;
    }
  this->SetMinimumAngle(minAngle);
  return this->SendLengthAndAngleToServer();
}

//----------------------------------------------------------------------------
void vtkCmbModelFaceMeshClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
