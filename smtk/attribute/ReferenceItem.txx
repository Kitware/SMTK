//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_ReferenceItem_txx
#define smtk_attribute_ReferenceItem_txx

#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"

#include "smtk/attribute/ReferenceItemDefinition.txx"

namespace smtk
{

namespace attribute
{

template <typename T>
ReferenceItem<T>::ReferenceItem(Attribute* owningAttribute, int itemPosition)
  : Item(owningAttribute, itemPosition)
{
}

template <typename T>
ReferenceItem<T>::ReferenceItem(Item* inOwningItem, int itemPosition, int mySubGroupPosition)
  : Item(inOwningItem, itemPosition, mySubGroupPosition)
{
}

template <typename T>
ReferenceItem<T>::~ReferenceItem()
{
}

template <typename T>
bool ReferenceItem<T>::isValid() const
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
  for (auto it = m_values.begin(); it != m_values.end(); ++it)
  {
    // If the pointer is NULL then it's unset:
    if (!(*it))
    {
      return false;
    }
  }
  return true;
}

template <typename T>
std::size_t ReferenceItem<T>::numberOfValues() const
{
  return m_values.size();
}

template <typename T>
bool ReferenceItem<T>::setNumberOfValues(std::size_t newSize)
{
  // If the current size is the same just return
  if (this->numberOfValues() == newSize)
  {
    return true;
  }

  // Next - are we allowed to change the number of values?
  auto def = static_cast<const ReferenceItemDefinition<T>*>(m_definition.get());
  if (!def->isExtensible())
    return false; // You may not resize.

  // Next - are we within the prescribed limits?
  std::size_t n = def->numberOfRequiredValues();
  if (newSize < n)
    return false; // The number of values requested is too small.

  n = def->maxNumberOfValues();
  if (n > 0 && newSize > n)
    return false; // The number of values requested is too large.

  m_values.resize(newSize);
  return true;
}

template <typename T>
std::shared_ptr<const ReferenceItemDefinition<T> > ReferenceItem<T>::definition() const
{
  auto ptr =
    smtk::dynamic_pointer_cast<const smtk::attribute::ReferenceItemDefinition<T> >(m_definition);
  return ptr;
}

template <typename T>
std::size_t ReferenceItem<T>::numberOfRequiredValues() const
{
  auto def = static_cast<const ReferenceItemDefinition<T>*>(m_definition.get());
  if (!def)
  {
    return 0;
  }
  return def->numberOfRequiredValues();
}

template <typename T>
std::size_t ReferenceItem<T>::maxNumberOfValues() const
{
  auto def = static_cast<const ReferenceItemDefinition<T>*>(m_definition.get());
  if (!def)
  {
    return 0;
  }
  return def->maxNumberOfValues();
}

template <typename T>
typename T::Ptr ReferenceItem<T>::value(std::size_t i) const
{
  if (i >= static_cast<std::size_t>(m_values.size()))
    return nullptr;
  auto result = m_values[i];
  return result;
}

template <typename T>
bool ReferenceItem<T>::setValue(typename T::Ptr val)
{
  return this->setValue(0, val);
}

template <typename T>
bool ReferenceItem<T>::setValue(std::size_t i, typename T::Ptr val)
{
  auto def = static_cast<const ReferenceItemDefinition<T>*>(this->definition().get());
  if (i < m_values.size() && def->isValueValid(val))
  {
    m_values[i] = val;
    return true;
  }
  return false;
}

template <typename T>
bool ReferenceItem<T>::appendValue(typename T::Ptr val)
{
  // First - is this value valid?
  auto def = static_cast<const ReferenceItemDefinition<T>*>(this->definition().get());
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
        m_values.size() >= def->maxNumberOfValues()) ||
    (!def->isExtensible() && m_values.size() >= def->numberOfRequiredValues()))
  {
    // The number of values is fixed or we reached the max number of items
    return false;
  }

  m_values.push_back(val);
  return true;
}

template <typename T>
bool ReferenceItem<T>::removeValue(std::size_t i)
{
  //First - are we allowed to change the number of values?
  auto def = static_cast<const ReferenceItemDefinition<T>*>(this->definition().get());
  if (!def->isExtensible())
  {
    return this->setValue(i, nullptr); // The number of values is fixed
  }

  m_values.erase(m_values.begin() + i);
  return true;
}

template <typename T>
void ReferenceItem<T>::reset()
{
  m_values.clear();
  if (this->numberOfRequiredValues() > 0)
    m_values.resize(this->numberOfRequiredValues());
}

template <typename T>
std::string ReferenceItem<T>::valueAsString() const
{
  return this->valueAsString(0);
}

template <typename T>
bool ReferenceItem<T>::isSet(std::size_t i) const
{
  return i < m_values.size() ? !!m_values[i] : false;
}

template <typename T>
void ReferenceItem<T>::unset(std::size_t i)
{
  this->setValue(i, nullptr);
}

template <typename T>
bool ReferenceItem<T>::assign(ConstItemPtr& sourceItem, unsigned int options)
{
  // Cast input pointer to ReferenceItem
  auto sourceReferenceItem = smtk::dynamic_pointer_cast<const ReferenceItem>(sourceItem);
  if (!sourceReferenceItem)
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
  this->setNumberOfValues(sourceReferenceItem->numberOfValues());
  for (std::size_t i = 0; i < sourceReferenceItem->numberOfValues(); ++i)
  {
    if (sourceReferenceItem->isSet(i))
    {
      auto val = sourceReferenceItem->value(i);
      this->setValue(i, val);
    }
    else
    {
      this->unset(i);
    }
  }
  return Item::assign(sourceItem, options);
}

template <typename T>
bool ReferenceItem<T>::isExtensible() const
{
  auto def = smtk::dynamic_pointer_cast<const ReferenceItemDefinition<T> >(this->definition());
  if (!def)
    return false;
  return def->isExtensible();
}

template <typename T>
bool ReferenceItem<T>::has(const smtk::common::UUID& entity) const
{
  return this->find(entity) >= 0;
}

template <typename T>
bool ReferenceItem<T>::has(typename T::Ptr entity) const
{
  return this->find(entity) >= 0;
}

template <typename T>
typename ReferenceItem<T>::const_iterator ReferenceItem<T>::begin() const
{
  return m_values.begin();
}

template <typename T>
typename ReferenceItem<T>::const_iterator ReferenceItem<T>::end() const
{
  return m_values.end();
}

template <typename T>
std::ptrdiff_t ReferenceItem<T>::find(const smtk::common::UUID& uid) const
{
  std::ptrdiff_t idx = 0;
  const_iterator it;
  for (it = this->begin(); it != this->end(); ++it, ++idx)
  {
    if ((*it)->id() == uid)
    {
      return idx;
    }
  }
  return -1;
}

template <typename T>
std::ptrdiff_t ReferenceItem<T>::find(typename T::Ptr comp) const
{
  std::ptrdiff_t idx = 0;
  const_iterator it;
  for (it = this->begin(); it != this->end(); ++it, ++idx)
  {
    if (*it == comp)
    {
      return idx;
    }
  }
  return -1;
}

template <typename T>
smtk::resource::LockType ReferenceItem<T>::lockType() const
{
  auto def = static_cast<const ReferenceItemDefinition<T>*>(this->definition().get());
  if (!def)
  {
    return smtk::resource::LockType::DoNotLock;
  }
  return def->lockType();
}

template <typename T>
bool ReferenceItem<T>::setDefinition(smtk::attribute::ConstItemDefinitionPtr adef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  auto def = dynamic_cast<const ReferenceItemDefinition<T>*>(adef.get());

  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if ((def == NULL) || (!Superclass::setDefinition(adef)))
  {
    return false;
  }
  std::size_t n = def->numberOfRequiredValues();
  if (n != 0)
  {
    m_values.resize(n);
  }
  return true;
}
}
}
#endif
