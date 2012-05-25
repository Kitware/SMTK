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

vtkDiscreteModelEntity* vtkDiscreteModelEntity::GetThisCMBModelEntity(
  vtkModelEntity* Entity)
{
  if(!Entity)
    {
    return 0;
    }

  vtkDiscreteModelRegion* Region = vtkDiscreteModelRegion::SafeDownCast(Entity);
  if(Region)
    {
    return Region;
    }

  vtkDiscreteModelFace* Face = vtkDiscreteModelFace::SafeDownCast(Entity);
  if(Face)
    {
    return Face;
    }

  vtkDiscreteModelEdge* Edge = vtkDiscreteModelEdge::SafeDownCast(Entity);
  if(Edge)
    {
    return Edge;
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

void vtkDiscreteModelEntity::CopyModelEntityGroups(vtkDiscreteModelEntity* SourceEntity)
{
  this->RemoveAllModelEntityGroups();
  vtkModelItemIterator* Iter = SourceEntity->NewModelEntityGroupIterator();

  for(Iter->Begin();!Iter->IsAtEnd();Iter->Next())
    {
    vtkDiscreteModelEntityGroup* EntityGroup = vtkDiscreteModelEntityGroup::SafeDownCast(
      Iter->GetCurrentItem());
    EntityGroup->AddModelEntity(this);
    }
  Iter->Delete();
}

void vtkDiscreteModelEntity::RemoveAllModelEntityGroups()
{
  vtkModelItemIterator* Iter = this->NewModelEntityGroupIterator();

  for(Iter->Begin();!Iter->IsAtEnd();Iter->Next())
    {
    vtkDiscreteModelEntityGroup* EntityGroup = vtkDiscreteModelEntityGroup::SafeDownCast(
      Iter->GetCurrentItem());
    EntityGroup->RemoveModelEntity(this);
    }
  Iter->Delete();
}

void vtkDiscreteModelEntity::PrintSelf(ostream& os, vtkIndent indent)
{

}

