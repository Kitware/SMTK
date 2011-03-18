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
#include "vtkCmbModelEdgeMeshClient.h"

#include "vtkCleanPolylines.h"
#include "vtkCmbMeshClient.h"
#include "vtkCMBModel.h"
#include "vtkCmbModelFaceMesh.h"
#include "vtkCMBModelEdge.h"
#include "vtkCmbModelVertexMesh.h"
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

vtkStandardNewMacro(vtkCmbModelEdgeMeshClient);
vtkCxxRevisionMacro(vtkCmbModelEdgeMeshClient, "");

//----------------------------------------------------------------------------
vtkCmbModelEdgeMeshClient::vtkCmbModelEdgeMeshClient()
{
}

//----------------------------------------------------------------------------
vtkCmbModelEdgeMeshClient::~vtkCmbModelEdgeMeshClient()
{
}

//----------------------------------------------------------------------------
bool vtkCmbModelEdgeMeshClient::SetLocalLength(double length)
{
  if(length == this->GetLength())
    {
    return true;
    }
  this->SetLength(length);
  return this->BuildMesh(false);
}

//----------------------------------------------------------------------------
bool vtkCmbModelEdgeMeshClient::BuildMesh(bool meshHigherDimensionalEntities)
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
    vtkCmbMeshClient::SafeDownCast(this->GetMasterMesh())->GetServerModelProxy();
  operatorProxy->SetConnectionID(serverModelProxy->GetConnectionID());
  operatorProxy->SetServers(serverModelProxy->GetServers());

  vtkSMPropertyHelper(operatorProxy, "Id").Set(
    this->GetModelGeometricEntity()->GetUniquePersistentId());
  vtkSMPropertyHelper(operatorProxy, "Length").Set(this->GetLength());
  vtkSMPropertyHelper(operatorProxy, "BuildModelEntityMesh").Set(true);
  vtkSMPropertyHelper(operatorProxy, "MeshHigherDimensionalEntities").Set(false);

  vtkCMBModel* model =
    vtkCMBModel::SafeDownCast(this->GetModelGeometricEntity()->GetModel());
  operatorProxy->Operate(
    model, vtkCmbMeshClient::SafeDownCast(
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
      vtkCmbModelFaceMesh* faceMesh = vtkCmbModelFaceMesh::SafeDownCast(
        this->GetMasterMesh()->GetModelEntityMesh(face));
      returnValue = returnValue && faceMesh->BuildModelEntityMesh(true);
      }
    faces->Delete();
    }

  return returnValue;
}

//----------------------------------------------------------------------------
void vtkCmbModelEdgeMeshClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
