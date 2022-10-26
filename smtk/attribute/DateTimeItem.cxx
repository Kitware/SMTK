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

#include "smtk/io/Logger.h"

namespace sc = smtk::common;
namespace smtk
{
namespace attribute
{

DateTimeItem::DateTimeItem(Attribute* owningAttribute, int itemPosition)
  : Item(owningAttribute, itemPosition)
{
}

DateTimeItem::DateTimeItem(Item* inOwningAttribute, int itemPosition, int mySubGroupPosition)
  : Item(inOwningAttribute, itemPosition, mySubGroupPosition)
{
}

DateTimeItem::~DateTimeItem() = default;

Item::Type DateTimeItem::type() const
{
  return Item::DateTimeType;
}

bool DateTimeItem::isValidInternal(bool useCategories, const std::set<std::string>& categories)
  const
{
  // If we have been given categories we need to see if the item passes its
  // category checks - if it doesn't it means its not be taken into account
  // for validity checking so just return true

  if (useCategories && !this->categories().passes(categories))
  {
    return true;
  }

  // Check to see if  all of its values are set
  for (auto it = m_isSet.begin(); it != m_isSet.end(); ++it)
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
    m_values.resize(newSize, def->defaultValue());
    m_isSet.resize(newSize, true);
  }
  else
  {
    m_values.resize(newSize);
    m_isSet.resize(newSize, false);
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

bool DateTimeItem::setValue(std::size_t element, const ::smtk::common::DateTimeZonePair& value)
{
  ConstDateTimeItemDefinitionPtr def = this->itemDefinition();
  if (def->isValueValid(value))
  {
    assert(m_values.size() > element);
    m_values[element] = value;
    assert(m_isSet.size() > element);
    m_isSet[element] = true;
    return true;
  }
  return false;
}

void DateTimeItem::reset()
{
  m_isSet.clear();
  m_values.clear();

  ConstDateTimeItemDefinitionPtr dtDef = itemDefinition();
  std::size_t numValues = dtDef->numberOfRequiredValues();
  if (dtDef->hasDefault())
  {
    m_isSet.resize(numValues, true);
    m_values.resize(numValues, dtDef->defaultValue());
  }
  else
  {
    m_isSet.resize(numValues, false);
    m_values.resize(numValues);
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
  return this->setValue(element, def->defaultValue());
}

bool DateTimeItem::isUsingDefault(std::size_t element) const
{
  assert(m_isSet.size() > element);
  ConstDateTimeItemDefinitionPtr def = itemDefinition();
  if (!def->hasDefault() || (!m_isSet[element]))
  {
    return false;
  }

  sc::DateTimeZonePair defaultVal = def->defaultValue();
  return m_values[element] == defaultVal;
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
  for (std::size_t i = 0; i < numValues; ++i)
  {
    if (!m_isSet[i] || !(m_values[i] == defaultVal))
    {
      return false;
    }
  }

  return true;
}

bool DateTimeItem::hasDefault() const
{
  const DateTimeItemDefinition* def =
    static_cast<const DateTimeItemDefinition*>(m_definition.get());
  if (!def)
  {
    return false;
  }
  return def->hasDefault();
}

bool DateTimeItem::assign(
  const smtk::attribute::ConstItemPtr& sourceItem,
  const CopyAssignmentOptions& options,
  smtk::io::Logger& logger)
{
  // Assigns my contents to be same as sourceItem
  ConstDateTimeItemPtr sourceDateTimeItem =
    smtk::dynamic_pointer_cast<const DateTimeItem>(sourceItem);
  if (!sourceDateTimeItem)
  {
    smtkErrorMacro(logger, "Source Item: " << name() << " is not a DateTimeItem");
    return false; // Source is not a DateTimeItem!
  }

  std::size_t numVals, myNumVals, sourceNumVals;
  myNumVals = this->numberOfValues();
  sourceNumVals = sourceDateTimeItem->numberOfValues();
  // Lets initially assumes we can fit all of the source's values
  numVals = sourceNumVals;

  if (myNumVals == sourceNumVals)
  {
    m_isSet = sourceDateTimeItem->m_isSet;
    m_values = sourceDateTimeItem->m_values;
  }
  else if (myNumVals < sourceNumVals)
  {
    // Ok so the source has more values than we can deal with - was partial copying permitted?
    if (options.itemOptions.allowPartialValues())
    {
      numVals = myNumVals;
      smtkInfoMacro(
        logger,
        "Item: " << this->name() << "'s number of values (" << myNumVals
                 << ") is smaller than source Item's number of values (" << sourceNumVals
                 << ") - will partially copy the values");
    }
    else
    {
      smtkErrorMacro(
        logger,
        "Item: " << this->name() << "'s number of values (" << myNumVals
                 << ") can not hold source Item's number of values (" << sourceNumVals
                 << ") and Partial Copying was not permitted");
      return false;
    }
  }

  for (std::size_t i = 0; i < numVals; ++i)
  {
    if (sourceDateTimeItem->isSet(i))
    {
      if (!this->setValue(i, sourceDateTimeItem->value(i)))
      {
        if (options.itemOptions.allowPartialValues())
        {
          smtkInfoMacro(
            logger,
            "Could not assign Value:" << sourceDateTimeItem->value(i).serialize()
                                      << " to DateTimeItem: " << sourceItem->name());
          this->unset(i);
        }
        else
        {
          smtkErrorMacro(
            logger,
            "Could not assign Value:" << sourceDateTimeItem->value(i).serialize()
                                      << " to DateTimeItem: " << sourceItem->name()
                                      << " and allowPartialValues options was not specified.");
          return false;
        }
      }
    }
    else
    {
      this->unset(i);
    }
  }

  return Item::assign(sourceItem, options, logger);
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
      m_isSet.resize(numValues, true);
      m_values.resize(numValues, dtDef->defaultValue());
    }
    else
    {
      m_isSet.clear();
      m_isSet.resize(numValues, false);
      m_values.clear();
      m_values.resize(numValues);
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

} // namespace attribute
} // namespace smtk
