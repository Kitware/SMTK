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
#include "vtkCMBModelEntityGroup.h"

#include "vtkDiscreteModel.h"
#include "vtkCMBModelEntity.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkSerializer.h"

vtkCxxRevisionMacro(vtkCMBModelEntityGroup, "");

vtkCMBModelEntityGroup* vtkCMBModelEntityGroup::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkCMBModelEntityGroup"); 
  if(ret) 
    {                                    
    return static_cast<vtkCMBModelEntityGroup*>(ret);
    } 
  return new vtkCMBModelEntityGroup;
}

vtkCMBModelEntityGroup::vtkCMBModelEntityGroup()
{
  this->EntityType = -1;
}

vtkCMBModelEntityGroup::~vtkCMBModelEntityGroup()
{
}

bool vtkCMBModelEntityGroup::IsDestroyable()
{
  return 1;
}

bool vtkCMBModelEntityGroup::Destroy()
{
  this->RemoveAllAssociations(this->EntityType);
  this->Modified();
  return 1;
}

void vtkCMBModelEntityGroup::AddModelEntity(vtkCMBModelEntity* Object)
{
  vtkModelEntity* entity = Object->GetThisModelEntity();
  int entityType = entity->GetType();
  this->AddAssociation(entityType, entity);
}

bool vtkCMBModelEntityGroup::RemoveModelEntity(vtkCMBModelEntity* Object)
{
  this->RemoveAssociation(Object->GetThisModelEntity()->GetType(), 
                          Object->GetThisModelEntity());
  return 1;
}

int vtkCMBModelEntityGroup::GetNumberOfModelEntities()
{
  return this->GetNumberOfAssociations(this->EntityType);
}

vtkModelItemIterator* vtkCMBModelEntityGroup::NewModelEntityIterator()
{
  vtkModelItemIterator* iter = this->NewIterator(this->EntityType);
  return iter;
}

int vtkCMBModelEntityGroup::GetType()
{
  return vtkCMBModelEntityGroupType;
}

void vtkCMBModelEntityGroup::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);

  ser->Serialize("EntityType", this->EntityType);
}


void vtkCMBModelEntityGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "EntityType: " << this->EntityType << "\n";
}
