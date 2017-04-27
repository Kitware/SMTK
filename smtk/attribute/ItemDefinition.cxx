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
  this->m_version = 0;
  this->m_advanceLevel[0] = 0;
  this->m_advanceLevel[1] = 0;
  this->m_isOptional = false;
  this->m_isEnabledByDefault = false;
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

void ItemDefinition::updateCategories()
{
}

void ItemDefinition::addCategory(const std::string& category)
{
  this->m_categories.insert(category);
}

void ItemDefinition::removeCategory(const std::string& category)
{
  this->m_categories.erase(category);
}

void ItemDefinition::setAdvanceLevel(int mode, int level)
{
  if ((mode < 0) || (mode > 1))
  {
    return;
  }
  this->m_advanceLevel[mode] = level;
}

void ItemDefinition::setAdvanceLevel(int level)
{
  this->m_advanceLevel[0] = level;
  this->m_advanceLevel[1] = level;
}

void ItemDefinition::copyTo(ItemDefinitionPtr def) const
{
  def->setLabel(m_label);
  def->setVersion(m_version);
  def->setIsOptional(m_isOptional);
  def->setIsEnabledByDefault(m_isEnabledByDefault);

  std::set<std::string>::const_iterator categoryIter = m_categories.begin();
  for (; categoryIter != m_categories.end(); categoryIter++)
  {
    def->addCategory(*categoryIter);
  }

  def->setAdvanceLevel(0, m_advanceLevel[0]);
  def->setAdvanceLevel(1, m_advanceLevel[1]);

  def->setDetailedDescription(m_detailedDescription);
  def->setBriefDescription(m_briefDescription);
}
