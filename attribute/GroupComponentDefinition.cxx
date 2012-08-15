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


#include "attribute/GroupComponentDefinition.h"
#include "attribute/GroupComponent.h"
using namespace slctk::attribute; 

//----------------------------------------------------------------------------
GroupComponentDefinition::GroupComponentDefinition(const std::string &myName):
  ComponentDefinition(myName)
{
}

//----------------------------------------------------------------------------
GroupComponentDefinition::~GroupComponentDefinition()
{
}
//----------------------------------------------------------------------------
slctk::AttributeComponentPtr GroupComponentDefinition::buildComponent() const
{
  return slctk::AttributeComponentPtr(new GroupComponent());
}
//----------------------------------------------------------------------------
bool GroupComponentDefinition::
addComponentDefinition(slctk::AttributeComponentDefinitionPtr cdef)
{
  // First see if there is a component by the same name
  if (this->findComponentPosition(cdef->name()) >= 0)
    {
    return false;
    }
  std::size_t n = this->m_componentDefs.size();
  this->m_componentDefs.push_back(cdef);
  this->m_componentDefPositions[cdef->name()] = n;
  return true;
}
//----------------------------------------------------------------------------
void GroupComponentDefinition::
buildGroup(std::vector<slctk::AttributeComponentPtr> &group) const
{
  std::size_t i, n = this->m_componentDefs.size();
  group.resize(n);
  for (i = 0; i < n; i++)
    {
    group[i] = this->m_componentDefs[i]->buildComponent();
    group[i]->setDefinition(this->m_componentDefs[i]);
    }
}
//----------------------------------------------------------------------------
