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


#include "attribute/GroupItem.h"
#include "attribute/GroupItemDefinition.h"
using namespace slctk::attribute; 

//----------------------------------------------------------------------------
GroupItem::GroupItem()
{
}

//----------------------------------------------------------------------------
GroupItem::~GroupItem()
{
}
//----------------------------------------------------------------------------
Item::Type GroupItem::type() const
{
  return GROUP;
}
//----------------------------------------------------------------------------
bool
GroupItem::setDefinition(slctk::ConstAttributeItemDefinitionPtr gdef) 
{
   // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const GroupItemDefinition *def = 
    dynamic_cast<const GroupItemDefinition *>(gdef.get());
  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if ((def == NULL) || (!Item::setDefinition(gdef)))
    {
    return false;
    }
  this->m_definition = gdef;
  std::size_t i, n = def->numberOfRequiredGroups();
  if (n)
    {
    this->m_items.resize(n);
    for (i = 0; i < n; i++)
      {
      def->buildGroup(this->m_items[i]);
      }
    }
  return true;
}
//----------------------------------------------------------------------------
void GroupItem::reset() 
{
  const GroupItemDefinition *def = 
    dynamic_cast<const GroupItemDefinition *>(this->m_definition.get());
  std::size_t i, n = def->numberOfRequiredGroups();
  if (!n)
    {
    this->m_items.clear();
    }
  else
    {
    for (i = 0; i < n; i++)
      {
      std::size_t j, m = this->m_items[i].size();
      for (j = 0; j < m; j++)
        {
        this->m_items[i][j]->reset();
        }
      }
    }
  Item::reset();
}
//----------------------------------------------------------------------------
std::size_t GroupItem::numberOfItemsPerGroup() const
{
  const GroupItemDefinition *def = 
    static_cast<const GroupItemDefinition *>(this->definition().get());
  return def->numberOfItemDefinitions();
}
//----------------------------------------------------------------------------
std::size_t GroupItem::numberOfRequiredGroups() const
{
  const GroupItemDefinition *def = 
    static_cast<const GroupItemDefinition *>(this->definition().get());
  return def->numberOfRequiredGroups();
}
//----------------------------------------------------------------------------
bool GroupItem::appendGroup()
{
  const GroupItemDefinition *def = 
    static_cast<const GroupItemDefinition *>(this->definition().get());
  std::size_t n = def->numberOfRequiredGroups();
  if (n)
    {
    // Can not change the number of items
    return false;
    }
  n = this->m_items.size();
  this->m_items.resize(n+1);
  def->buildGroup(this->m_items[n]);
  return true;
}
//----------------------------------------------------------------------------
bool GroupItem::removeGroup(int element)
{
  const GroupItemDefinition *def = 
    static_cast<const GroupItemDefinition *>(this->definition().get());
  std::size_t n = def->numberOfRequiredGroups();
  if (n)
    {
    // Can not change the number of items
    return false;
    }
  this->m_items.erase(this->m_items.begin() + element);
  return true;
}
//----------------------------------------------------------------------------
bool GroupItem::setNumberOfGroups(std::size_t newSize)
{
  // If the current size is the same just return
  if (this->numberOfGroups() == newSize)
    {
    return true;
    }
  
  //Next - are we allowed to change the number of values?
  const GroupItemDefinition *def =
    static_cast<const GroupItemDefinition *>(this->definition().get());
  std::size_t i, n = def->numberOfRequiredGroups();
  if (n)
    {
    return false; // The number of values is fixed
    }
  this->m_items.resize(newSize);
  for (i = n; i < newSize; i++)
    {
    def->buildGroup(this->m_items[i]);
    }
  return true;
}
//----------------------------------------------------------------------------
slctk::AttributeItemPtr GroupItem::find(int element, const std::string &name)
{
  const GroupItemDefinition *def = 
    static_cast<const GroupItemDefinition *>(this->definition().get());
  int i = def->findItemPosition(name);
  return (i < 0) ? slctk::AttributeItemPtr() : this->m_items[element][i];
}
//----------------------------------------------------------------------------
slctk::ConstAttributeItemPtr GroupItem::find(int element, const std::string &name) const
{
  const GroupItemDefinition *def = 
    static_cast<const GroupItemDefinition *>(this->definition().get());
  int i = def->findItemPosition(name);
  if (i < 0)
    {
    return slctk::ConstAttributeItemPtr();
    }
  return this->m_items[element][i];
}
//----------------------------------------------------------------------------
