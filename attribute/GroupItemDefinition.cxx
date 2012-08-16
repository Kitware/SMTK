/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
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


#include "attribute/GroupItemDefinition.h"
#include "attribute/GroupItem.h"
using namespace slctk::attribute; 

//----------------------------------------------------------------------------
GroupItemDefinition::GroupItemDefinition(const std::string &myName):
  ItemDefinition(myName)
{
}

//----------------------------------------------------------------------------
GroupItemDefinition::~GroupItemDefinition()
{
}
//----------------------------------------------------------------------------
slctk::AttributeItemPtr GroupItemDefinition::buildItem() const
{
  return slctk::AttributeItemPtr(new GroupItem());
}
//----------------------------------------------------------------------------
bool GroupItemDefinition::
addItemDefinition(slctk::AttributeItemDefinitionPtr cdef)
{
  // First see if there is a item by the same name
  if (this->findItemPosition(cdef->name()) >= 0)
    {
    return false;
    }
  std::size_t n = this->m_itemDefs.size();
  this->m_itemDefs.push_back(cdef);
  this->m_itemDefPositions[cdef->name()] = n;
  return true;
}
//----------------------------------------------------------------------------
void GroupItemDefinition::
buildGroup(std::vector<slctk::AttributeItemPtr> &group) const
{
  std::size_t i, n = this->m_itemDefs.size();
  group.resize(n);
  for (i = 0; i < n; i++)
    {
    group[i] = this->m_itemDefs[i]->buildItem();
    group[i]->setDefinition(this->m_itemDefs[i]);
    }
}
//----------------------------------------------------------------------------
