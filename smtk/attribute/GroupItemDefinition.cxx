//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/GroupItem.h"
#include <iostream>

using namespace smtk::attribute;

GroupItemDefinition::GroupItemDefinition(const std::string& myName)
  : ItemDefinition(myName)
  , m_numberOfRequiredGroups(1)
  , m_maxNumberOfGroups(0)
  , m_isExtensible(false)
  , m_useCommonLabel(false)
{
}

GroupItemDefinition::~GroupItemDefinition()
{
}

Item::Type GroupItemDefinition::type() const
{
  return Item::GROUP;
}

smtk::attribute::ItemPtr GroupItemDefinition::buildItem(
  Attribute* owningAttribute, int itemPosition) const
{
  return smtk::attribute::ItemPtr(new GroupItem(owningAttribute, itemPosition));
}

smtk::attribute::ItemPtr GroupItemDefinition::buildItem(
  Item* owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new GroupItem(owningItem, itemPosition, subGroupPosition));
}

bool GroupItemDefinition::addItemDefinition(smtk::attribute::ItemDefinitionPtr cdef)
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

void GroupItemDefinition::buildGroup(GroupItem* groupItem, int subGroupPosition) const
{
  std::size_t i, n = this->m_itemDefs.size();
  std::vector<smtk::attribute::ItemPtr>& items =
    groupItem->m_items[static_cast<size_t>(subGroupPosition)];
  items.resize(n);
  for (i = 0; i < n; i++)
  {
    items[i] = this->m_itemDefs[i]->buildItem(groupItem, static_cast<int>(i), subGroupPosition);
    items[i]->setDefinition(this->m_itemDefs[i]);
  }
}

void GroupItemDefinition::updateCategories()
{
  this->m_categories.clear();
  std::size_t i, n = this->m_itemDefs.size();
  for (i = 0; i < n; i++)
  {
    this->m_itemDefs[i]->updateCategories();
    const std::set<std::string>& itemCats = this->m_itemDefs[i]->categories();
    this->m_categories.insert(itemCats.begin(), itemCats.end());
  }
}

void GroupItemDefinition::addCategory(const std::string& /*category*/)
{
  std::cerr << "Cannot add categories to a group item definition. "
            << "The name is " << this->name() << std::endl;
}

void GroupItemDefinition::removeCategory(const std::string& /*category*/)
{
  std::cerr << "Cannot remove categories to a group item definition. "
            << "The name is " << this->name() << std::endl;
}

void GroupItemDefinition::setSubGroupLabel(std::size_t element, const std::string& elabel)
{
  if (this->m_isExtensible)
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

void GroupItemDefinition::setCommonSubGroupLabel(const std::string& elabel)
{
  if (this->m_labels.size() != 1)
  {
    this->m_labels.resize(1);
  }
  this->m_useCommonLabel = true;
  this->m_labels[0] = elabel;
}

std::string GroupItemDefinition::subGroupLabel(std::size_t element) const
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

bool GroupItemDefinition::setMaxNumberOfGroups(std::size_t esize)
{
  if (esize && (esize < this->m_numberOfRequiredGroups))
  {
    return false;
  }
  this->m_maxNumberOfGroups = esize;
  return true;
}

bool GroupItemDefinition::setNumberOfRequiredGroups(std::size_t gsize)
{
  if (gsize == this->m_numberOfRequiredGroups)
  {
    return true;
  }
  std::size_t maxN = this->maxNumberOfGroups();
  if (maxN && (gsize > maxN))
  {
    return false;
  }

  this->m_numberOfRequiredGroups = gsize;
  if (!this->hasSubGroupLabels())
  {
    return true;
  }
  if (!(this->m_useCommonLabel || this->m_isExtensible))
  {
    this->m_labels.resize(gsize);
  }
  return true;
}

void GroupItemDefinition::setIsExtensible(bool mode)
{
  this->m_isExtensible = mode;
  if (mode && !this->usingCommonSubGroupLabel())
  {
    // Need to clear individual labels - can only use common label with
    // extensible groups
    this->setCommonSubGroupLabel("");
  }
}

smtk::attribute::ItemDefinitionPtr smtk::attribute::GroupItemDefinition::createCopy(
  smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  (void)info;

  std::size_t i;

  smtk::attribute::GroupItemDefinitionPtr instance =
    smtk::attribute::GroupItemDefinition::New(this->name());
  ItemDefinition::copyTo(instance);

  // Copy item definitions
  for (i = 0; i < m_itemDefs.size(); ++i)
  {
    smtk::attribute::ItemDefinitionPtr itemDef = m_itemDefs[i]->createCopy(info);
    instance->addItemDefinition(itemDef);
  }

  instance->setIsExtensible(m_isExtensible);
  instance->setNumberOfRequiredGroups(m_numberOfRequiredGroups);
  instance->setMaxNumberOfGroups(m_maxNumberOfGroups);

  // Labels
  if (m_useCommonLabel)
  {
    instance->setCommonSubGroupLabel(m_labels[0]);
  }
  else if (this->hasSubGroupLabels())
  {
    for (i = 0; i < m_labels.size(); ++i)
    {
      instance->setSubGroupLabel(i, m_labels[i]);
    }
  }

  return instance;
}
