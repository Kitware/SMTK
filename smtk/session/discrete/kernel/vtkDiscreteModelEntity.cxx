//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkDiscreteModelEntity.h"

#include "smtk/session/discrete/kernel/Model/vtkModelItemIterator.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkObjectFactory.h"

vtkDiscreteModelEntity::vtkDiscreteModelEntity() = default;

vtkDiscreteModelEntity::~vtkDiscreteModelEntity() = default;

vtkDiscreteModelEntity* vtkDiscreteModelEntity::GetThisDiscreteModelEntity(vtkModelEntity* entity)
{
  if (!entity)
  {
    return nullptr;
  }

  if (vtkDiscreteModelRegion* region = vtkDiscreteModelRegion::SafeDownCast(entity))
  {
    return region;
  }

  if (vtkDiscreteModelFace* face = vtkDiscreteModelFace::SafeDownCast(entity))
  {
    return face;
  }

  if (vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(entity))
  {
    return edge;
  }

  return nullptr;
}

int vtkDiscreteModelEntity::GetNumberOfModelEntityGroups()
{
  return this->GetThisModelEntity()->GetNumberOfAssociations(vtkDiscreteModelEntityGroupType);
}

vtkModelItemIterator* vtkDiscreteModelEntity::NewModelEntityGroupIterator()
{
  vtkModelEntity* thisEntity = this->GetThisModelEntity();
  vtkModelItemIterator* iter = thisEntity->NewIterator(vtkDiscreteModelEntityGroupType);
  return iter;
}

void vtkDiscreteModelEntity::CopyModelEntityGroups(vtkDiscreteModelEntity* sourceEntity)
{
  this->RemoveAllModelEntityGroups();
  vtkModelItemIterator* iter = sourceEntity->NewModelEntityGroupIterator();

  for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
  {
    vtkDiscreteModelEntityGroup* entityGroup =
      vtkDiscreteModelEntityGroup::SafeDownCast(iter->GetCurrentItem());
    entityGroup->AddModelEntity(this);
  }
  iter->Delete();
}

void vtkDiscreteModelEntity::RemoveAllModelEntityGroups()
{
  vtkModelItemIterator* iter = this->NewModelEntityGroupIterator();

  for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
  {
    vtkDiscreteModelEntityGroup* entityGroup =
      vtkDiscreteModelEntityGroup::SafeDownCast(iter->GetCurrentItem());
    entityGroup->RemoveModelEntity(this);
  }
  iter->Delete();
}

void vtkDiscreteModelEntity::PrintSelf(ostream& /*os*/, vtkIndent /*indent*/)
{
}
