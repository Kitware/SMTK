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
#include "vtkDiscreteModelEntity.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkModelItemIterator.h"
#include "vtkObjectFactory.h"

vtkDiscreteModelEntity::vtkDiscreteModelEntity()
{
}

vtkDiscreteModelEntity::~vtkDiscreteModelEntity()
{
}

vtkDiscreteModelEntity* vtkDiscreteModelEntity::GetThisDiscreteModelEntity(
  vtkModelEntity* entity)
{
  if(!entity)
    {
    return 0;
    }

  if(vtkDiscreteModelRegion* region = vtkDiscreteModelRegion::SafeDownCast(entity))
    {
    return region;
    }

  if(vtkDiscreteModelFace* face = vtkDiscreteModelFace::SafeDownCast(entity))
    {
    return face;
    }

  if(vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(entity))
    {
    return edge;
    }

  return 0;
}

int vtkDiscreteModelEntity::GetNumberOfModelEntityGroups()
{
  return this->GetThisModelEntity()->GetNumberOfAssociations(
    vtkDiscreteModelEntityGroupType);
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

  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkDiscreteModelEntityGroup* entityGroup = vtkDiscreteModelEntityGroup::SafeDownCast(
      iter->GetCurrentItem());
    entityGroup->AddModelEntity(this);
    }
  iter->Delete();
}

void vtkDiscreteModelEntity::RemoveAllModelEntityGroups()
{
  vtkModelItemIterator* iter = this->NewModelEntityGroupIterator();

  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkDiscreteModelEntityGroup* entityGroup = vtkDiscreteModelEntityGroup::SafeDownCast(
      iter->GetCurrentItem());
    entityGroup->RemoveModelEntity(this);
    }
  iter->Delete();
}

void vtkDiscreteModelEntity::PrintSelf(ostream& os, vtkIndent indent)
{

}

