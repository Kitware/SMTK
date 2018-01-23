//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"

using namespace smtk::attribute;

ComponentItem::ComponentItem(Attribute* owningAttribute, int itemPosition)
  : Item(owningAttribute, itemPosition)
{
}

ComponentItem::ComponentItem(Item* inOwningItem, int itemPosition, int mySubGroupPosition)
  : Item(inOwningItem, itemPosition, mySubGroupPosition)
{
}

ComponentItem::~ComponentItem()
{
}

Item::Type ComponentItem::type() const
{
  return ComponentType;
}

bool ComponentItem::isValid() const
{
  if (!this->isEnabled())
  {
    return true;
  }
  // Do we have at least the number of required values present?
  if (this->numberOfValues() < this->numberOfRequiredValues())
  {
    return false;
  }
  for (auto it = this->m_values.begin(); it != this->m_values.end(); ++it)
  {
    // If the pointer is NULL then it's unset:
    if (!(*it))
    {
      return false;
    }
  }
  return true;
}

std::size_t ComponentItem::numberOfValues() const
{
  return this->m_values.size();
}

bool ComponentItem::setNumberOfValues(std::size_t newSize)
{
  // If the current size is the same just return
  if (this->numberOfValues() == newSize)
  {
    return true;
  }

  // Next - are we allowed to change the number of values?
  const ComponentItemDefinition* def =
    static_cast<const ComponentItemDefinition*>(this->definition().get());
  if (!def->isExtensible())
    return false; // You may not resize.

  // Next - are we within the prescribed limits?
  std::size_t n = def->numberOfRequiredValues();
  if (newSize < n)
    return false; // The number of values requested is too small.

  n = def->maxNumberOfValues();
  if (n > 0 && newSize > n)
    return false; // The number of values requested is too large.

  this->m_values.resize(newSize);
  return true;
}

smtk::attribute::ConstComponentItemDefinitionPtr ComponentItem::definition() const
{
  smtk::attribute::ConstComponentItemDefinitionPtr ptr =
    smtk::dynamic_pointer_cast<const smtk::attribute::ComponentItemDefinition>(this->m_definition);
  return ptr;
}

std::size_t ComponentItem::numberOfRequiredValues() const
{
  const ComponentItemDefinition* def =
    static_cast<const ComponentItemDefinition*>(this->m_definition.get());
  if (def == NULL)
  {
    return 0;
  }
  return def->numberOfRequiredValues();
}

smtk::resource::ComponentPtr ComponentItem::value(std::size_t i) const
{
  if (i >= static_cast<std::size_t>(this->m_values.size()))
    return smtk::resource::ComponentPtr();
  auto result = this->m_values[i];
  return result;
}

bool ComponentItem::setValue(smtk::resource::ComponentPtr val)
{
  return this->setValue(0, val);
}

bool ComponentItem::setValue(std::size_t i, smtk::resource::ComponentPtr val)
{
  const ComponentItemDefinition* def =
    static_cast<const ComponentItemDefinition*>(this->definition().get());
  if (i < this->m_values.size() && def->isValueValid(val))
  {
    this->m_values[i] = val;
    return true;
  }
  return false;
}

bool ComponentItem::appendValue(smtk::resource::ComponentPtr val)
{
  // First - is this value valid?
  const ComponentItemDefinition* def =
    static_cast<const ComponentItemDefinition*>(this->definition().get());
  if (!def->isValueValid(val))
  {
    return false;
  }

  // Second - is the value already in the item?
  std::size_t emptyIndex, n = this->numberOfValues();
  bool foundEmpty = false;
  for (std::size_t i = 0; i < n; ++i)
  {
    if (this->isSet(i) && (this->value(i) == val))
    {
      return true;
    }
    if (!this->isSet(i))
    {
      foundEmpty = true;
      emptyIndex = i;
    }
  }
  // If not, was there a space available?
  if (foundEmpty)
  {
    return this->setValue(emptyIndex, val);
  }
  // Finally - are we allowed to change the number of values?
  if ((def->isExtensible() && def->maxNumberOfValues() &&
        this->m_values.size() >= def->maxNumberOfValues()) ||
    (!def->isExtensible() && this->m_values.size() >= def->numberOfRequiredValues()))
  {
    // The number of values is fixed or we reached the max number of items
    return false;
  }

  this->m_values.push_back(val);
  return true;
}

bool ComponentItem::removeValue(std::size_t i)
{
  //First - are we allowed to change the number of values?
  const ComponentItemDefinition* def =
    static_cast<const ComponentItemDefinition*>(this->definition().get());
  if (!def->isExtensible())
  {
    return this->setValue(i, smtk::resource::ComponentPtr()); // The number of values is fixed
  }

  this->m_values.erase(this->m_values.begin() + i);
  return true;
}

void ComponentItem::reset()
{
  this->m_values.clear();
  if (this->numberOfRequiredValues() > 0)
    this->m_values.resize(this->numberOfRequiredValues());
}

std::string ComponentItem::valueAsString() const
{
  return this->valueAsString(0);
}

std::string ComponentItem::valueAsString(std::size_t i) const
{
  smtk::resource::ComponentPtr val = this->value(i);
  std::ostringstream str;
  str << "[" << val->resource()->id() << "," << val->id() << "]";
  return str.str();
}

bool ComponentItem::isSet(std::size_t i) const
{
  return i < this->m_values.size() ? !!this->m_values[i] : false;
}

void ComponentItem::unset(std::size_t i)
{
  this->setValue(i, smtk::resource::ComponentPtr());
}

bool ComponentItem::assign(ConstItemPtr& sourceItem, unsigned int options)
{
  // Cast input pointer to ComponentItem
  smtk::shared_ptr<const ComponentItem> sourceComponentItem =
    smtk::dynamic_pointer_cast<const ComponentItem>(sourceItem);

  if (!sourceComponentItem)
  {
    return false; // Source is not a model entity item
  }
  // Are we suppose to assign the model enity values?
  if (options & Item::IGNORE_RESOURCE_COMPONENTS)
  {
    return Item::assign(sourceItem, options);
  }

  // Update values
  // Only set values if both att collections are using the same model
  this->setNumberOfValues(sourceComponentItem->numberOfValues());
  for (std::size_t i = 0; i < sourceComponentItem->numberOfValues(); ++i)
  {
    if (sourceComponentItem->isSet(i))
    {
      smtk::resource::ComponentPtr val = sourceComponentItem->value(i);
      this->setValue(i, val);
    }
    else
    {
      this->unset(i);
    }
  }
  return Item::assign(sourceItem, options);
}

bool ComponentItem::isExtensible() const
{
  smtk::attribute::ConstComponentItemDefinitionPtr def =
    smtk::dynamic_pointer_cast<const ComponentItemDefinition>(this->definition());
  if (!def)
    return false;
  return def->isExtensible();
}

bool ComponentItem::has(const smtk::common::UUID& entity) const
{
  return this->find(entity) >= 0;
}

bool ComponentItem::has(smtk::resource::ComponentPtr entity) const
{
  return this->find(entity) >= 0;
}

smtk::resource::ComponentArray::const_iterator ComponentItem::begin() const
{
  return this->m_values.begin();
}

smtk::resource::ComponentArray::const_iterator ComponentItem::end() const
{
  return this->m_values.end();
}

std::ptrdiff_t ComponentItem::find(const smtk::common::UUID& uid) const
{
  std::ptrdiff_t idx = 0;
  smtk::resource::ComponentArray::const_iterator it;
  for (it = this->begin(); it != this->end(); ++it, ++idx)
  {
    if ((*it)->id() == uid)
    {
      return idx;
    }
  }
  return -1;
}

std::ptrdiff_t ComponentItem::find(smtk::resource::ComponentPtr comp) const
{
  std::ptrdiff_t idx = 0;
  smtk::resource::ComponentArray::const_iterator it;
  for (it = this->begin(); it != this->end(); ++it, ++idx)
  {
    if (*it == comp)
    {
      return idx;
    }
  }
  return -1;
}

bool ComponentItem::isWritable() const
{
  const ComponentItemDefinition* def =
    static_cast<const ComponentItemDefinition*>(this->definition().get());
  if (!def)
  {
    return true;
  }
  return def->isWritable();
}

bool ComponentItem::setDefinition(smtk::attribute::ConstItemDefinitionPtr adef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const ComponentItemDefinition* def = dynamic_cast<const ComponentItemDefinition*>(adef.get());

  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if ((def == NULL) || (!Item::setDefinition(adef)))
  {
    return false;
  }
  std::size_t n = def->numberOfRequiredValues();
  if (n != 0)
  {
    this->m_values.resize(n);
  }
  return true;
}
