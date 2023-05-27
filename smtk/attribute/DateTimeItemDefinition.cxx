//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/DateTimeItemDefinition.h"
#include "smtk/attribute/DateTimeItem.h"

namespace sc = smtk::common;
using namespace smtk::attribute;

DateTimeItemDefinition::DateTimeItemDefinition(const std::string& myName)
  : ItemDefinition(myName)
{
}

DateTimeItemDefinition::~DateTimeItemDefinition() = default;

Item::Type DateTimeItemDefinition::type() const
{
  return Item::DateTimeType;
}

bool DateTimeItemDefinition::setDefaultValue(const sc::DateTimeZonePair& value)
{
  m_defaultValue = value;
  m_hasDefault = true;
  return true;
}

bool DateTimeItemDefinition::setNumberOfRequiredValues(std::size_t esize)
{
  m_numberOfRequiredValues = esize;
  return true;
}

bool DateTimeItemDefinition::isValueValid(const sc::DateTimeZonePair& /*value*/) const
{
  // Currently, all values are valid
  // Later might have range checking
  return true;
}

smtk::attribute::ItemPtr DateTimeItemDefinition::buildItem(
  Attribute* owningAttribute,
  int itemPosition) const
{
  return smtk::attribute::ItemPtr(new DateTimeItem(owningAttribute, itemPosition));
}

smtk::attribute::ItemPtr
DateTimeItemDefinition::buildItem(Item* owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new DateTimeItem(owningItem, itemPosition, subGroupPosition));
}

ItemDefinitionPtr DateTimeItemDefinition::createCopy(ItemDefinition::CopyInfo& info) const
{
  (void)info;

  DateTimeItemDefinitionPtr newDef = DateTimeItemDefinition::New(this->name());
  ItemDefinition::copyTo(newDef);

  if (m_hasDefault)
  {
    newDef->setDefaultValue(m_defaultValue);
  }
  newDef->setNumberOfRequiredValues(m_numberOfRequiredValues);
  newDef->setDisplayFormat(m_displayFormat);
  newDef->setUseTimeZone(m_useTimeZone);
  newDef->setEnableCalendarPopup(m_useCalendarPopup);

  return newDef;
}
