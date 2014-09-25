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

#include "vtkModelEntityGroupOperatorBase.h"

#include "vtkDiscreteModel.h"
#
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include "vtkIdList.h"
#
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkModelEntityGroupOperatorBase);

vtkModelEntityGroupOperatorBase::vtkModelEntityGroupOperatorBase()
{
  this->ItemType = vtkDiscreteModelEntityGroupType;
  this->IsItemTypeSet = 1;
  this->EntitiesToAdd = vtkIdList::New();
  this->EntitiesToRemove = vtkIdList::New();
  this->BuildEnityType = vtkModelFaceType;
}

vtkModelEntityGroupOperatorBase::~vtkModelEntityGroupOperatorBase()
{
  if(this->EntitiesToAdd)
    {
    this->EntitiesToAdd->Delete();
    this->EntitiesToAdd = 0;
    }
  if(this->EntitiesToRemove)
    {
    this->EntitiesToRemove->Delete();
    this->EntitiesToRemove = 0;
    }
}

void vtkModelEntityGroupOperatorBase::SetItemType(int /*itemType*/)
{
  // override the super class because we don't want to allow
  // this to be changes
}

void vtkModelEntityGroupOperatorBase::SetBuildEnityType(int enType)
{
  if(this->BuildEnityType != enType &&
    (enType == vtkModelFaceType || enType == vtkModelEdgeType))
    {
    this->BuildEnityType = enType;
    this->Modified();
    }
}

vtkDiscreteModelEntityGroup* vtkModelEntityGroupOperatorBase::GetModelEntityGroup(
  vtkDiscreteModel* Model)
{
  vtkModelEntity* Entity =
    this->vtkModelEntityOperatorBase::GetModelEntity(Model);
  vtkDiscreteModelEntityGroup* EntityGroup =
    vtkDiscreteModelEntityGroup::SafeDownCast(Entity);

  return EntityGroup;
}

bool vtkModelEntityGroupOperatorBase::AbleToOperate(vtkDiscreteModel* Model)
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
  if(!this->GetModelEntityGroup(Model))
    {
    vtkErrorMacro("Cannot find the entity group.");
    return 0;
    }

  return 1;
}

void vtkModelEntityGroupOperatorBase::AddModelEntity(
  vtkIdType EntityId)
{
  this->EntitiesToAdd->InsertUniqueId(EntityId);
}

void vtkModelEntityGroupOperatorBase::AddModelEntity(
  vtkDiscreteModelEntity* ModelEntity)
{
  vtkModelEntity* Ent = ModelEntity->GetThisModelEntity();
  this->AddModelEntity(Ent->GetUniquePersistentId());
}

void vtkModelEntityGroupOperatorBase::ClearEntitiesToAdd()
{
  this->EntitiesToAdd->Reset();
}

void vtkModelEntityGroupOperatorBase::RemoveModelEntity(
  vtkIdType EntityId)
{
  this->EntitiesToRemove->InsertUniqueId(EntityId);
}

void vtkModelEntityGroupOperatorBase::RemoveModelEntity(
  vtkDiscreteModelEntity* ModelEntity)
{
  vtkModelEntity* Ent = ModelEntity->GetThisModelEntity();
  this->RemoveModelEntity(Ent->GetUniquePersistentId());
}

void vtkModelEntityGroupOperatorBase::ClearEntitiesToRemove()
{
  this->EntitiesToRemove->Reset();
}

bool vtkModelEntityGroupOperatorBase::Operate(vtkDiscreteModel* Model)
{
  if(!this->AbleToOperate(Model))
    {
    return 0;
    }

  vtkDiscreteModelEntityGroup* EntityGroup = this->GetModelEntityGroup(Model);

  vtkIdType i;
  for(i=0;i<this->EntitiesToAdd->GetNumberOfIds();i++)
    {
    vtkModelEntity* Entity =
      Model->GetModelEntity(this->EntitiesToAdd->GetId(i));

    if(Model->GetModelDimension() == 3)
      {
      if(Entity->GetType() != vtkModelFaceType && Entity->GetType() != vtkModelEdgeType)
        {
        vtkWarningMacro("Unsupported entity type for a model entity group.");
        continue;
        }
      }
    else if(Model->GetModelDimension() == 2)
      {
      if(Entity->GetType() != vtkModelEdgeType)
        {
        vtkWarningMacro("Currently only model edges can be added to a model entity group.");
        continue;
        }
      }

    vtkDiscreteModelEntity* CMBEntity = vtkDiscreteModelEntity::GetThisDiscreteModelEntity(
      Entity);

    EntityGroup->AddModelEntity(CMBEntity);
    }

  for(i=0;i<this->EntitiesToRemove->GetNumberOfIds();i++)
    {
    vtkModelEntity* Entity =
      Model->GetModelEntity(this->EntitiesToRemove->GetId(i));

    vtkDiscreteModelEntity* CMBEntity = vtkDiscreteModelEntity::GetThisDiscreteModelEntity(
      Entity);

    EntityGroup->RemoveModelEntity(CMBEntity);
    }

  return this->Superclass::Operate(Model);
}

vtkIdType vtkModelEntityGroupOperatorBase::Build(vtkDiscreteModel* Model)
{
  if(Model->GetModelDimension() == 2)
    {
    this->SetBuildEnityType(vtkModelEdgeType);
    }
  vtkDiscreteModelEntityGroup* EntityGroup = Model->BuildModelEntityGroup(
    this->BuildEnityType, 0, 0);
  return EntityGroup->GetUniquePersistentId();
}

bool vtkModelEntityGroupOperatorBase::Destroy(vtkDiscreteModel* Model)
{
  vtkDiscreteModelEntityGroup* EntityGroup = this->GetModelEntityGroup(Model);
  return Model->DestroyModelEntityGroup(EntityGroup);
}

void vtkModelEntityGroupOperatorBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "EntitiesToAdd: " << this->EntitiesToAdd << endl;
  os << indent << "EntitiesToRemove: " << this->EntitiesToRemove << endl;
}
