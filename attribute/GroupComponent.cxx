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


#include "attribute/GroupComponent.h"
#include "attribute/GroupComponentDefinition.h"
using namespace slctk::attribute; 

//----------------------------------------------------------------------------
GroupComponent::GroupComponent()
{
}

//----------------------------------------------------------------------------
GroupComponent::~GroupComponent()
{
}
//----------------------------------------------------------------------------
Component::Type GroupComponent::type() const
{
  return GROUP;
}
//----------------------------------------------------------------------------
bool
GroupComponent::setDefinition(slctk::ConstAttributeComponentDefinitionPtr gdef) 
{
   // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const GroupComponentDefinition *def = 
    dynamic_cast<const GroupComponentDefinition *>(gdef.get());
  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if ((def == NULL) || (Component::setDefinition(gdef)))
    {
    return false;
    }
  this->m_definition = gdef;
  std::size_t i, n = def->numberOfGroups();
  if (n)
    {
    this->m_components.resize(n);
    for (i = 0; i < n; i++)
      {
      def->buildGroup(this->m_components[i]);
      }
    }
  return true;
}
//----------------------------------------------------------------------------
std::size_t GroupComponent::numberOfComponentsPerGroup() const
{
  const GroupComponentDefinition *def = 
    static_cast<const GroupComponentDefinition *>(this->definition().get());
  return def->numberOfComponentDefinitions();
}
//----------------------------------------------------------------------------
bool GroupComponent::appendGroup()
{
  const GroupComponentDefinition *def = 
    static_cast<const GroupComponentDefinition *>(this->definition().get());
  std::size_t n = def->numberOfGroups();
  if (n)
    {
    // Can not change the number of components
    return false;
    }
  n = this->m_components.size();
  this->m_components.resize(n+1);
  def->buildGroup(this->m_components[n]);
  return true;
}
//----------------------------------------------------------------------------
bool GroupComponent::removeGroup(int element)
{
  const GroupComponentDefinition *def = 
    static_cast<const GroupComponentDefinition *>(this->definition().get());
  std::size_t n = def->numberOfGroups();
  if (n)
    {
    // Can not change the number of components
    return false;
    }
  this->m_components.erase(this->m_components.begin() + element);
  return true;
}
//----------------------------------------------------------------------------
slctk::AttributeComponentPtr GroupComponent::find(int element, const std::string &name)
{
  const GroupComponentDefinition *def = 
    static_cast<const GroupComponentDefinition *>(this->definition().get());
  int i = def->findComponentPosition(name);
  return (i < 0) ? slctk::AttributeComponentPtr() : this->m_components[element][i];
}
//----------------------------------------------------------------------------
slctk::ConstAttributeComponentPtr GroupComponent::find(int element, const std::string &name) const
{
  const GroupComponentDefinition *def = 
    static_cast<const GroupComponentDefinition *>(this->definition().get());
  int i = def->findComponentPosition(name);
  if (i < 0)
    {
    return slctk::ConstAttributeComponentPtr();
    }
  return this->m_components[element][i];
}
//----------------------------------------------------------------------------
