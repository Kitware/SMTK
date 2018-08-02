//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"

#include "smtk/resource/Component.h"

#include <cassert>
#include <sstream>

namespace smtk
{

namespace attribute
{

ReferenceItem::ReferenceItem(Attribute* owningAttribute, int itemPosition)
  : Item(owningAttribute, itemPosition)
{
}

ReferenceItem::ReferenceItem(Item* inOwningItem, int itemPosition, int mySubGroupPosition)
  : Item(inOwningItem, itemPosition, mySubGroupPosition)
{
}

ReferenceItem::~ReferenceItem()
{
}

bool ReferenceItem::isValid() const
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

  // Do all objects resolve to a valid reference?
  //
  // NOTE: we const_cast here because we treat m_values as a cache variable with
  //       lazy evalutation semantics. We could declare m_values to be mutable
  //       instead.
  if (const_cast<ReferenceItem*>(this)->resolve() == false)
  {
    return false;
  }

  return true;
}

std::size_t ReferenceItem::numberOfValues() const
{
  return m_values.size();
}

bool ReferenceItem::setNumberOfValues(std::size_t newSize)
{
  // If the current size is the same just return
  if (this->numberOfValues() == newSize)
  {
    return true;
  }

  // Next - are we allowed to change the number of values?
  auto def = static_cast<const ReferenceItemDefinition*>(m_definition.get());
  if (!def->isExtensible())
    return false; // You may not resize.

  // Next - are we within the prescribed limits?
  std::size_t n = def->numberOfRequiredValues();
  if (newSize < n)
    return false; // The number of values requested is too small.

  n = def->maxNumberOfValues();
  if (n > 0 && newSize > n)
    return false; // The number of values requested is too large.

  m_keys.resize(newSize);
  m_values.resize(newSize);
  return true;
}

std::shared_ptr<const ReferenceItemDefinition> ReferenceItem::definition() const
{
  auto ptr =
    smtk::dynamic_pointer_cast<const smtk::attribute::ReferenceItemDefinition>(m_definition);
  return ptr;
}

std::size_t ReferenceItem::numberOfRequiredValues() const
{
  auto def = static_cast<const ReferenceItemDefinition*>(m_definition.get());
  if (!def)
  {
    return 0;
  }
  return def->numberOfRequiredValues();
}

std::size_t ReferenceItem::maxNumberOfValues() const
{
  auto def = static_cast<const ReferenceItemDefinition*>(m_definition.get());
  if (!def)
  {
    return 0;
  }
  return def->maxNumberOfValues();
}

void ReferenceItem::visit(std::function<bool(PersistentObjectPtr)> visitor) const
{
  for (auto it = this->begin(); it != this->end(); ++it)
  {
    if (!visitor(*it))
    {
      break;
    }
  }
}

smtk::attribute::ReferenceItem::Key ReferenceItem::objectKey(std::size_t i) const
{
  if (i >= static_cast<std::size_t>(m_values.size()))
    return Key();
  return m_keys[i];
}

bool ReferenceItem::setObjectKey(std::size_t i, const smtk::attribute::ReferenceItem::Key& key)
{
  if (i < m_values.size())
  {
    this->attribute()->links().removeLink(m_keys[i]);
    m_keys[i] = key;
    return true;
  }
  return false;
}

smtk::resource::PersistentObjectPtr ReferenceItem::objectValue(std::size_t i) const
{
  if (i >= static_cast<std::size_t>(m_values.size()))
    return nullptr;
  if (m_values[i] == nullptr)
  {
    // NOTE: we const_cast here because we treat m_values as a cache variable
    //       with lazy evalutation semantics. We could declare m_values to be
    //       mutable instead.
    const_cast<ReferenceItem*>(this)->m_values[i] = this->objectValue(m_keys[i]);
  }
  auto result = m_values[i];
  return result;
}

bool ReferenceItem::setObjectValue(PersistentObjectPtr val)
{
  return this->setObjectValue(0, val);
}

ReferenceItem::Key ReferenceItem::linkTo(PersistentObjectPtr val)
{
  // If the object is a component...
  if (auto component = std::dynamic_pointer_cast<smtk::resource::Component>(val))
  {
    return this->attribute()->links().addLinkTo(component, -1);
  }
  // If the object is a resource...
  else if (auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(val))
  {
    return this->attribute()->links().addLinkTo(resource, -1);
  }

  // If the object cannot be cast to a resource or component, there's not much
  // we can do.
  return ReferenceItem::Key();
}

bool ReferenceItem::setObjectValue(std::size_t i, PersistentObjectPtr val)
{
  auto def = static_cast<const ReferenceItemDefinition*>(this->definition().get());
  if (i < m_values.size() && (val == nullptr || def->isValueValid(val)))
  {
    this->attribute()->links().removeLink(m_keys[i]);
    m_keys[i] = this->linkTo(val);
    m_values[i] = val;
    return true;
  }
  return false;
}

bool ReferenceItem::appendObjectValue(PersistentObjectPtr val)
{
  // First - is this value valid?
  auto def = static_cast<const ReferenceItemDefinition*>(this->definition().get());
  if (!def->isValueValid(val))
  {
    return false;
  }

  // Second - is the value already in the item?
  std::size_t emptyIndex, n = this->numberOfValues();
  bool foundEmpty = false;
  for (std::size_t i = 0; i < n; ++i)
  {
    if (this->isSet(i) && (this->objectValue(i) == val))
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
    return this->setObjectValue(emptyIndex, val);
  }
  // Finally - are we allowed to change the number of values?
  if ((def->isExtensible() && def->maxNumberOfValues() &&
        m_values.size() >= def->maxNumberOfValues()) ||
    (!def->isExtensible() && m_values.size() >= def->numberOfRequiredValues()))
  {
    // The number of values is fixed or we reached the max number of items
    return false;
  }

  m_keys.push_back(this->linkTo(val));
  m_values.push_back(val);
  return true;
}

bool ReferenceItem::removeValue(std::size_t i)
{
  auto def = static_cast<const ReferenceItemDefinition*>(this->definition().get());
  if (!def->isExtensible())
  {
    return false; // The number of values is fixed
  }
  if (this->numberOfValues() <= def->numberOfRequiredValues())
  {
    return false; // min number of values reached
  }
  this->attribute()->links().removeLink(m_keys[i]);
  m_keys.erase(m_keys.begin() + i);
  m_values.erase(m_values.begin() + i);
  return true;
}

void ReferenceItem::reset()
{
  m_keys.clear();
  m_values.clear();
  if (this->numberOfRequiredValues() > 0)
  {
    m_keys.resize(this->numberOfRequiredValues());
    m_values.resize(this->numberOfRequiredValues());
  }
}

std::string ReferenceItem::valueAsString() const
{
  return this->valueAsString(0);
}

std::string ReferenceItem::valueAsString(std::size_t i) const
{
  std::ostringstream result;
  auto entry = this->objectValue(i);
  smtk::resource::ResourcePtr rsrc;
  smtk::resource::ComponentPtr comp;
  if ((rsrc = std::dynamic_pointer_cast<smtk::resource::Resource>(entry)))
  {
    result << "resource(" << rsrc->id().toString() << ")";
  }
  else if ((comp = std::dynamic_pointer_cast<smtk::resource::Component>(entry)))
  {
    rsrc = comp->resource();
    smtk::common::UUID rid;
    if (rsrc)
    {
      rid = rsrc->id();
    }
    result << "component(" << rid << "," << comp->id() << ")";
  }
  else
  {
    result << "unknown(" << (m_values[i] ? m_values[i]->id().toString() : "") << ")";
  }
  return result.str();
}

bool ReferenceItem::isSet(std::size_t i) const
{
  return i < m_keys.size() ? !m_keys[i].first.isNull() : false;
}

void ReferenceItem::unset(std::size_t i)
{
  this->setObjectValue(i, nullptr);
}

bool ReferenceItem::assign(ConstItemPtr& sourceItem, unsigned int options)
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
  // Only set values if both att resources are using the same model
  this->setNumberOfValues(sourceReferenceItem->numberOfValues());
  for (std::size_t i = 0; i < sourceReferenceItem->numberOfValues(); ++i)
  {
    if (sourceReferenceItem->isSet(i))
    {
      auto val = sourceReferenceItem->objectValue(i);
      this->setObjectValue(i, val);
    }
    else
    {
      this->unset(i);
    }
  }
  return Item::assign(sourceItem, options);
}

bool ReferenceItem::isExtensible() const
{
  auto def = smtk::dynamic_pointer_cast<const ReferenceItemDefinition>(this->definition());
  if (!def)
    return false;
  return def->isExtensible();
}

bool ReferenceItem::has(const smtk::common::UUID& entity) const
{
  return this->find(entity) >= 0;
}

bool ReferenceItem::has(PersistentObjectPtr entity) const
{
  return this->find(entity) >= 0;
}

typename ReferenceItem::const_iterator ReferenceItem::begin() const
{
  // NOTE: we const_cast here because we treat m_values as a cache variable with
  //       lazy evalutation semantics. We could declare m_values to be mutable
  //       instead.
  const_cast<ReferenceItem*>(this)->resolve();

  return m_values.begin();
}

typename ReferenceItem::const_iterator ReferenceItem::end() const
{
  return m_values.end();
}

std::ptrdiff_t ReferenceItem::find(const smtk::common::UUID& uid) const
{
  std::ptrdiff_t idx = 0;
  const_iterator it;
  for (it = this->begin(); it != this->end(); ++it, ++idx)
  {
    if (*it && (*it)->id() == uid)
    {
      return idx;
    }
  }
  return -1;
}

std::ptrdiff_t ReferenceItem::find(PersistentObjectPtr comp) const
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

smtk::resource::LockType ReferenceItem::lockType() const
{
  auto def = static_cast<const ReferenceItemDefinition*>(this->definition().get());
  if (!def)
  {
    return smtk::resource::LockType::DoNotLock;
  }
  return def->lockType();
}

bool ReferenceItem::setDefinition(smtk::attribute::ConstItemDefinitionPtr adef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  auto def = dynamic_cast<const ReferenceItemDefinition*>(adef.get());

  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if ((def == NULL) || (!Superclass::setDefinition(adef)))
  {
    return false;
  }
  std::size_t n = def->numberOfRequiredValues();
  if (n != 0)
  {
    m_keys.resize(n);
    m_values.resize(n);
  }
  return true;
}

smtk::resource::PersistentObjectPtr ReferenceItem::objectValue(const ReferenceItem::Key& key) const
{
  // We first try to resolve the item as a component.
  auto linkedComponent = this->attribute()->links().linkedComponent(key);
  if (linkedComponent.first != nullptr)
  {
    // We can resolve the linked component.
    return linkedComponent.first;
  }

  // We then try to resolve the item as a resource.
  auto linkedResource = this->attribute()->links().linkedResource(key);
  if (linkedResource.first != nullptr)
  {
    // We can resolve the linked resource.
    return linkedResource.first;
  }
  return PersistentObjectPtr();
}

bool ReferenceItem::resolve()
{
  bool allResolved = true;

  // We treat keys and values as vectors in lockstep with each other. If they
  // are not, then something unexpected has occured.
  assert(m_keys.size() == m_values.size());

  // Iterate over the objects' keys and values.
  auto key = m_keys.begin();
  auto value = m_values.begin();
  for (; value != m_values.end(); ++value, ++key)
  {
    // If a value is not currently resolved...
    if (*value == nullptr)
    {
      // ...set it equal to the object pointer accessed using its key.
      *value = this->objectValue(*key);

      // If it's still not resolved...
      if (*value == nullptr)
      {
        // ...there's not much we can do.
        allResolved = false;
      }
    }
  }
  return allResolved;
}
}
}
