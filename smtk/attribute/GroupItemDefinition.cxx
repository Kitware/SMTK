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


#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/GroupItem.h"
#include <iostream>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
GroupItemDefinition::GroupItemDefinition(const std::string &myName):
  ItemDefinition(myName), m_numberOfRequiredGroups(1), m_useCommonLabel(false)
{
}

//----------------------------------------------------------------------------
GroupItemDefinition::~GroupItemDefinition()
{
}
//----------------------------------------------------------------------------
Item::Type GroupItemDefinition::type() const
{
  return Item::GROUP;
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr
GroupItemDefinition::buildItem(Attribute *owningAttribute,
                               int itemPosition) const
{
  return smtk::attribute::ItemPtr(new GroupItem(owningAttribute,
                                               itemPosition));
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr
GroupItemDefinition::buildItem(Item *owningItem,
                               int itemPosition,
                               int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new GroupItem(owningItem,
                                               itemPosition,
                                               subGroupPosition));
}
//----------------------------------------------------------------------------
bool GroupItemDefinition::
addItemDefinition(smtk::attribute::ItemDefinitionPtr cdef)
{
  // First see if there is a item by the same name
  if (this->findItemPosition(cdef->name()) >= 0)
    {
    return false;
    }
  std::size_t n = this->m_itemDefs.size();
  this->m_itemDefs.push_back(cdef);
  this->m_itemDefPositions[cdef->name()] = static_cast<int>(n);
  return true;
}
//----------------------------------------------------------------------------
void GroupItemDefinition::
buildGroup(GroupItem *groupItem, int subGroupPosition) const
{
  std::size_t i, n = this->m_itemDefs.size();
  std::vector<smtk::attribute::ItemPtr> &items = groupItem->m_items[subGroupPosition];
  items.resize(n);
  for (i = 0; i < n; i++)
    {
    items[i] = this->m_itemDefs[i]->buildItem(groupItem,
                                              static_cast<int>(i),
                                              subGroupPosition);
    items[i]->setDefinition(this->m_itemDefs[i]);
    }
}
//----------------------------------------------------------------------------
void GroupItemDefinition::updateCategories()
{
  this->m_categories.clear();
  std::size_t i, n = this->m_itemDefs.size();
  for (i = 0; i < n; i++)
    {
    this->m_itemDefs[i]->updateCategories();
    const std::set<std::string> &itemCats = this->m_itemDefs[i]->categories();
    this->m_categories.insert(itemCats.begin(), itemCats.end());
    }
}
//----------------------------------------------------------------------------
void GroupItemDefinition::addCategory(const std::string &/*category*/)
{
  std::cerr << "Cannot add categories to a group item definition. "
            << "The name is " << this->name() << std::endl;
}
//----------------------------------------------------------------------------
void GroupItemDefinition::removeCategory(const std::string &/*category*/)
{
  std::cerr << "Cannot remove categories to a group item definition. "
            << "The name is " << this->name() << std::endl;
}
//----------------------------------------------------------------------------
void GroupItemDefinition::setSubGroupLabel(int element, const std::string &elabel)
{
  if (this->m_numberOfRequiredGroups == 0)
    {
    return;
    }
  if (this->m_labels.size() != this->m_numberOfRequiredGroups)
    {
    this->m_labels.resize(this->m_numberOfRequiredGroups);
    }
  this->m_useCommonLabel = false;
  this->m_labels[element] = elabel;
}
//----------------------------------------------------------------------------
void GroupItemDefinition::setCommonSubGroupLabel(const std::string &elabel)
{
  if (this->m_labels.size() != 1)
    {
    this->m_labels.resize(1);
    }
  this->m_useCommonLabel = true;
  this->m_labels[0] = elabel;
}

//----------------------------------------------------------------------------
std::string GroupItemDefinition::subGroupLabel(int element) const
{
  if (this->m_useCommonLabel)
    {
    return this->m_labels[0];
    }
  if (this->m_labels.size())
    {
    return this->m_labels[element];
    }
  return ""; // If we threw execeptions this method could return const string &
}
//----------------------------------------------------------------------------
