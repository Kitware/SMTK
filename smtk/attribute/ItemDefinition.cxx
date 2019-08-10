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
  m_advanceLevel[0] = 0;
  m_advanceLevel[1] = 0;
  m_isOptional = false;
  m_isEnabledByDefault = false;
  m_isOkToInherit = true;
}

ItemDefinition::~ItemDefinition()
{
}

bool ItemDefinition::isMemberOf(const std::vector<std::string>& inCategories) const
{
  std::size_t i, n = inCategories.size();
  for (i = 0; i < n; i++)
  {
    if (this->isMemberOf(inCategories[i]))
    {
      return true;
    }
  }
  return false;
}

void ItemDefinition::applyCategories(
  const std::set<std::string>& inheritedFromParent, std::set<std::string>& inheritedToParent)
{
  // The item's definition's categories are it's local categories and (if its ok to inherit
  // categories from it's owning item definition/attribute definition) those inherited
  // from its parent
  m_categories = m_localCategories;
  if (m_isOkToInherit)
  {
    m_categories.insert(inheritedFromParent.begin(), inheritedFromParent.end());
  }
  inheritedToParent.insert(m_localCategories.begin(), m_localCategories.end());
}

void ItemDefinition::addLocalCategory(const std::string& category)
{
  m_localCategories.insert(category);
}

void ItemDefinition::removeLocalCategory(const std::string& category)
{
  m_localCategories.erase(category);
}

void ItemDefinition::setAdvanceLevel(int mode, int level)
{
  if ((mode < 0) || (mode > 1))
  {
    return;
  }
  m_advanceLevel[mode] = level;
}

void ItemDefinition::setAdvanceLevel(int level)
{
  m_advanceLevel[0] = level;
  m_advanceLevel[1] = level;
}

void ItemDefinition::copyTo(ItemDefinitionPtr def) const
{
  def->setLabel(m_label);
  def->setVersion(m_version);
  def->setIsOptional(m_isOptional);
  def->setIsEnabledByDefault(m_isEnabledByDefault);
  def->setIsOkToInherit(m_isOkToInherit);

  std::set<std::string>::const_iterator categoryIter = m_localCategories.begin();
  for (; categoryIter != m_localCategories.end(); categoryIter++)
  {
    def->addLocalCategory(*categoryIter);
  }

  def->setAdvanceLevel(0, m_advanceLevel[0]);
  def->setAdvanceLevel(1, m_advanceLevel[1]);

  def->setDetailedDescription(m_detailedDescription);
  def->setBriefDescription(m_briefDescription);
}
