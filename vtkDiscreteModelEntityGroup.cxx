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
#include "vtkDiscreteModelEntityGroup.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEntity.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkSerializer.h"

vtkCxxRevisionMacro(vtkDiscreteModelEntityGroup, "");

vtkDiscreteModelEntityGroup* vtkDiscreteModelEntityGroup::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkDiscreteModelEntityGroup");
  if(ret)
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
  int entityType = entity->GetType();
  this->AddAssociation(entityType, entity);
}

bool vtkDiscreteModelEntityGroup::RemoveModelEntity(vtkDiscreteModelEntity* object)
{
  this->RemoveAssociation(object->GetThisModelEntity()->GetType(),
                          object->GetThisModelEntity());
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
  this->Superclass::PrintSelf(os,indent);

  os << indent << "EntityType: " << this->EntityType << "\n";
}
