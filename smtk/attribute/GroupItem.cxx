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
#include <iostream>

using namespace smtk::attribute;

GroupItem::GroupItem(Attribute* owningAttribute, int itemPosition)
  : Item(owningAttribute, itemPosition)
{
}

GroupItem::GroupItem(Item* inOwningItem, int itemPosition, int mySubGroupPosition)
  : Item(inOwningItem, itemPosition, mySubGroupPosition)
{
}

GroupItem::~GroupItem()
{
  // This group is going away so make sure any items that are
  // being held externally no longer think they are owned by it
  this->detachAllItems();
}

void GroupItem::detachAllItems()
{
  // Detatch all top level items contained in this group
  std::size_t i, j, n, m;
  n = m_items.size();
  for (i = 0; i < n; i++)
  {
    std::vector<smtk::attribute::ItemPtr>& items = m_items[i];
    m = items.size();
    for (j = 0; j < m; j++)
    {
      items[j]->detachOwningItem();
    }
  }
}

Item::Type GroupItem::type() const
{
  return GroupType;
}

bool GroupItem::isValid(const std::set<std::string>& cats) const
{
  // Firdt lets see if the group itself would be filtered out based on the categories
  if (!(cats.empty() || this->categories().passes(cats)))
  {
    return true;
  }
  // If the item is not enabled or if all of its values are set then it is valid
  // else it is enabled and contains unset values making it invalid
  if (!this->isEnabled())
  {
    return true;
  }
  for (auto it = m_items.begin(); it != m_items.end(); ++it)
  {
    for (auto it1 = (*it).begin(); it1 != (*it).end(); ++it1)
    {
      if (!*it1 || !(*it1)->isValid(cats))
      {
        return false;
      }
    }
  }
  return true;
}

bool GroupItem::setDefinition(smtk::attribute::ConstItemDefinitionPtr gdef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const GroupItemDefinition* def = dynamic_cast<const GroupItemDefinition*>(gdef.get());
  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if ((def == nullptr) || (!Item::setDefinition(gdef)))
  {
    return false;
  }
  m_definition = gdef;
  std::size_t i, n = def->numberOfRequiredGroups();
  if (n)
  {
    m_items.resize(n);
    for (i = 0; i < n; i++)
    {
      def->buildGroup(this, static_cast<int>(i));
    }
  }
  return true;
}

void GroupItem::detachOwningResource()
{
  for (auto it = m_items.begin(); it != m_items.end(); ++it)
  {
    for (auto it1 = (*it).begin(); it1 != (*it).end(); ++it1)
    {
      if (*it1 != nullptr)
      {
        (*it1)->detachOwningResource();
      }
    }
  }
}

void GroupItem::reset()
{
  const GroupItemDefinition* def = static_cast<const GroupItemDefinition*>(m_definition.get());
  std::size_t i, n = def->numberOfRequiredGroups();
  if (this->numberOfGroups() != n)
  {
    this->setNumberOfGroups(n);
  }
  if (!n)
  {
    this->detachAllItems();
    m_items.clear();
  }
  else
  {
    for (i = 0; i < n; i++)
    {
      std::size_t j, m = m_items[i].size();
      for (j = 0; j < m; j++)
      {
        m_items[i][j]->reset();
      }
    }
  }
  Item::reset();
}

bool GroupItem::rotate(std::size_t fromPosition, std::size_t toPosition)
{
  return this->rotateVector(m_items, fromPosition, toPosition);
}

/**\brief Return an iterator to the first group in this item.
  *
  */
GroupItem::const_iterator GroupItem::begin() const
{
  return m_items.begin();
}

/**\brief Return an iterator just past the last group in this item.
  *
  */
GroupItem::const_iterator GroupItem::end() const
{
  return m_items.end();
}

bool GroupItem::isExtensible() const
{
  const GroupItemDefinition* def = static_cast<const GroupItemDefinition*>(m_definition.get());
  if (!def)
  {
    return false;
  }
  return def->isExtensible();
}

std::size_t GroupItem::numberOfItemsPerGroup() const
{
  const GroupItemDefinition* def =
    static_cast<const GroupItemDefinition*>(this->definition().get());
  return def->numberOfItemDefinitions();
}

std::size_t GroupItem::numberOfRequiredGroups() const
{
  const GroupItemDefinition* def =
    static_cast<const GroupItemDefinition*>(this->definition().get());
  return def->numberOfRequiredGroups();
}

std::size_t GroupItem::maxNumberOfGroups() const
{
  const GroupItemDefinition* def = static_cast<const GroupItemDefinition*>(m_definition.get());
  if (!def)
  {
    return 0;
  }
  return def->maxNumberOfGroups();
}

void GroupItem::visitChildren(std::function<void(ItemPtr, bool)> visitor, bool activeChildren)
{
  for (size_t elementIndex = 0; elementIndex < this->numberOfGroups(); elementIndex++)
  {
    for (size_t valueIndex = 0; valueIndex < this->numberOfItemsPerGroup(); valueIndex++)
    {
      visitor(this->item(elementIndex, valueIndex), activeChildren);
    }
  }
}

bool GroupItem::appendGroup()
{
  if (!this->isExtensible())
  {
    return false;
  }

  const GroupItemDefinition* def =
    static_cast<const GroupItemDefinition*>(this->definition().get());
  std::size_t maxN = def->maxNumberOfGroups(), n = this->numberOfGroups();
  if (maxN && (n >= maxN))
  {
    // max number of groups reached
    return false;
  }
  m_items.resize(n + 1);
  def->buildGroup(this, static_cast<int>(n));
  return true;
}

bool GroupItem::prependGroup()
{
  if (!this->isExtensible())
  {
    return false;
  }

  const GroupItemDefinition* def =
    static_cast<const GroupItemDefinition*>(this->definition().get());
  std::size_t maxN = def->maxNumberOfGroups(), n = this->numberOfGroups();
  if (maxN && (n >= maxN))
  {
    // max number of groups reached
    return false;
  }
  std::vector<smtk::attribute::ItemPtr> placeHolder;
  m_items.insert(m_items.begin(), placeHolder);
  def->buildGroup(this, 0);
  return true;
}

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

  assert(m_items.size() > element);
  std::vector<smtk::attribute::ItemPtr>& items = m_items[element];
  std::size_t j, m = items.size();
  for (j = 0; j < m; j++)
  {
    items[j]->detachOwningItem();
  }
  m_items.erase(m_items.begin() + element);
  return true;
}

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

  const GroupItemDefinition* def =
    static_cast<const GroupItemDefinition*>(this->definition().get());
  std::size_t i;
  n = this->numberOfGroups();
  if (newSize < n)
  {
    // We need to detach all of the items we no longer need
    std::size_t j, m;
    for (i = newSize; i < n; i++)
    {
      std::vector<smtk::attribute::ItemPtr>& items = m_items[i];
      m = items.size();
      for (j = 0; j < m; j++)
      {
        items[j]->detachOwningItem();
      }
    }
  }
  else
  {
    m_items.resize(newSize);
    for (i = n; i < newSize; i++)
    {
      def->buildGroup(this, static_cast<int>(i));
    }
  }
  return true;
}

smtk::attribute::ItemPtr GroupItem::findInternal(const std::string& childName, SearchStyle style)
{
  return this->find(0, childName, style);
}

smtk::attribute::ConstItemPtr GroupItem::findInternal(
  const std::string& childName, SearchStyle style) const
{
  return this->find(0, childName, style);
}

smtk::attribute::ItemPtr GroupItem::find(
  std::size_t element, const std::string& inName, SearchStyle style)
{
  // Make sure element is valid
  if (m_items.size() <= element)
  {
    return nullptr;
  }

  // Lets see if we can find it in the group's items
  for (auto& item : m_items[element])
  {
    if (item->name() == inName)
    {
      return item;
    }
  }

  if ((style == IMMEDIATE) || (style == IMMEDIATE_ACTIVE))
  {
    return nullptr; // its not amoung the group's item
  }

  // Lets check the children
  for (auto& item : m_items[element])
  {
    ItemPtr result = item->find(inName, style);
    if (result)
    {
      return result;
    }
  }
  return nullptr;
}

smtk::attribute::ConstItemPtr GroupItem::find(
  std::size_t element, const std::string& inName, SearchStyle style) const
{
  // Make sure element is valid
  if (m_items.size() <= element)
  {
    return nullptr;
  }

  // Lets see if we can find it in the group's items
  for (auto& item : m_items[element])
  {
    if (item->name() == inName)
    {
      return item;
    }
  }

  if ((style == IMMEDIATE) || (style == IMMEDIATE_ACTIVE))
  {
    return nullptr; // its not amoung the group's items
  }

  // Lets check the children
  for (auto& item : m_items[element])
  {
    ConstItemPtr result = item->find(inName, style);
    if (result)
    {
      return result;
    }
  }
  return nullptr;
}

bool GroupItem::assign(ConstItemPtr& sourceItem, unsigned int options)
{
  // Assigns my contents to be same as sourceItem
  // Cast input pointer to GroupItem
  smtk::shared_ptr<const GroupItem> sourceGroupItem =
    smtk::dynamic_pointer_cast<const GroupItem>(sourceItem);

  if (!sourceGroupItem)
  {
    return false; // Source is not a group item
  }

  // Update children (items)
  this->setNumberOfGroups(sourceGroupItem->numberOfGroups());
  for (std::size_t i = 0; i < sourceGroupItem->numberOfGroups(); ++i)
  {
    for (std::size_t j = 0; j < sourceGroupItem->numberOfItemsPerGroup(); ++j)
    {
      ConstItemPtr sourceChildItem =
        smtk::const_pointer_cast<const Item>(sourceGroupItem->item(i, j));
      ItemPtr childItem = this->item(i, j);
      if (!childItem->assign(sourceChildItem, options))
      {
        std::cerr << "ERROR:Failed to assign child item: " << this->item(i, j)->name() << "\n";
        return false;
      }
    }
  }
  return Item::assign(sourceItem, options);
}
