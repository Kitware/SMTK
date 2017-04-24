//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"

using namespace smtk::attribute;

/// Construct an item given its owning attribute and location in the attribute.
ModelEntityItem::ModelEntityItem(Attribute* owningAttribute, int itemPosition)
  : Item(owningAttribute, itemPosition)
{
}

/// Construct an item given its owning item and position inside the item.
ModelEntityItem::ModelEntityItem(
  Item* inOwningItem, int itemPosition, int mySubGroupPosition):
  Item(inOwningItem, itemPosition, mySubGroupPosition)
{
}

/// Destructor
ModelEntityItem::~ModelEntityItem()
{
}

/// Return the type of storage used by the item.
Item::Type ModelEntityItem::type() const
{
  return MODEL_ENTITY;
}

bool ModelEntityItem::isValid() const
{
  // If the item is not enabled or if all of its values are set then it is valid
  // else it is enabled and contains unset values making it invalid
  if (!this->isEnabled())
    {
    return true;
    }
  // Do we have atleats the number of required values present?
  if(this->numberOfValues() < this->numberOfRequiredValues())
    {
    return false;
    }
  for (auto it = this->m_values.begin(); it != this->m_values.end(); ++it)
    {
    // If the enitity is NULL then its unset
    if (!(*it).entity())
      {
      return false;
      }
    }
  return true;

}

/// Set the definition of this attribute.
bool ModelEntityItem::setDefinition(
  smtk::attribute::ConstItemDefinitionPtr adef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const ModelEntityItemDefinition *def =
    dynamic_cast<const ModelEntityItemDefinition *>(adef.get());

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

/// Return the size of the item (number of entities associated with the item).
std::size_t ModelEntityItem::numberOfValues() const
{
  return this->m_values.size();
}

/// Return the number of values required by this item's definition (if it has one).
std::size_t ModelEntityItem::numberOfRequiredValues() const
{
  const ModelEntityItemDefinition *def =
    static_cast<const ModelEntityItemDefinition*>(this->m_definition.get());
  if (def == NULL)
    {
    return 0;
    }
  return def->numberOfRequiredValues();
}

/// Set the number of entities to be associated with this item (returns true if permitted).
bool ModelEntityItem::setNumberOfValues(std::size_t newSize)
{
  // If the current size is the same just return
  if (this->numberOfValues() == newSize)
    {
    return true;
    }

  // Next - are we allowed to change the number of values?
  const ModelEntityItemDefinition* def =
    static_cast<const ModelEntityItemDefinition *>(this->definition().get());
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

/// Return the \a i-th entity stored in this item.
smtk::model::EntityRef ModelEntityItem::value(std::size_t i) const
{
  if (i >= static_cast<std::size_t>(this->m_values.size()))
    return smtk::model::EntityRef();
  return this->m_values[i];
}

/**\brief Set the entity stored with this item.
  *
  * This always sets the 0-th item and is a convenience method
  * for cases where only 1 value is needed.
  */
bool ModelEntityItem::setValue(const smtk::model::EntityRef& val)
{
  return this->setValue(0, val);
}

/// Set the \a i-th value to the given item. This method does no checking to see if \a i is valid.
bool ModelEntityItem::setValue(std::size_t i, const smtk::model::EntityRef& val)
{
  const ModelEntityItemDefinition* def =
    static_cast<const ModelEntityItemDefinition *>(this->definition().get());
  if (i<this->m_values.size() && def->isValueValid(val))
    {
    this->m_values[i] = val;
    return true;
    }
  return false;
}

bool ModelEntityItem::appendValue(const smtk::model::EntityRef& val)
{
  // First - is this value valid?
  const ModelEntityItemDefinition* def =
    static_cast<const ModelEntityItemDefinition *>(this->definition().get());
  if (!def->isValueValid(val))
    {
    return false;
    }

  // Second - is the value already in the item?
  std::size_t emptyIndex, n = this->numberOfValues();
  bool foundEmpty = false;
  for (std::size_t i = 0; i < n; ++i)
    {
      if (this->isSet(i) && (this->value(i).entity() == val.entity()))
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
  if (
    (def->isExtensible() && def->maxNumberOfValues() && this->m_values.size() >= def->maxNumberOfValues()) ||
    (!def->isExtensible() && this->m_values.size() >= def->numberOfRequiredValues()))
    {
    // The number of values is fixed or we reached the max number of items
    return false;
    }

  this->m_values.push_back(val);
  return true;
}

bool ModelEntityItem::removeValue(std::size_t i)
{
  //First - are we allowed to change the number of values?
  const ModelEntityItemDefinition *def =
    static_cast<const ModelEntityItemDefinition *>(this->definition().get());
  if (!def->isExtensible())
    {
    return this->setValue(i, smtk::model::EntityRef()); // The number of values is fixed
    }

  this->m_values.erase(this->m_values.begin()+i);
  return true;
}

/// This clears the list of values and then fills it with null entities up to the number of required values.
void ModelEntityItem::reset()
{
  this->m_values.clear();
  if (this->numberOfRequiredValues() > 0)
    this->m_values.resize(this->numberOfRequiredValues());
}

/// A convenience method to obtain the first value in the item as a string.
std::string ModelEntityItem::valueAsString() const
{
  return this->valueAsString(0);
}

/// Return the value of the \a i-th model item as a string. This returns the UUID of the entity.
std::string ModelEntityItem::valueAsString(std::size_t i) const
{
  smtk::model::EntityRef val = this->value(i);
  return val.entity().toString();
}

/**\brief Return whether the \a i-th value is set.
  *
  * This returns true when the UUID is non-NULL and false otherwise.
  *
  * Note that this is <b>not always what you would expect</b>!
  * You can set a value to be an invalid, non-NULL UUID so that
  * entities which have been expunged can be reported (and other
  * use cases).
  * If you want to guarantee that particular value is valid or
  * invalid, you should use the EntityRef's isValid() method after
  * fetching the value from the ModelEntityItem.
  */
bool ModelEntityItem::isSet(std::size_t i) const
{
  return i < this->m_values.size() ?
    !!this->m_values[i].entity() :
    false;
}

/// Force the \a i-th value of the item to be invalid.
void ModelEntityItem::unset(std::size_t i)
{
  this->setValue(i, smtk::model::EntityRef());
}

/// Assigns contents to be same as source item
bool ModelEntityItem::assign(ConstItemPtr &sourceItem, unsigned int options)
{
  // Cast input pointer to ModelEntityItem
  smtk::shared_ptr<const ModelEntityItem > sourceModelEntityItem =
    smtk::dynamic_pointer_cast<const ModelEntityItem>(sourceItem);

  if (!sourceModelEntityItem)
    {
    return false; // Source is not a model entity item
    }
  // Are we suppose to assign the model enity values?
  if (options & Item::IGNORE_MODEL_ENTITIES)
    {
      return Item::assign(sourceItem, options);
    }

  // Update values
  // Only set values if both att systems are using the same model
  this->setNumberOfValues(sourceModelEntityItem->numberOfValues());
  for (std::size_t i=0; i<sourceModelEntityItem->numberOfValues(); ++i)
    {
    if (sourceModelEntityItem->isSet(i))
      {
      smtk::model::EntityRef val = sourceModelEntityItem->value(i);
      this->setValue(i, val);
      }
    else
      {
      this->unset(i);
      }
    }
  return Item::assign(sourceItem, options);
}

/// A convenience method returning whether the item's definition is extensible.
bool ModelEntityItem::isExtensible() const
{
  smtk::attribute::ConstModelEntityItemDefinitionPtr def =
    smtk::dynamic_pointer_cast<const ModelEntityItemDefinition>(
      this->definition());
  if (!def)
    return false;
  return def->isExtensible();
}

/**\brief Return true if the entity is associated with this item; false otherwise.
  *
  */
bool ModelEntityItem::has(const smtk::common::UUID& entity) const
{
  return this->find(entity) >= 0;
}

/**\brief
  *
  */
bool ModelEntityItem::has(const smtk::model::EntityRef& entity) const
{
  return this->find(entity) >= 0;
}

/**\brief Return an iterator to the first model-entity value in this item.
  *
  */
smtk::model::EntityRefArray::const_iterator ModelEntityItem::begin() const
{
  return this->m_values.begin();
}

/**\brief Return an iterator just past the last model-entity value in this item.
  *
  */
smtk::model::EntityRefArray::const_iterator ModelEntityItem::end() const
{
  return this->m_values.end();
}

/**\brief
  *
  */
std::ptrdiff_t ModelEntityItem::find(const smtk::common::UUID& entity) const
{
  std::ptrdiff_t idx = 0;
  smtk::model::EntityRefArray::const_iterator it;
  for (it = this->begin(); it != this->end(); ++it, ++idx)
    {
    if (it->entity() == entity)
      {
      return idx;
      }
    }
  return -1;
}

/**\brief
  *
  */
std::ptrdiff_t ModelEntityItem::find(const smtk::model::EntityRef& entity) const
{
  std::ptrdiff_t idx = 0;
  smtk::model::EntityRefArray::const_iterator it;
  for (it = this->begin(); it != this->end(); ++it, ++idx)
    {
    if (*it == entity)
      {
      return idx;
      }
    }
  return -1;
}
