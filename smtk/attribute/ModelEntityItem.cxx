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


#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/model/Cursor.h"

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

  //Next - are we allowed to change the number of values?
  const ModelEntityItemDefinition* def =
    static_cast<const ModelEntityItemDefinition *>(this->definition().get());
  std::size_t n = def->numberOfRequiredValues();
  if (n)
    {
    return false; // The number of values is fixed
    }
  this->m_values.resize(newSize);
  return true;
}

/// Return the \a i-th entity stored in this item.
smtk::model::Cursor ModelEntityItem::value(std::size_t i) const
{
  if (i >= static_cast<std::size_t>(this->m_values.size()))
    return smtk::model::Cursor();
  return this->m_values[i];
}

/**\brief Set the entity stored with this item.
  *
  * This always sets the 0-th item and is a convenience method
  * for cases where only 1 value is needed.
  */
bool ModelEntityItem::setValue(const smtk::model::Cursor& val)
{
  return this->setValue(0, val);
}

/// Set the \a i-th value to the given item. This method does no checking to see if \a i is valid.
bool ModelEntityItem::setValue(std::size_t i, const smtk::model::Cursor& val)
{
  const ModelEntityItemDefinition* def =
    static_cast<const ModelEntityItemDefinition *>(this->definition().get());
  if (def->isValueValid(val))
    {
    this->m_values[i] = val;
    return true;
    }
  return false;
}

bool ModelEntityItem::appendValue(const smtk::model::Cursor& val)
{
  // First - are we allowed to change the number of values?
  const ModelEntityItemDefinition* def =
    static_cast<const ModelEntityItemDefinition *>(this->definition().get());
  if (def->numberOfRequiredValues() != 0)
    {
    return false; // The number of values is fixed
    }

  if (def->isValueValid(val))
    {
    this->m_values.push_back(val);
    return true;
    }
  return false;
}

bool ModelEntityItem::removeValue(std::size_t i)
{
  //First - are we allowed to change the number of values?
  const ModelEntityItemDefinition *def =
    static_cast<const ModelEntityItemDefinition *>(this->definition().get());
  if (def->numberOfRequiredValues() != 0)
    {
    return false; // The number of values is fixed
    }
  this->m_values.erase(this->m_values.begin()+i);
  return true;
}

void ModelEntityItem::reset()
{
}

/// A convenience method to obtain the first value in the item as a string.
std::string ModelEntityItem::valueAsString() const
{
  return this->valueAsString(0);
}

/// Return the value of the \a i-th model item as a string. This returns the UUID of the entity.
std::string ModelEntityItem::valueAsString(std::size_t i) const
{
  smtk::model::Cursor val = this->value(i);
  return val.entity().toString();
}

/// Return whether the \a i-th value is set (i.e., a valid model entity).
bool ModelEntityItem::isSet(std::size_t i) const
{
  return this->m_values[i].isValid();
}

/// Force the \a i-th value of the item to be invalid.
void ModelEntityItem::unset(std::size_t i)
{
  this->setValue(i, smtk::model::Cursor());
}

/// Assigns contents to be same as source item
void ModelEntityItem::copyFrom(ItemPtr sourceItem, CopyInfo& info)
{
  Item::copyFrom(sourceItem, info);

  // Cast input pointer to ModelEntityItem
  ModelEntityItemPtr sourceModelEntityItem =
    smtk::dynamic_pointer_cast<ModelEntityItem>(sourceItem);

  // Update values
  // Only set values if both att managers are using the same model
  this->setNumberOfValues(sourceModelEntityItem->numberOfValues());
  for (std::size_t i=0; i<sourceModelEntityItem->numberOfValues(); ++i)
    {
    if (sourceModelEntityItem->isSet(i) && info.IsSameModel)
      {
      smtk::model::Cursor val = sourceModelEntityItem->value(i);
      this->setValue(i, val);
      }
    else
      {
      this->unset(i);
      }
    }
}
