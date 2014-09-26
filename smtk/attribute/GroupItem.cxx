//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
using namespace smtk::attribute;

//----------------------------------------------------------------------------
GroupItem::GroupItem(Attribute *owningAttribute,
                     int itemPosition):
  Item(owningAttribute, itemPosition)
{
}

//----------------------------------------------------------------------------
GroupItem::GroupItem(Item *inOwningItem,
                     int itemPosition,
                     int mySubGroupPosition):
  Item(inOwningItem, itemPosition, mySubGroupPosition)
{
}

//----------------------------------------------------------------------------
GroupItem::~GroupItem()
{
  // This group is going away so make sure any items that are
  // being held externally no longer think they are owned by it
  this->detachAllItems();
 }
//----------------------------------------------------------------------------
void GroupItem::detachAllItems()
{
  // Detatch all top level items contained in this group
  std::size_t i, j, n, m;
  n = this->m_items.size();
  for (i = 0; i < n; i++)
    {
    std::vector<smtk::attribute::ItemPtr> &items = this->m_items[i];
    m = items.size();
    for (j = 0; j < m; j++)
      {
      items[j]->detachOwningItem();
      }
    }
}
//----------------------------------------------------------------------------
Item::Type GroupItem::type() const
{
  return GROUP;
}
//----------------------------------------------------------------------------
bool
GroupItem::setDefinition(smtk::attribute::ConstItemDefinitionPtr gdef)
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
      def->buildGroup(this, static_cast<int>(i));
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
  if (this->numberOfGroups() != n)
    {
    this->setNumberOfGroups(n);
    }
  if (!n)
    {
    this->detachAllItems();
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
bool GroupItem::isExtensible() const
{
  const GroupItemDefinition *def =
    static_cast<const GroupItemDefinition*>(this->m_definition.get());
  if (!def)
    {
    return false;
    }
  return def->isExtensible();
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
std::size_t GroupItem::maxNumberOfGroups() const
{
  const GroupItemDefinition *def =
    static_cast<const GroupItemDefinition*>(this->m_definition.get());
  if (!def)
    {
    return 0;
    }
  return def->maxNumberOfGroups();
}
//----------------------------------------------------------------------------
bool GroupItem::appendGroup()
{
  if (!this->isExtensible())
    {
    return false;
    }

  const GroupItemDefinition *def =
    static_cast<const GroupItemDefinition *>(this->definition().get());
  std::size_t maxN = def->maxNumberOfGroups(), n = this->numberOfGroups();
  if (maxN && (n >= maxN))
    {
    // max number of groups reached
    return false;
    }
  this->m_items.resize(n+1);
  def->buildGroup(this, static_cast<int>(n));
  return true;
}
//----------------------------------------------------------------------------
bool GroupItem::removeGroup(std::size_t element)
{
  if (!this->isExtensible())
    {
    return false;
    }
  if (this->numberOfGroups() <= this->numberOfRequiredGroups())
    {
    return false; // min number of groups reached
    }

  std::vector<smtk::attribute::ItemPtr> &items = this->m_items[element];
  std::size_t j, m = items.size();
  for(j = 0; j < m; j++)
    {
    items[j]->detachOwningItem();
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
  if (!this->isExtensible())
    {
    return false;
    }
  // Is this size between the required number and the max?
  if (newSize < this->numberOfRequiredGroups())
    {
    return false;
    }

  std::size_t n = this->maxNumberOfGroups();
  if (n && (newSize > n))
    {
    return false; // greater than max number
    }

  const GroupItemDefinition *def =
    static_cast<const GroupItemDefinition *>(this->definition().get());
  std::size_t i;
  n = this->numberOfGroups();
  if (newSize < n)
    {
    // We need to detach all of the items we no longer need
    std::size_t j, m;
    for (i = newSize; i < n; i++)
      {
      std::vector<smtk::attribute::ItemPtr> &items = this->m_items[i];
      m = items.size();
      for (j = 0; j < m; j++)
        {
        items[j]->detachOwningItem();
        }
      }
    }
  else
    {
    this->m_items.resize(newSize);
    for (i = n; i < newSize; i++)
      {
      def->buildGroup(this, static_cast<int>(i));
      }
    }
  return true;
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr GroupItem::find(std::size_t element, const std::string &inName)
{
  const GroupItemDefinition *def =
    static_cast<const GroupItemDefinition *>(this->definition().get());
  int i = def->findItemPosition(inName);
  return (i < 0) ? smtk::attribute::ItemPtr() : this->m_items[element][static_cast<std::size_t>(i)];
}
//----------------------------------------------------------------------------
smtk::attribute::ConstItemPtr GroupItem::find(std::size_t element, const std::string &inName) const
{
  const GroupItemDefinition *def =
    static_cast<const GroupItemDefinition *>(this->definition().get());
  int i = def->findItemPosition(inName);
  if (i < 0)
    {
    return smtk::attribute::ConstItemPtr();
    }
  return this->m_items[element][static_cast<std::size_t>(i)];
}
//----------------------------------------------------------------------------
void GroupItem::copyFrom(ItemPtr sourceItem, CopyInfo& info)
{
  // Assigns my contents to be same as sourceItem
  Item::copyFrom(sourceItem, info);

  // Cast input pointer to GroupItem
  GroupItemPtr sourceGroupItem =
    smtk::dynamic_pointer_cast<GroupItem>(sourceItem);

  // Update children (items)
  this->setNumberOfGroups(sourceGroupItem->numberOfGroups());
  for (std::size_t i=0; i<sourceGroupItem->numberOfGroups(); ++i)
    {
    for (std::size_t j=0; j<sourceGroupItem->numberOfItemsPerGroup(); ++j)
      {
      ItemPtr sourceChildItem = sourceGroupItem->item(i, j);
      ItemPtr childItem = this->item(i, j);
      childItem->copyFrom(sourceChildItem, info);
      }
    }
}
//----------------------------------------------------------------------------
