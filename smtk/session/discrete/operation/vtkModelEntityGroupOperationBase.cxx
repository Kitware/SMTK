//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelEntityGroupOperationBase.h"

#include "vtkDiscreteModel.h"
#
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include "vtkIdList.h"
#
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkModelEntityGroupOperationBase);

vtkModelEntityGroupOperationBase::vtkModelEntityGroupOperationBase()
{
  this->ItemType = vtkDiscreteModelEntityGroupType;
  this->IsItemTypeSet = 1;
  this->EntitiesToAdd = vtkIdList::New();
  this->EntitiesToRemove = vtkIdList::New();
  this->BuildEnityType = vtkModelFaceType;
}

vtkModelEntityGroupOperationBase::~vtkModelEntityGroupOperationBase()
{
  if (this->EntitiesToAdd)
  {
    this->EntitiesToAdd->Delete();
    this->EntitiesToAdd = nullptr;
  }
  if (this->EntitiesToRemove)
  {
    this->EntitiesToRemove->Delete();
    this->EntitiesToRemove = nullptr;
  }
}

void vtkModelEntityGroupOperationBase::SetItemType(int /*itemType*/)
{
  // override the super class because we don't want to allow
  // this to be changes
}

void vtkModelEntityGroupOperationBase::SetBuildEnityType(int enType)
{
  if (this->BuildEnityType != enType && (enType == vtkModelFaceType || enType == vtkModelEdgeType))
  {
    this->BuildEnityType = enType;
    this->Modified();
  }
}

vtkDiscreteModelEntityGroup* vtkModelEntityGroupOperationBase::GetModelEntityGroup(
  vtkDiscreteModel* Model)
{
  vtkModelEntity* Entity = this->vtkModelEntityOperationBase::GetModelEntity(Model);
  vtkDiscreteModelEntityGroup* EntityGroup = vtkDiscreteModelEntityGroup::SafeDownCast(Entity);

  return EntityGroup;
}

bool vtkModelEntityGroupOperationBase::AbleToOperate(vtkDiscreteModel* Model)
{
  if (!Model)
  {
    vtkErrorMacro("Passed in a null model.");
    return false;
  }
  if (this->GetIsIdSet() == 0)
  {
    vtkErrorMacro("No entity id specified.");
    return false;
  }
  if (!this->GetModelEntityGroup(Model))
  {
    vtkErrorMacro("Cannot find the entity group.");
    return false;
  }

  return true;
}

void vtkModelEntityGroupOperationBase::AddModelEntity(vtkIdType EntityId)
{
  this->EntitiesToAdd->InsertUniqueId(EntityId);
}

void vtkModelEntityGroupOperationBase::AddModelEntity(vtkDiscreteModelEntity* ModelEntity)
{
  vtkModelEntity* Ent = ModelEntity->GetThisModelEntity();
  this->AddModelEntity(Ent->GetUniquePersistentId());
}

void vtkModelEntityGroupOperationBase::ClearEntitiesToAdd()
{
  this->EntitiesToAdd->Reset();
}

void vtkModelEntityGroupOperationBase::RemoveModelEntity(vtkIdType EntityId)
{
  this->EntitiesToRemove->InsertUniqueId(EntityId);
}

void vtkModelEntityGroupOperationBase::RemoveModelEntity(vtkDiscreteModelEntity* ModelEntity)
{
  vtkModelEntity* Ent = ModelEntity->GetThisModelEntity();
  this->RemoveModelEntity(Ent->GetUniquePersistentId());
}

void vtkModelEntityGroupOperationBase::ClearEntitiesToRemove()
{
  this->EntitiesToRemove->Reset();
}

bool vtkModelEntityGroupOperationBase::Operate(vtkDiscreteModel* Model)
{
  if (!this->AbleToOperate(Model))
  {
    return false;
  }

  vtkDiscreteModelEntityGroup* EntityGroup = this->GetModelEntityGroup(Model);

  vtkIdType i;
  for (i = 0; i < this->EntitiesToAdd->GetNumberOfIds(); i++)
  {
    vtkModelEntity* Entity = Model->GetModelEntity(this->EntitiesToAdd->GetId(i));

    if (Model->GetModelDimension() == 3)
    {
      if (Entity->GetType() != vtkModelFaceType && Entity->GetType() != vtkModelEdgeType)
      {
        vtkWarningMacro("Unsupported entity type for a model entity group.");
        continue;
      }
    }
    else if (Model->GetModelDimension() == 2)
    {
      if (Entity->GetType() != vtkModelEdgeType)
      {
        vtkWarningMacro("Currently only model edges can be added to a model entity group.");
        continue;
      }
    }

    vtkDiscreteModelEntity* CMBEntity = vtkDiscreteModelEntity::GetThisDiscreteModelEntity(Entity);

    EntityGroup->AddModelEntity(CMBEntity);
  }

  for (i = 0; i < this->EntitiesToRemove->GetNumberOfIds(); i++)
  {
    vtkModelEntity* Entity = Model->GetModelEntity(this->EntitiesToRemove->GetId(i));

    vtkDiscreteModelEntity* CMBEntity = vtkDiscreteModelEntity::GetThisDiscreteModelEntity(Entity);

    EntityGroup->RemoveModelEntity(CMBEntity);
  }

  return this->Superclass::Operate(Model);
}

vtkIdType vtkModelEntityGroupOperationBase::Build(vtkDiscreteModel* Model)
{
  if (Model->GetModelDimension() == 2)
  {
    this->SetBuildEnityType(vtkModelEdgeType);
  }
  vtkDiscreteModelEntityGroup* EntityGroup =
    Model->BuildModelEntityGroup(this->BuildEnityType, 0, nullptr);
  return EntityGroup->GetUniquePersistentId();
}

bool vtkModelEntityGroupOperationBase::Destroy(vtkDiscreteModel* Model)
{
  vtkDiscreteModelEntityGroup* EntityGroup = this->GetModelEntityGroup(Model);
  return Model->DestroyModelEntityGroup(EntityGroup);
}

void vtkModelEntityGroupOperationBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "EntitiesToAdd: " << this->EntitiesToAdd << endl;
  os << indent << "EntitiesToRemove: " << this->EntitiesToRemove << endl;
}
