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
#include <vtkCMBModel.h>
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
bool vtkCmbModelFaceMeshClient::BuildMesh(bool meshHigherDimensionalEntities)
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
  operatorProxy->SetConnectionID(serverModelProxy->GetConnectionID());
  operatorProxy->SetServers(serverModelProxy->GetServers());

  vtkSMIntVectorProperty* meshProperty =
    vtkSMIntVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("MeshHigherDimensionalEntities"));
  meshProperty->SetElement(0, false);

  vtkCMBModel* model =
    vtkCMBModel::SafeDownCast(this->GetModelGeometricEntity()->GetModel());
  operatorProxy->Operate(model, serverModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* operateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("OperateSucceeded"));

  operatorProxy->UpdatePropertyInformation();

  int succeeded = operateSucceeded->GetElement(0);
  operatorProxy->Delete();
  operatorProxy = 0;
  if(!succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    return false;
    }
  // when we do volume meshing we'll have to fill this in

  return true;
}

//----------------------------------------------------------------------------
bool vtkCmbModelFaceMeshClient::SetLocalMaxArea(double maxArea,
  bool meshHigherDimensionalEntities)
{
  if(maxArea == this->GetMaximumArea())
    {
    return true;
    }
  bool res = this->SetFaceParameters("MaximumArea",
    maxArea, meshHigherDimensionalEntities);
  if(res)
    {
    this->SetMaximumArea(maxArea);
    this->SetMeshedMaximumArea(this->GetActualMaximumArea());

    // now we go and remesh any adjacent model regions
    if(meshHigherDimensionalEntities)
    {
    }
  }
  return res;
}

//----------------------------------------------------------------------------
bool vtkCmbModelFaceMeshClient::SetLocalMinAngle(
  double minAngle, bool meshHigherDimensionalEntities)
{
  if(minAngle == this->GetMinimumAngle())
    {
    return true;
    }
  bool res = this->SetFaceParameters("MinimumAngle",
    minAngle, meshHigherDimensionalEntities);
  if(res)
    {
    this->SetMinimumAngle(minAngle);
    this->SetMeshedMinimumAngle(this->GetActualMinimumAngle());

    // now we go and remesh any adjacent model regions
    if(meshHigherDimensionalEntities)
      {
      }
    }
  return res;
}

//----------------------------------------------------------------------------
bool vtkCmbModelFaceMeshClient::SetFaceParameters(
  const char* pName, double pValue, bool meshHigherDimensionalEntities)
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
  operatorProxy->SetConnectionID(serverModelProxy->GetConnectionID());
  operatorProxy->SetServers(serverModelProxy->GetServers());

  vtkSMPropertyHelper(operatorProxy, pName).Set(pValue);
  vtkSMPropertyHelper(operatorProxy, "Id").Set(
    this->GetModelGeometricEntity()->GetUniquePersistentId());
  operatorProxy->UpdateVTKObjects();

  vtkCMBModel* model =
    vtkCMBModel::SafeDownCast(this->GetModelGeometricEntity()->GetModel());
  operatorProxy->Operate(model,
    vtkCmbMeshClient::SafeDownCast(this->GetMasterMesh())->GetServerMeshProxy());

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* operateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
    operatorProxy->GetProperty("OperateSucceeded"));

  operatorProxy->UpdatePropertyInformation();

  int succeeded = operateSucceeded->GetElement(0);
  operatorProxy->Delete();
  operatorProxy = 0;
  if(!succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    return false;
    }

  return true;
}
//----------------------------------------------------------------------------
void vtkCmbModelFaceMeshClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
