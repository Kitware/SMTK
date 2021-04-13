//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/ItemDefinition.h"
#include <iostream>
using namespace smtk::attribute;

ItemDefinition::ItemDefinition(const std::string& myName)
  : m_name(myName)
{
  m_version = 0;
  m_advanceLevel[0] = m_advanceLevel[1] = 0;
  m_localAdvanceLevel[0] = m_localAdvanceLevel[1] = 0;
  m_hasLocalAdvanceLevelInfo[0] = m_hasLocalAdvanceLevelInfo[1] = false;
  m_isOptional = false;
  m_isEnabledByDefault = false;
  m_isOkToInherit = true;
}

ItemDefinition::~ItemDefinition() = default;

void ItemDefinition::applyCategories(
  const smtk::attribute::Categories& inheritedFromParent,
  smtk::attribute::Categories& inheritedToParent)
{
  // The item's definition's categories are it's local categories and (if its ok to inherit
  // categories from it's owning item definition/attribute definition) those inherited
  // from its parent
  m_categories.reset();
  m_categories.insert(m_localCategories);
  if (m_isOkToInherit)
  {
    m_categories.insert(inheritedFromParent);
  }
  inheritedToParent.insert(m_localCategories);
}

void ItemDefinition::setLocalAdvanceLevel(int mode, unsigned int level)
{
  if ((mode < 0) || (mode > 1))
  {
    return;
  }
  m_hasLocalAdvanceLevelInfo[mode] = true;
  m_localAdvanceLevel[mode] = m_advanceLevel[mode] = level;
}

void ItemDefinition::setLocalAdvanceLevel(unsigned int level)
{
  m_hasLocalAdvanceLevelInfo[0] = m_hasLocalAdvanceLevelInfo[1] = true;
  m_advanceLevel[0] = m_advanceLevel[1] = m_localAdvanceLevel[0] = m_localAdvanceLevel[1] = level;
}

void ItemDefinition::unsetLocalAdvanceLevel(int mode)
{
  if ((mode < 0) || (mode > 1))
  {
    return;
  }
  m_hasLocalAdvanceLevelInfo[mode] = false;
}

void ItemDefinition::applyAdvanceLevels(
  const unsigned int& readLevelFromParent,
  const unsigned int& writeLevelFromParent)
{
  if (!m_hasLocalAdvanceLevelInfo[0])
  {
    m_advanceLevel[0] = readLevelFromParent;
  }
  if (!m_hasLocalAdvanceLevelInfo[1])
  {
    m_advanceLevel[1] = writeLevelFromParent;
  }
}
void ItemDefinition::copyTo(ItemDefinitionPtr def) const
{
  def->setLabel(m_label);
  def->setVersion(m_version);
  def->setIsOptional(m_isOptional);
  def->setIsEnabledByDefault(m_isEnabledByDefault);
  def->setIsOkToInherit(m_isOkToInherit);
  def->localCategories() = m_localCategories;

  if (m_hasLocalAdvanceLevelInfo[0])
  {
    def->setLocalAdvanceLevel(0, m_localAdvanceLevel[0]);
  }

  if (m_hasLocalAdvanceLevelInfo[1])
  {
    def->setLocalAdvanceLevel(1, m_localAdvanceLevel[1]);
  }

  def->setDetailedDescription(m_detailedDescription);
  def->setBriefDescription(m_briefDescription);
}

const Tag* ItemDefinition::tag(const std::string& name) const
{
  const Tag* tag = nullptr;

  auto t = m_tags.find(Tag(name));
  if (t != m_tags.end())
  {
    tag = &(*t);
  }

  return tag;
}

Tag* ItemDefinition::tag(const std::string& name)
{
  const Tag* tag = nullptr;

  auto t = m_tags.find(Tag(name));
  if (t != m_tags.end())
  {
    tag = &(*t);
  }

  // Tags are ordered according to their name. This name is set at construction,
  // and there is deliberately no API to modify the name after construction. Tag
  // values are editable, however. Rather than make Tag values mutable, we
  // perform a const_cast here to facilitate Tag value modification. This does
  // not change the ordering of the Tag in the Tags set, so we do not break our
  // contract with std::set.
  return const_cast<Tag*>(tag);
}

bool ItemDefinition::addTag(const Tag& tag)
{
  auto t = m_tags.find(tag);
  if (t == m_tags.end())
  {
    m_tags.insert(tag);
    return true;
  }
  return false;
}

bool ItemDefinition::removeTag(const std::string& name)
{
  auto t = m_tags.find(Tag(name));
  if (t != m_tags.end())
  {
    m_tags.erase(t);
    return true;
  }
  return false;
}
