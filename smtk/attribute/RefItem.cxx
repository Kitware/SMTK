//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/System.h"
#include <iostream>
#include <stdio.h>

using namespace smtk::attribute;

RefItem::RefItem(Attribute *owningAttribute,
                 int itemPosition):
  Item(owningAttribute, itemPosition)
{
}

RefItem::RefItem(Item *inOwningItem,
                 int itemPosition,
                 int mySubGroupPosition):
  Item(inOwningItem, itemPosition, mySubGroupPosition)
{
}

bool RefItem::
setDefinition(smtk::attribute::ConstItemDefinitionPtr adef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const RefItemDefinition *def =
    dynamic_cast<const RefItemDefinition *>(adef.get());

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

RefItem::~RefItem()
{
  this->clearAllReferences();
}

void RefItem::clearAllReferences()
{
  std::size_t i, n = this->m_values.size();
  Attribute *att;
  for (i = 0; i < n; i++)
    {
    att = this->m_values[i].lock().get();
    if (att)
      {
      att->removeReference(this);
      }
    }
}

Item::Type RefItem::type() const
{
  return ATTRIBUTE_REF;
}

bool RefItem::isValid() const
{
  // If the item is not enabled or if all of its values are set then it is valid
  // else it is enabled and contains unset values making it invalid
  if (!this->isEnabled())
    {
    return true;
    }
  for (auto it = this->m_values.begin(); it != this->m_values.end(); ++it)
    {
    // Is the attribute this is referencing valid?
    auto att = (*it).lock();
    if (!att)
      {
      return false;
      }
    }

 return true;
}

std::size_t RefItem::numberOfRequiredValues() const
{
  const RefItemDefinition *def =
    static_cast<const RefItemDefinition*>(this->m_definition.get());
  if (def == NULL)
    {
    return 0;
    }
  return def->numberOfRequiredValues();
}

bool RefItem::setValue(std::size_t element, smtk::attribute::AttributePtr att)
{
  const RefItemDefinition *def =
    static_cast<const RefItemDefinition *>(this->definition().get());
  if (def->isValueValid(att))
    {
    assert(this->m_values.size() > element);
    Attribute *attPtr = this->m_values[element].lock().get();
    if (attPtr != NULL)
      {
      attPtr->removeReference(this, element);
      }
    this->m_values[element] = att;
    att->addReference(this, element);
    return true;
    }
  return false;
}

std::string
RefItem::valueAsString(std::size_t element,
                       const std::string &format) const
{
  // For the initial design we will use sprintf and force a limit of 300 char
  char dummy[300];
  assert(this->m_values.size() > element);
  if (format != "")
    {
    sprintf(dummy, format.c_str(), this->m_values[element].lock()->name().c_str());
    }
  else
    {
    sprintf(dummy, "%s", this->m_values[element].lock()->name().c_str());
    }
  return dummy;
}

/**\brief Return an iterator to the first attribute-reference value in this item.
  *
  */
RefItem::const_iterator RefItem::begin() const
{
  return this->m_values.begin();
}

/**\brief Return an iterator just past the last attribute-reference value in this item.
  *
  */
RefItem::const_iterator RefItem::end() const
{
  return this->m_values.end();
}

bool
RefItem::appendValue(smtk::attribute::AttributePtr val)
{
  //First - are we allowed to change the number of values?
  const RefItemDefinition *def =
    static_cast<const RefItemDefinition *>(this->definition().get());
  if (def->numberOfRequiredValues() != 0)
    {
    return false; // The number of values is fixed
    }

  if (def->isValueValid(val))
    {
    this->m_values.push_back(val);
    val->addReference(this, this->m_values.size() - 1);
    return true;
    }
  return false;
}

bool
RefItem::removeValue(std::size_t element)
{
  //First - are we allowed to change the number of values?
  const RefItemDefinition *def =
    static_cast<const RefItemDefinition *>(this->definition().get());
  if (def->numberOfRequiredValues() != 0)
    {
    return false; // The number of values is fixed
    }
  // Tell the attribute we are no longer referencing it (if needed)
  assert(this->m_values.size() > element);
  Attribute *att = this->m_values[element].lock().get();
  if (att != NULL)
    {
    att->removeReference(this, element);
    }
  this->m_values.erase(this->m_values.begin()+element);
  return true;
}

bool
RefItem::setNumberOfValues(std::size_t newSize)
{
  // If the current size is the same just return
  if (this->numberOfValues() == newSize)
    {
    return true;
    }

  //Next - are we allowed to change the number of values?
  const RefItemDefinition *def =
    static_cast<const RefItemDefinition *>(this->definition().get());
  std::size_t n = def->numberOfRequiredValues();
  if (n)
    {
    return false; // The number of values is fixed
    }
  if (newSize < n)
    {
    std::size_t i;
    Attribute *att;
    assert(this->m_values.size() >= n);
    for (i = newSize; i < n; i++)
      {
      att = this->m_values[i].lock().get();
      if (att != NULL)
        {
        att->removeReference(this, i);
        }
      }
    }
  this->m_values.resize(newSize);
  return true;
}

void
RefItem::unset(std::size_t element)
{
  assert(this->m_values.size() > element);
  Attribute *att = this->m_values[element].lock().get();
  if (att == NULL)
    {
    return;
    }
  this->m_values[element].reset();
  // See if we need to tell the attribute we are no longer referencing it
  if (!att->isAboutToBeDeleted())
    {
    att->removeReference(this, element);
    }
}

void
RefItem::reset()
{
  const RefItemDefinition *def
    = static_cast<const RefItemDefinition *>(this->definition().get());
  // Was the initial size 0?
  size_t i, n = def->numberOfRequiredValues();
  if (!n)
    {
    this->clearAllReferences();
    this->m_values.clear();
    return;
    }
  for (i = 0; i < n; i++)
    {
    this->unset(i);
    }
  Item::reset();
}

bool RefItem::assign(ConstItemPtr &sourceItem, unsigned int options)
{
  // Assigns my contents to be same as sourceItem
  // Cast input pointer to RefItem
  smtk::shared_ptr<const RefItem > sourceRefItem =
    smtk::dynamic_pointer_cast<const RefItem>(sourceItem);

  if (!sourceRefItem)
    {
    return false; // Source is not a RefItem
    }

  if (options & Item::IGNORE_ATTRIBUTE_REF_ITEMS)
    {
    return Item::assign(sourceItem, options);
    }

  // Get reference to attribute system
  System *system = this->attribute()->system();

  // Update values, copying as practical
  this->setNumberOfValues(sourceRefItem->numberOfValues());
  for (std::size_t i=0; i<sourceRefItem->numberOfValues(); ++i)
    {
    if (sourceRefItem->isSet(i))
      {
      std::string nameStr = sourceRefItem->value()->name();
      AttributePtr att = system->findAttribute(nameStr);
      if (!att)
        {
        att = system->copyAttribute(sourceRefItem->value(),
                                    (options & Item::COPY_MODEL_ASSOCIATIONS) != 0,
                                    options);
        if (!att)
          {
          std::cerr << "ERROR: Could not copy Attribute:"
                    << sourceRefItem->value()->name() << " referenced by item: "
                    << sourceItem->name() << "\n";
          return false; // Something went wrong!
          }
        }
      this->setValue(i, att);
      }
    else
      {
      this->unset(i);
      }
    }

  return Item::assign(sourceItem, options);
}
