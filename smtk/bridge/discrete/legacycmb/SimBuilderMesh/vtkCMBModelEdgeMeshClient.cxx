//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelEdgeMeshClient.h"

#include "vtkCleanPolylines.h"
#include "vtkCMBMeshClient.h"
#include "vtkDiscreteModel.h"
#include "vtkCMBModelFaceMesh.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkCMBModelVertexMesh.h"
#include "vtkMeshModelEdgesFilter.h"
#include <vtkModel.h>
#include <vtkModelEdge.h>
#include <vtkModelFace.h>
#include <vtkModelItemIterator.h>
#include <vtkModelVertex.h>

#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMOperatorProxy.h>
#include <vtkSMProxyManager.h>
#include "vtkSMPropertyHelper.h"
#include <vtkSMProxy.h>

vtkStandardNewMacro(vtkCMBModelEdgeMeshClient);

//----------------------------------------------------------------------------
vtkCMBModelEdgeMeshClient::vtkCMBModelEdgeMeshClient()
{
}

//----------------------------------------------------------------------------
vtkCMBModelEdgeMeshClient::~vtkCMBModelEdgeMeshClient()
{
}

//----------------------------------------------------------------------------
bool vtkCMBModelEdgeMeshClient::SetLocalLength(double length)
{
  if(length == this->GetLength())
    {
    return true;
    }
  this->SetLength(length);
  return this->SendLengthToServer();
}

//----------------------------------------------------------------------------
bool vtkCMBModelEdgeMeshClient::SendLengthToServer()
{
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* operatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBSimBuilderMeshGroup", "ModelEdgeMeshOperator"));
  if(!operatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return false;
    }
  vtkSMProxy* serverModelProxy =
    vtkCMBMeshClient::SafeDownCast(this->GetMasterMesh())->GetServerModelProxy();
  operatorProxy->SetLocation(serverModelProxy->GetLocation());

  vtkSMPropertyHelper(operatorProxy, "Id").Set(
    this->GetModelGeometricEntity()->GetUniquePersistentId());
  vtkSMPropertyHelper(operatorProxy, "Length").Set(this->GetLength());
  vtkSMPropertyHelper(operatorProxy, "BuildModelEntityMesh").Set(false);

  vtkDiscreteModel* model =
    vtkDiscreteModel::SafeDownCast(this->GetModelGeometricEntity()->GetModel());
  operatorProxy->Operate(
    model, vtkCMBMeshClient::SafeDownCast(
      this->GetMasterMesh())->GetServerMeshProxy());

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
    vtkErrorMacro("Server side setting mesh length failed.");
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBModelEdgeMeshClient::BuildMesh(bool meshHigherDimensionalEntities)
{
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* operatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBSimBuilderMeshGroup", "ModelEdgeMeshOperator"));
  if(!operatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return false;
    }
  vtkSMProxy* serverModelProxy =
    vtkCMBMeshClient::SafeDownCast(this->GetMasterMesh())->GetServerModelProxy();
  operatorProxy->SetLocation(serverModelProxy->GetLocation());

  vtkSMPropertyHelper(operatorProxy, "Id").Set(
    this->GetModelGeometricEntity()->GetUniquePersistentId());
  vtkSMPropertyHelper(operatorProxy, "Length").Set(this->GetLength());
  vtkSMPropertyHelper(operatorProxy, "BuildModelEntityMesh").Set(true);
  vtkSMPropertyHelper(operatorProxy, "MeshHigherDimensionalEntities").Set(false);

  vtkDiscreteModel* model =
    vtkDiscreteModel::SafeDownCast(this->GetModelGeometricEntity()->GetModel());
  operatorProxy->Operate(
    model, vtkCMBMeshClient::SafeDownCast(
      this->GetMasterMesh())->GetServerMeshProxy());

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
  this->SetMeshedLength(this->GetActualLength());
  bool returnValue = true;
  // now we go and remesh any adjacent model face meshes that exist
  if(meshHigherDimensionalEntities)
    {
    vtkModelItemIterator* faces =
      this->GetModelEdge()->NewAdjacentModelFaceIterator();
    for(faces->Begin();!faces->IsAtEnd();faces->Next())
      {
      vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
      vtkCMBModelFaceMesh* faceMesh = vtkCMBModelFaceMesh::SafeDownCast(
        this->GetMasterMesh()->GetModelEntityMesh(face));
      returnValue = returnValue && faceMesh->BuildModelEntityMesh(true);
      }
    faces->Delete();
    }

  return returnValue;
}

//----------------------------------------------------------------------------
void vtkCMBModelEdgeMeshClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
