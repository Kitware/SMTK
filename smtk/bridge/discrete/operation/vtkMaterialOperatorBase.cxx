//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkMaterialOperatorBase.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkModelMaterial.h"
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
  this->GeometricEntitiesToRemove = vtkIdList::New();
  this->PreviousMaterialsOfGeometricEntities = vtkIdList::New();
}

vtkMaterialOperatorBase::~vtkMaterialOperatorBase()
{
  if(this->GeometricEntitiesToAdd)
    {
    this->GeometricEntitiesToAdd->Delete();
    this->GeometricEntitiesToAdd = 0;
    }
  if(this->GeometricEntitiesToRemove)
    {
    this->GeometricEntitiesToRemove->Delete();
    this->GeometricEntitiesToRemove = 0;
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

void vtkMaterialOperatorBase::RemoveModelGeometricEntity(
  vtkIdType GeometricEntityId)
{
  this->GeometricEntitiesToRemove->InsertUniqueId(GeometricEntityId);
}

void vtkMaterialOperatorBase::RemoveModelGeometricEntity(
  vtkModelGeometricEntity* GeometricEntity)
{
  this->RemoveModelGeometricEntity(GeometricEntity->GetUniquePersistentId());
}

void vtkMaterialOperatorBase::ClearGeometricEntitiesToRemove()
{
  this->GeometricEntitiesToRemove->Reset();
}


bool vtkMaterialOperatorBase::Operate(vtkDiscreteModel* Model)
{
  if(!this->AbleToOperate(Model))
    {
    return 0;
    }

  vtkModelMaterial* Material = this->GetMaterial(Model);
  this->PreviousMaterialsOfGeometricEntities->Reset();

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
    if(GeometricEntity && GeometricEntity->GetMaterial() != Material)
      {
      vtkModelMaterial* PreviousMaterial = GeometricEntity->GetMaterial();
      if(PreviousMaterial)
        this->PreviousMaterialsOfGeometricEntities->InsertUniqueId(
          PreviousMaterial->GetUniquePersistentId());
      Material->AddModelGeometricEntity(vtkModelGeometricEntity::SafeDownCast(Entity));
      }
    }
  // remove entities
  for(vtkIdType i=0;i<this->GeometricEntitiesToRemove->GetNumberOfIds();i++)
    {
    vtkModelEntity* Entity = Model->GetModelEntity(this->GeometricEntitiesToRemove->GetId(i));
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
      if(PreviousMaterial == Material)
        Material->RemoveModelGeometricEntity(vtkModelGeometricEntity::SafeDownCast(Entity));
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
