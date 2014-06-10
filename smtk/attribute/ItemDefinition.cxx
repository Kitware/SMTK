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


#include "smtk/attribute/ItemDefinition.h"
#include <iostream>
using namespace smtk::attribute; 

//----------------------------------------------------------------------------
ItemDefinition::ItemDefinition(const std::string &myName) : m_name(myName)
{
  this->m_version = 0;
  this->m_advanceLevel[0] = 0;
  this->m_advanceLevel[1] = 0;
  this->m_isOptional = false;
  this->m_isEnabledByDefault = false;
}

//----------------------------------------------------------------------------
ItemDefinition::~ItemDefinition()
{
}
//----------------------------------------------------------------------------
bool ItemDefinition::isMemberOf(const std::vector<std::string> &inCategories) const
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
//----------------------------------------------------------------------------
void ItemDefinition::updateCategories()
{
}
//----------------------------------------------------------------------------
void ItemDefinition::addCategory(const std::string &category)
{
  this->m_categories.insert(category);
}
//----------------------------------------------------------------------------
void ItemDefinition::removeCategory(const std::string &category)
{
  this->m_categories.erase(category);
}
//----------------------------------------------------------------------------
void ItemDefinition::setAdvanceLevel(int mode, int level)
{
  if ((mode < 0) || (mode > 1))
    {
    return;
    }
  this->m_advanceLevel[mode] = level;
}
//----------------------------------------------------------------------------
void ItemDefinition::setAdvanceLevel(int level)
{
  this->m_advanceLevel[0] = level;
  this->m_advanceLevel[1] = level;
}
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
