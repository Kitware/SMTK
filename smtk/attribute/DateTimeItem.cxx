//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/DateTimeItemDefinition.h"

namespace sc = smtk::common;
namespace smtk {
  namespace attribute {


DateTimeItem::DateTimeItem(Attribute *owningAttribute, int itemPosition):
  Item(owningAttribute, itemPosition)
{
}

DateTimeItem::DateTimeItem(
  Item *inOwningAttribute, int itemPosition, int mySubGroupPosition):
  Item(inOwningAttribute, itemPosition, mySubGroupPosition)
{
}

DateTimeItem::~DateTimeItem()
{
}

Item::Type DateTimeItem::type() const
{
  return Item::DATE_TIME;
}

bool DateTimeItem::isValid() const
{
  // If the item is not enabled or if all of its values are set then it is valid
  // else it is enabled and contains unset values making it invalid
  if (!this->isEnabled())
    {
    return true;
    }
  for (auto it = this->m_isSet.begin(); it != this->m_isSet.end(); ++it)
    {
    if (!(*it))
      {
      return false;
      }
    }
  return true;
}

bool DateTimeItem::setNumberOfValues(std::size_t newSize)
{
  if (newSize != this->numberOfRequiredValues())
    {
    return false;
    }

  ConstDateTimeItemDefinitionPtr def = this->itemDefinition();
  if (def->hasDefault())
    {
    this->m_values.resize(newSize, def->defaultValue());
    this->m_isSet.resize(newSize, true);
    }
  else
    {
    this->m_values.resize(newSize);
    this->m_isSet.resize(newSize, false);
    }
  return true;
}

std::size_t DateTimeItem::numberOfRequiredValues() const
{
  ConstDateTimeItemDefinitionPtr def = this->itemDefinition();
  if (!def)
    {
    return 0;
    }
  return def->numberOfRequiredValues();
}

bool DateTimeItem::setValue(
  size_t element, const ::smtk::common::DateTimeZonePair& value)
{
  ConstDateTimeItemDefinitionPtr def = this->itemDefinition();
  if (def->isValueValid(value))
    {
    assert(this->m_values.size() > element);
    this->m_values[element] = value;
    assert(this->m_isSet.size() > element);
    this->m_isSet[element] = true;
    return true;
    }
  return false;
}

void DateTimeItem::reset()
{
  this->m_isSet.clear();
  this->m_values.clear();

  ConstDateTimeItemDefinitionPtr dtDef = itemDefinition();
  std::size_t numValues = dtDef->numberOfRequiredValues();
  if (dtDef->hasDefault())
    {
    this->m_isSet.resize(numValues, true);
    this->m_values.resize(numValues, dtDef->defaultValue());
    }
  else
    {
    this->m_isSet.resize(numValues, false);
    this->m_values.resize(numValues);
    }
}

bool DateTimeItem::setToDefault(std::size_t element)
{
  ConstDateTimeItemDefinitionPtr def = this->itemDefinition();
  if (!def->hasDefault())
    {
    return false;
    }

  // (else)
  this->setValue(element, def->defaultValue());
  return false;
}

bool DateTimeItem::isUsingDefault(std::size_t element) const
{
  assert(this->m_isSet.size() > element);
  ConstDateTimeItemDefinitionPtr def = itemDefinition();
  if (!def->hasDefault() || (!this->m_isSet[element]))
    {
    return false;
    }

  sc::DateTimeZonePair defaultVal = def->defaultValue();
  return this->m_values[element] == defaultVal;
}

bool DateTimeItem::isUsingDefault() const
{
  ConstDateTimeItemDefinitionPtr def = itemDefinition();
  if (!def->hasDefault())
    {
    return false;
    }

  std::size_t numValues = this->numberOfValues();
  sc::DateTimeZonePair defaultVal = def->defaultValue();
  for (std::size_t i=0; i<numValues; ++i)
    {
    if (!this->m_isSet[i] || !(this->m_values[i] == defaultVal))
      {
      return false;
      }
    }

  return true;
}

bool DateTimeItem::assign(ConstItemPtr &sourceItem, unsigned int options)
{
  // Assigns my contents to be same as sourceItem
  ConstDateTimeItemPtr sourceDateTimeItem =
    smtk::dynamic_pointer_cast<const DateTimeItem>(sourceItem);
  if (!sourceDateTimeItem)
    {
    return false; // Source is not a DateTimeItem!
    }

  this->setNumberOfValues(sourceDateTimeItem->numberOfValues());
  for (std::size_t i=0; i<sourceDateTimeItem->numberOfValues(); ++i)
    {
    if (sourceDateTimeItem->isSet(i))
      {
      this->setValue(i, sourceDateTimeItem->value(i));
      }
    else
      {
      this->unset(i);
      }
    }

  return Item::assign(sourceItem, options);
}

bool DateTimeItem::setDefinition(smtk::attribute::ConstItemDefinitionPtr def)
{
  if (!def || (!Item::setDefinition(def)))
    {
    return false;
    }

  ConstDateTimeItemDefinitionPtr dtDef = itemDefinition();
  std::size_t numValues = dtDef->numberOfRequiredValues();
  if (numValues)
    {
    if (dtDef->hasDefault())
      {
      this->m_isSet.resize(numValues, true);
      this->m_values.resize(numValues, dtDef->defaultValue());
      }
    else
      {
      this->m_isSet.clear();
      this->m_isSet.resize(numValues, false);
      this->m_values.clear();
      this->m_values.resize(numValues);
      }
    }

  return true;
}

ConstDateTimeItemDefinitionPtr DateTimeItem::itemDefinition() const
{
  ConstDateTimeItemDefinitionPtr def =
    smtk::dynamic_pointer_cast<const DateTimeItemDefinition>(this->definition());
  return def;
}

  }  // namespace attribute
}  // namespace smtk
