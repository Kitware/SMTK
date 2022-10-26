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

#include "smtk/io/Logger.h"
#include <iostream>

using namespace smtk::attribute;

GroupItem::GroupItem(Attribute* owningAttribute, int itemPosition)
  : Item(owningAttribute, itemPosition)
  , m_maxNumberOfChoices(0)
  , m_minNumberOfChoices(0)
{
}

GroupItem::GroupItem(Item* inOwningItem, int itemPosition, int mySubGroupPosition)
  : Item(inOwningItem, itemPosition, mySubGroupPosition)
  , m_maxNumberOfChoices(0)
  , m_minNumberOfChoices(0)
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

bool GroupItem::isConditional() const
{
  const GroupItemDefinition* def = static_cast<const GroupItemDefinition*>(m_definition.get());
  return def->isConditional();
}

bool GroupItem::conditionalsSatisfied(bool useActiveCategories) const
{
  if (!this->isConditional())
  {
    return true; // We have no conditional requirements
  }

  for (auto it = m_items.begin(); it != m_items.end(); ++it)
  {
    unsigned int numChoices = 0;
    for (auto it1 = (*it).begin(); it1 != (*it).end(); ++it1)
    {
      if (*it1 && (*it1)->isRelevant(useActiveCategories) && (*it1)->isEnabled())
      {
        numChoices++;
      }
    }
    if (
      (numChoices < m_minNumberOfChoices) ||
      ((m_maxNumberOfChoices != 0) && (numChoices > m_maxNumberOfChoices)))
    {
      return false;
    }
  }
  return true;
}

bool GroupItem::isValidInternal(bool useCategories, const std::set<std::string>& categories) const
{
  // Lets see if the group itself would be filtered out based on the categories
  if (useCategories && !this->categories().passes(categories))
  {
    return true;
  }

  // Finally, if all of its values are set then it is valid
  // else it is invalid
  unsigned int numChoices = 0;
  for (auto it = m_items.begin(); it != m_items.end(); ++it)
  {
    for (auto it1 = (*it).begin(); it1 != (*it).end(); ++it1)
    {
      if (useCategories)
      {
        if (!*it1 || !(*it1)->isValid(categories))
        {
          return false;
        }
        else if (*it1 && (*it1)->isEnabled())
        {
          numChoices++;
        }
      }
      else
      {
        if (!*it1 || !(*it1)->isValid())
        {
          return false;
        }
        else if (*it1 && (*it1)->isEnabled())
        {
          numChoices++;
        }
      }
    }

    // Ideally we could just call GroupItem::conditionalsSatisfied but that method
    // assumes we are testing against active categories, while this method is passed
    // in the set of categories to be checked. So we do our own check.
    if (
      this->isConditional() &&
      ((numChoices < m_minNumberOfChoices) ||
       ((m_maxNumberOfChoices != 0) && (numChoices > m_maxNumberOfChoices))))
    {
      return false;
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
  if (def->isConditional())
  {
    m_minNumberOfChoices = def->minNumberOfChoices();
    m_maxNumberOfChoices = def->maxNumberOfChoices();
  }

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

bool GroupItem::insertGroups(std::size_t pos, std::size_t num)
{
  if (!this->isExtensible())
  {
    return false;
  }

  const GroupItemDefinition* def =
    static_cast<const GroupItemDefinition*>(this->definition().get());
  std::size_t maxN = def->maxNumberOfGroups(), n = this->numberOfGroups();

  if ((pos > n) || (maxN && ((n + num) > maxN)))
  {
    // inserting beyond the end of the vector or
    // inserting these groups would violate the max size
    return false;
  }

  std::vector<smtk::attribute::ItemPtr> placeHolder;

  m_items.insert(m_items.begin() + pos, num, placeHolder);

  for (std::size_t i = pos; i < pos + num; i++)
  {
    def->buildGroup(this, static_cast<int>(i));
  }

  return true;
}

bool GroupItem::appendGroup()
{
  return this->insertGroups(m_items.size(), 1);
}

bool GroupItem::prependGroup()
{
  return this->insertGroups(0, 1);
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
    m_items.resize(newSize);
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
  const std::string& childName,
  SearchStyle style) const
{
  return this->find(0, childName, style);
}

smtk::attribute::ItemPtr
GroupItem::find(std::size_t element, const std::string& inName, SearchStyle style)
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

smtk::attribute::ConstItemPtr
GroupItem::find(std::size_t element, const std::string& inName, SearchStyle style) const
{
  // Make sure element is valid
  if (m_items.size() <= element)
  {
    return nullptr;
  }

  // Lets see if we can find it in the group's items
  for (const auto& item : m_items[element])
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
  for (const auto& item : m_items[element])
  {
    ConstItemPtr result = item->find(inName, style);
    if (result)
    {
      return result;
    }
  }
  return nullptr;
}

bool GroupItem::assign(
  const smtk::attribute::ConstItemPtr& sourceItem,
  const CopyAssignmentOptions& options,
  smtk::io::Logger& logger)
{
  // Assigns my contents to be same as sourceItem
  // Cast input pointer to GroupItem
  smtk::shared_ptr<const GroupItem> sourceGroupItem =
    smtk::dynamic_pointer_cast<const GroupItem>(sourceItem);

  if (!sourceGroupItem)
  {
    smtkErrorMacro(logger, "Source Item: " << this->name() << " is not a GroupItem");
    return false; // Source is not a group item
  }

  // Update children (items)
  this->setNumberOfGroups(sourceGroupItem->numberOfGroups());

  // Were we able to allocate enough space to fit all of the source's values?
  std::size_t myNumVals, sourceNumVals, numVals;
  myNumVals = this->numberOfGroups();
  sourceNumVals = sourceGroupItem->numberOfGroups();
  if (myNumVals < sourceNumVals)
  {
    // Ok so the source has more values than we can deal with - was partial copying permitted?
    if (options.itemOptions.allowPartialValues())
    {
      numVals = myNumVals;
      smtkInfoMacro(
        logger,
        "GroupItem: " << this->name() << "'s number of values (" << myNumVals
                      << ") is smaller than source Item's number of values (" << sourceNumVals
                      << ") - will partially copy the values");
    }
    else
    {
      smtkErrorMacro(
        logger,
        "GroupItem: " << this->name() << "'s number of groups (" << myNumVals
                      << ") can not hold source GroupItem's number of groups (" << sourceNumVals
                      << ") and Partial Copying was not permitted");
      return false;
    }
  }
  else
  {
    numVals = sourceNumVals;
  }

  for (std::size_t i = 0; i < numVals; ++i)
  {
    for (std::size_t j = 0; j < sourceGroupItem->numberOfItemsPerGroup(); ++j)
    {
      ConstItemPtr sourceChildItem =
        smtk::const_pointer_cast<const Item>(sourceGroupItem->item(i, j));
      auto childItem = this->find(i, sourceChildItem->name());

      if (childItem == nullptr)
      {
        // Are missing items allowed?
        if (!options.itemOptions.ignoreMissingChildren())
        {
          smtkErrorMacro(
            logger,
            "Could not find Child Item: " << sourceChildItem->name()
                                          << " in GroupItem: " << this->name()
                                          << " and IGNORE MISSING CHILDREN option was not set");
          return false;
        }
        continue;
      }
      if (!childItem->assign(sourceChildItem, options, logger))
      {
        smtkErrorMacro(
          logger,
          "Could not assign GroupItem: " << this->name() << "'s Child Item: " << childItem->name());
        return false;
      }
    }
  }
  return Item::assign(sourceItem, options, logger);
}

bool GroupItem::hasRelevantChildren(
  bool includeCategories,
  bool includeReadAccess,
  int readAccessLevel) const
{
  for (size_t elementIndex = 0; elementIndex < this->numberOfGroups(); elementIndex++)
  {
    for (size_t valueIndex = 0; valueIndex < this->numberOfItemsPerGroup(); valueIndex++)
    {
      if (this->item(elementIndex, valueIndex)
            ->isRelevant(includeCategories, includeReadAccess, readAccessLevel))
      {
        return true;
      }
    }
  }
  return false;
}
