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
#include "vtkCMBModelEntity.h"

#include "vtkDiscreteModel.h"
#include "vtkCMBModelEdge.h"
#include "vtkCMBModelEntityGroup.h"
#include "vtkCMBModelFace.h"
#include "vtkCMBModelRegion.h"
#include "vtkModelItemIterator.h"
#include "vtkObjectFactory.h"

vtkCMBModelEntity::vtkCMBModelEntity()
{
}

vtkCMBModelEntity::~vtkCMBModelEntity()
{
}

vtkCMBModelEntity* vtkCMBModelEntity::GetThisCMBModelEntity(
  vtkModelEntity* Entity)
{
  if(!Entity)
    {
    return 0;
    }

  vtkCMBModelRegion* Region = vtkCMBModelRegion::SafeDownCast(Entity);
  if(Region)
    {
    return Region;
    }

  vtkCMBModelFace* Face = vtkCMBModelFace::SafeDownCast(Entity);
  if(Face)
    {
    return Face;
    }

  vtkCMBModelEdge* Edge = vtkCMBModelEdge::SafeDownCast(Entity);
  if(Edge)
    {
    return Edge;
    }

  return 0;
}

int vtkCMBModelEntity::GetNumberOfModelEntityGroups()
{
  return this->GetThisModelEntity()->GetNumberOfAssociations(
    vtkCMBModelEntityGroupType);
}

vtkModelItemIterator* vtkCMBModelEntity::NewModelEntityGroupIterator()
{
  vtkModelEntity* thisEntity = this->GetThisModelEntity();
  vtkModelItemIterator* iter = thisEntity->NewIterator(vtkCMBModelEntityGroupType);
  return iter;
}

void vtkCMBModelEntity::CopyModelEntityGroups(vtkCMBModelEntity* SourceEntity)
{
  this->RemoveAllModelEntityGroups();
  vtkModelItemIterator* Iter = SourceEntity->NewModelEntityGroupIterator();

  for(Iter->Begin();!Iter->IsAtEnd();Iter->Next())
    {
    vtkCMBModelEntityGroup* EntityGroup = vtkCMBModelEntityGroup::SafeDownCast(
      Iter->GetCurrentItem());
    EntityGroup->AddModelEntity(this);
    }
  Iter->Delete();
}

void vtkCMBModelEntity::RemoveAllModelEntityGroups()
{
  vtkModelItemIterator* Iter = this->NewModelEntityGroupIterator();

  for(Iter->Begin();!Iter->IsAtEnd();Iter->Next())
    {
    vtkCMBModelEntityGroup* EntityGroup = vtkCMBModelEntityGroup::SafeDownCast(
      Iter->GetCurrentItem());
    EntityGroup->RemoveModelEntity(this);
    }
  Iter->Delete();
}

void vtkCMBModelEntity::PrintSelf(ostream& os, vtkIndent indent)
{

}

