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

#include "vtkMaterialOperatorBase.h"

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

vtkStandardNewMacro(vtkMaterialOperatorBase);

vtkMaterialOperatorBase::vtkMaterialOperatorBase()
{
  this->ItemType = vtkModelMaterialType;
  this->IsItemTypeSet = 1;
  this->GeometricEntitiesToAdd = vtkIdList::New();
  this->PreviousMaterialsOfGeometricEntities = vtkIdList::New();
}

vtkMaterialOperatorBase::~vtkMaterialOperatorBase()
{
  if(this->GeometricEntitiesToAdd)
    {
    this->GeometricEntitiesToAdd->Delete();
    this->GeometricEntitiesToAdd = 0;
    }
  if(this->PreviousMaterialsOfGeometricEntities)
    {
    this->PreviousMaterialsOfGeometricEntities->Delete();
    this->PreviousMaterialsOfGeometricEntities = 0;
    }
}

void vtkMaterialOperatorBase::SetItemType(int /*itemType*/)
{
  // override the super class because we don't want to allow
  // this to be changes
}

vtkModelMaterial* vtkMaterialOperatorBase::GetMaterial(vtkDiscreteModel* Model)
{
  vtkModelEntity* Entity = this->vtkModelEntityOperatorBase::GetModelEntity(Model);
  vtkModelMaterial* Material = vtkModelMaterial::SafeDownCast(Entity);

  return Material;
}

bool vtkMaterialOperatorBase::AbleToOperate(vtkDiscreteModel* Model)
{
  if(!Model)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }
  if(this->GetIsIdSet() == 0)
    {
    vtkErrorMacro("No entity id specified.");
    return 0;
    }
  if(!this->GetMaterial(Model))
    {
    vtkErrorMacro("Cannot find the material.");
    return 0;
    }

  return 1;
}

void vtkMaterialOperatorBase::AddModelGeometricEntity(
  vtkIdType GeometricEntityId)
{
  this->GeometricEntitiesToAdd->InsertUniqueId(GeometricEntityId);
}

void vtkMaterialOperatorBase::AddModelGeometricEntity(
  vtkModelGeometricEntity* GeometricEntity)
{
  this->AddModelGeometricEntity(GeometricEntity->GetUniquePersistentId());
}

void vtkMaterialOperatorBase::ClearGeometricEntitiesToAdd()
{
  this->GeometricEntitiesToAdd->Reset();
}

bool vtkMaterialOperatorBase::Operate(vtkDiscreteModel* Model)
{
  if(!this->AbleToOperate(Model))
    {
    return 0;
    }

  vtkModelMaterial* Material = this->GetMaterial(Model);
  this->PreviousMaterialsOfGeometricEntities->Reset();
  this->PreviousMaterialsOfGeometricEntities->SetNumberOfIds(
    this->GeometricEntitiesToAdd->GetNumberOfIds());

  for(vtkIdType i=0;i<this->GeometricEntitiesToAdd->GetNumberOfIds();i++)
    {
    vtkModelEntity* Entity = Model->GetModelEntity(this->GeometricEntitiesToAdd->GetId(i));
    vtkDiscreteModelGeometricEntity* GeometricEntity = 0;
    vtkDiscreteModelRegion* Region = vtkDiscreteModelRegion::SafeDownCast(Entity);
    if(Region)
      {
      GeometricEntity = Region;
      }
    else
      {
      vtkDiscreteModelFace* Face = vtkDiscreteModelFace::SafeDownCast(Entity);
      if(Face && Face->GetModelRegion(0) ==  NULL && Face->GetModelRegion(1) == NULL)
        { // Face can have a material associated with it since it is not associated with a region
        GeometricEntity = Face;
        }
      }
    if(GeometricEntity)
      {
      vtkModelMaterial* PreviousMaterial = GeometricEntity->GetMaterial();
      this->PreviousMaterialsOfGeometricEntities->SetId(
        i, PreviousMaterial->GetUniquePersistentId());
      Material->AddModelGeometricEntity(vtkModelGeometricEntity::SafeDownCast(Entity));
      }
    else
      {
      this->PreviousMaterialsOfGeometricEntities->SetId(i, -1);
      }
    }

  return this->Superclass::Operate(Model);
}

vtkIdType vtkMaterialOperatorBase::Build(vtkDiscreteModel* Model)
{
  vtkModelMaterial* Material = Model->BuildMaterial();
  return Material->GetUniquePersistentId();
}

bool vtkMaterialOperatorBase::Destroy(vtkDiscreteModel* Model)
{
  vtkModelMaterial* Material = this->GetMaterial(Model);
  return Model->DestroyMaterial(Material);
}

void vtkMaterialOperatorBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "GeometricEntitiesToAdd: " <<
    this->GeometricEntitiesToAdd << endl;
  os << indent << "PreviousMaterialsOfGeometricEntities: " <<
    this->PreviousMaterialsOfGeometricEntities << endl;
}
