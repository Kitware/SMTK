//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkDiscreteModelEntityGroup.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEntity.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkSerializer.h"

vtkDiscreteModelEntityGroup* vtkDiscreteModelEntityGroup::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkDiscreteModelEntityGroup");
  if (ret)
  {
    return static_cast<vtkDiscreteModelEntityGroup*>(ret);
  }
  return new vtkDiscreteModelEntityGroup;
}

vtkDiscreteModelEntityGroup::vtkDiscreteModelEntityGroup()
{
  this->EntityType = -1;
}

vtkDiscreteModelEntityGroup::~vtkDiscreteModelEntityGroup()
{
}

bool vtkDiscreteModelEntityGroup::IsDestroyable()
{
  return 1;
}

bool vtkDiscreteModelEntityGroup::Destroy()
{
  this->RemoveAllAssociations(this->EntityType);
  this->Modified();
  return 1;
}

void vtkDiscreteModelEntityGroup::AddModelEntity(vtkDiscreteModelEntity* object)
{
  vtkModelEntity* entity = object->GetThisModelEntity();
  this->AddAssociation(entity);
}

bool vtkDiscreteModelEntityGroup::RemoveModelEntity(vtkDiscreteModelEntity* object)
{
  this->RemoveAssociation(object->GetThisModelEntity());
  return 1;
}

int vtkDiscreteModelEntityGroup::GetNumberOfModelEntities()
{
  return this->GetNumberOfAssociations(this->EntityType);
}

vtkModelItemIterator* vtkDiscreteModelEntityGroup::NewModelEntityIterator()
{
  vtkModelItemIterator* iter = this->NewIterator(this->EntityType);
  return iter;
}

int vtkDiscreteModelEntityGroup::GetType()
{
  return vtkDiscreteModelEntityGroupType;
}

void vtkDiscreteModelEntityGroup::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);

  ser->Serialize("EntityType", this->EntityType);
}

void vtkDiscreteModelEntityGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "EntityType: " << this->EntityType << "\n";
}
