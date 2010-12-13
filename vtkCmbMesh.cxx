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
#include "vtkCmbMesh.h"

#include <vtkCallbackCommand.h>
#include <vtkCMBModel.h>
#include "vtkCmbModelVertexMesh.h"
#include <vtkModelEntity.h>
#include <vtkModelItemIterator.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

#include <map>

vtkCxxRevisionMacro(vtkCmbMesh, "");

class vtkCmbMeshInternals
{
public:
  std::map<vtkIdType, vtkSmartPointer<vtkCmbModelEntityMesh> > ModelEntities;
  vtkWeakPointer<vtkCMBModel> Model;
  vtkCmbMeshInternals() : Model(NULL)
    {};
};

//----------------------------------------------------------------------------
vtkCmbMesh::vtkCmbMesh()
{
  this->Visible = true;
  this->GlobalLength = 0;
  this->Internal = new vtkCmbMeshInternals;
}

//----------------------------------------------------------------------------
vtkCmbMesh::~vtkCmbMesh()
{
  if(this->Internal)
    {
    delete this->Internal;
    this->Internal = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkCmbMesh::Initialize(vtkCMBModel* model)
{
  if(model == NULL)
    {
    vtkErrorMacro("Passed in NULL model.");
    return;
    }
  if(this->Internal->Model != model)
    {
    this->Reset();
    this->Internal->Model = model;
    }
  // register model modification events that we want
  vtkSmartPointer<vtkCallbackCommand> callbackCommand =
    vtkSmartPointer<vtkCallbackCommand>::New();
  callbackCommand->SetCallback(vtkCmbMesh::ModelGeometricEntityChanged);
  callbackCommand->SetClientData((void*) this);
  model->AddObserver(ModelGeometricEntityBoundaryModified, callbackCommand);
  model->AddObserver(ModelGeometricEntityCreated, callbackCommand);
  model->AddObserver(ModelGeometricEntityAboutToDestroy, callbackCommand);
  model->AddObserver(ModelGeometricEntitiesAboutToMerge, callbackCommand);
  model->AddObserver(ModelGeometricEntitySplit, callbackCommand);



  // vertices
  vtkModelItemIterator* iter = model->NewIterator(vtkModelVertexType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelEntity* entity =
      vtkModelEntity::SafeDownCast(iter->GetCurrentItem());
    vtkSmartPointer<vtkCmbModelVertexMesh> meshRepresentation =
      vtkSmartPointer<vtkCmbModelVertexMesh>::New();
    meshRepresentation->SetGridSize(this->GlobalLength);
    this->Internal->ModelEntities[entity->GetUniquePersistentId()] =
      meshRepresentation;
    }
  iter->Delete();
  // edges
//   iter = model->NewIterator(vtkModelEdgeType);
//   for(iter->Begin();!iter->IsAtEnd();iter->Next())
//     {
//     vtkModelEntity* entity =
//       vtkModelEntity::SafeDownCast(iter->GetCurrentItem());
//     vtkSmartPointer<vtkCmbModelEntityMesh> meshRepresentation =
//       vtkSmartPointer<vtkCmbModelEntityMesh>::New();
//     meshRepresentation->SetGridSize(this->GlobalLength);
//     this->Internal->ModelEntities[entity->GetUniquePersistentId()] =
//       meshRepresentation;
//     }
//   iter->Delete();
//   // faces
//   iter = model->NewIterator(vtkModelFaceType);
//   for(iter->Begin();!iter->IsAtEnd();iter->Next())
//     {
//     vtkModelEntity* entity =
//       vtkModelEntity::SafeDownCast(iter->GetCurrentItem());
//     vtkSmartPointer<vtkCmbModelEntityMesh> meshRepresentation =
//       vtkSmartPointer<vtkCmbModelEntityMesh>::New();
//     meshRepresentation->SetGridSize(this->GlobalLength);
//     this->Internal->ModelEntities[entity->GetUniquePersistentId()] =
//       meshRepresentation;
//     }
//   iter->Delete();
//   // regions
//   iter = model->NewIterator(vtkModelRegionType);
//   for(iter->Begin();!iter->IsAtEnd();iter->Next())
//     {
//     vtkModelEntity* entity =
//       vtkModelEntity::SafeDownCast(iter->GetCurrentItem());
//     vtkSmartPointer<vtkCmbModelEntityMesh> meshRepresentation =
//       vtkSmartPointer<vtkCmbModelEntityMesh>::New();
//     meshRepresentation->SetGridSize(this->GlobalLength);
//     this->Internal->ModelEntities[entity->GetUniquePersistentId()] =
//       meshRepresentation;
//     }
//   iter->Delete();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMesh::SetGlobalLength(double globalLength)
{
  if(this->GlobalLength == globalLength)
    {
    return;
    }
  if(globalLength <= 0)
    {
    vtkWarningMacro("Trying to set GlobalLength to invalid value of " << globalLength);
    return;
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMesh::Reset()
{
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMesh::ModelGeometricEntityChanged(
  vtkObject *caller, unsigned long event, void *cData, void *callData)
{
  vtkCmbMesh* cmbMesh = (vtkCmbMesh*) callData;
  cmbMesh->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Visible: " << this->Visible << "\n";
  os << indent << "GlobalLength: " << this->GlobalLength << "\n";
}

