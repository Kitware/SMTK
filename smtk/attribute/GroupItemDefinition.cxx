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

#include <algorithm>
#include <functional>
#include <iostream>

using namespace smtk::attribute;

GroupItemDefinition::GroupItemDefinition(const std::string& myName)
  : ItemDefinition(myName)
{
}

GroupItemDefinition::~GroupItemDefinition() = default;

Item::Type GroupItemDefinition::type() const
{
  return Item::GroupType;
}

smtk::attribute::ItemPtr GroupItemDefinition::buildItem(
  Attribute* owningAttribute,
  int itemPosition) const
{
  return smtk::attribute::ItemPtr(new GroupItem(owningAttribute, itemPosition));
}

smtk::attribute::ItemPtr
GroupItemDefinition::buildItem(Item* owningItem, int itemPosition, int subGroupPosition) const
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
  std::size_t n = m_itemDefs.size();
  m_itemDefs.push_back(cdef);
  m_itemDefPositions[cdef->name()] = static_cast<int>(n);
  // If we represent a set of conditionals then each item should be considered optional.
  if (m_isConditional)
  {
    cdef->setIsOptional(true);
  }
  cdef->setUnitSystem(m_unitSystem);
  return true;
}

void GroupItemDefinition::buildGroup(GroupItem* groupItem, int subGroupPosition) const
{
  std::size_t i, n = m_itemDefs.size();
  std::vector<smtk::attribute::ItemPtr>& items =
    groupItem->m_items[static_cast<size_t>(subGroupPosition)];
  items.resize(n);
  for (i = 0; i < n; i++)
  {
    items[i] = m_itemDefs[i]->buildItem(groupItem, static_cast<int>(i), subGroupPosition);
    items[i]->setDefinition(m_itemDefs[i]);
  }
}

void GroupItemDefinition::applyCategories(
  const smtk::common::Categories::Stack& inheritedFromParent,
  smtk::common::Categories& inheritedToParent)
{
  smtk::common::Categories::Stack myCats = inheritedFromParent;

  myCats.append(m_combinationMode, m_localCategories);
  m_categories.reset();

  smtk::common::Categories myChildrenCats;
  for (auto& item : m_itemDefs)
  {
    item->applyCategories(myCats, myChildrenCats);
  }

  m_categories.insert(myChildrenCats);
  inheritedToParent.insert(m_categories);
}

void GroupItemDefinition::applyAdvanceLevels(
  const unsigned int& readLevelFromParent,
  const unsigned int& writeLevelFromParent)
{
  ItemDefinition::applyAdvanceLevels(readLevelFromParent, writeLevelFromParent);
  for (auto& item : m_itemDefs)
  {
    item->applyAdvanceLevels(m_advanceLevel[0], m_advanceLevel[1]);
  }
}

void GroupItemDefinition::setSubGroupLabel(std::size_t element, const std::string& elabel)
{
  if (m_isExtensible)
  {
    return;
  }
  if (m_labels.size() != m_numberOfRequiredGroups)
  {
    m_labels.resize(m_numberOfRequiredGroups);
  }
  m_useCommonLabel = false;
  m_labels[element] = elabel;
}

void GroupItemDefinition::setCommonSubGroupLabel(const std::string& elabel)
{
  if (m_labels.size() != 1)
  {
    m_labels.resize(1);
  }
  m_useCommonLabel = true;
  m_labels[0] = elabel;
}

std::string GroupItemDefinition::subGroupLabel(std::size_t element) const
{
  if (m_useCommonLabel)
  {
    return m_labels[0];
  }
  if (!m_labels.empty())
  {
    return m_labels[element];
  }
  return ""; // If we threw execeptions this method could return const string &
}

bool GroupItemDefinition::setMaxNumberOfGroups(std::size_t esize)
{
  if (esize && (esize < m_numberOfRequiredGroups))
  {
    return false;
  }
  m_maxNumberOfGroups = esize;
  return true;
}

bool GroupItemDefinition::setNumberOfRequiredGroups(std::size_t gsize)
{
  if (gsize == m_numberOfRequiredGroups)
  {
    return true;
  }
  std::size_t maxN = this->maxNumberOfGroups();
  if (maxN && (gsize > maxN))
  {
    return false;
  }

  m_numberOfRequiredGroups = gsize;
  if (!this->hasSubGroupLabels())
  {
    return true;
  }
  if (!(m_useCommonLabel || m_isExtensible))
  {
    m_labels.resize(gsize);
  }
  return true;
}

void GroupItemDefinition::setIsExtensible(bool mode)
{
  m_isExtensible = mode;
  if (mode && !this->usingCommonSubGroupLabel())
  {
    // Need to clear individual labels - can only use common label with
    // extensible groups
    this->setCommonSubGroupLabel("");
  }
}

ItemDefinitionPtr GroupItemDefinition::createCopy(ItemDefinition::CopyInfo& info) const
{
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

bool GroupItemDefinition::removeItemDefinition(ItemDefinitionPtr itemDef)
{
  if (!itemDef || this->findItemPosition(itemDef->name()) < 0)
  {
    // Not found
    return false;
  }

  auto itItemDef = std::find(m_itemDefs.begin(), m_itemDefs.end(), itemDef);
  if (itItemDef != m_itemDefs.end())
  {
    m_itemDefs.erase(itItemDef);
  }
  m_itemDefPositions.erase(itemDef->name());
  return true;
}

void GroupItemDefinition::setUnitSystem(const shared_ptr<units::System>& unitSystem)
{
  m_unitSystem = unitSystem;

  for (const auto& item : m_itemDefs)
  {
    item->setUnitSystem(m_unitSystem);
  }
}
