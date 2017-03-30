//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/FileSystemItem.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileSystemItemDefinition.h"
#include <cassert>
#include <iostream>
#include <stdio.h>

using namespace smtk::attribute;

FileSystemItem::FileSystemItem(Attribute* owningAttribute, int itemPosition)
  : Item(owningAttribute, itemPosition)
{
}

FileSystemItem::FileSystemItem(Item* inOwningItem, int itemPosition, int inSubGroupPosition)
  : Item(inOwningItem, itemPosition, inSubGroupPosition)
{
}

bool FileSystemItem::setDefinition(smtk::attribute::ConstItemDefinitionPtr adef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const FileSystemItemDefinition* def = dynamic_cast<const FileSystemItemDefinition*>(adef.get());

  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if ((def == NULL) || (!Item::setDefinition(adef)))
  {
    return false;
  }
  // Find out how many values this item is suppose to have
  // if the size is 0 then its unbounded
  std::size_t n = def->numberOfRequiredValues();
  if (n)
  {
    if (def->hasDefault())
    {
      this->m_values.resize(n, def->defaultValue());
      this->m_isSet.resize(n, true);
    }
    else
    {
      this->m_isSet.resize(n, false);
      this->m_values.resize(n);
    }
  }
  return true;
}

FileSystemItem::~FileSystemItem()
{
}

bool FileSystemItem::isValid() const
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

bool FileSystemItem::isExtensible() const
{
  auto def = static_cast<const FileSystemItemDefinition*>(this->m_definition.get());
  if (!def)
  {
    return false;
  }
  return def->isExtensible();
}

std::size_t FileSystemItem::numberOfRequiredValues() const
{
  const FileSystemItemDefinition* def =
    static_cast<const FileSystemItemDefinition*>(this->m_definition.get());
  if (def == NULL)
  {
    return 0;
  }
  return def->numberOfRequiredValues();
}

std::size_t FileSystemItem::maxNumberOfValues() const
{
  const FileSystemItemDefinition* def =
    static_cast<const FileSystemItemDefinition*>(this->m_definition.get());
  if (def == NULL)
  {
    return 0;
  }
  return def->maxNumberOfValues();
}

bool FileSystemItem::shouldBeRelative() const
{
  const FileSystemItemDefinition* def =
    static_cast<const FileSystemItemDefinition*>(this->definition().get());
  if (def != NULL)
  {
    return def->shouldBeRelative();
  }
  return true;
}

bool FileSystemItem::shouldExist() const
{
  const FileSystemItemDefinition* def =
    static_cast<const FileSystemItemDefinition*>(this->definition().get());
  if (def != NULL)
  {
    return def->shouldExist();
  }
  return true;
}

bool FileSystemItem::setValue(std::size_t element, const std::string& val)
{
  const FileSystemItemDefinition* def =
    static_cast<const FileSystemItemDefinition*>(this->definition().get());
  if ((def == NULL) || (def->isValueValid(val)))
  {
    assert(this->m_values.size() > element);
    assert(this->m_isSet.size() > element);
    this->m_values[element] = val;
    this->m_isSet[element] = true;
    return true;
  }
  return false;
}

std::string FileSystemItem::valueAsString(std::size_t element, const std::string& format) const
{
  // For the initial design we will use sprintf and force a limit of 300 char
  char dummy[300];
  assert(this->m_values.size() > element);
  if (format != "")
  {
    sprintf(dummy, format.c_str(), this->m_values[element].c_str());
  }
  else
  {
    sprintf(dummy, "%s", this->m_values[element].c_str());
  }
  return dummy;
}

/**\brief Return an iterator to the first directory value in this item.
  *
  */
FileSystemItem::const_iterator FileSystemItem::begin() const
{
  return this->m_values.begin();
}

/**\brief Return an iterator just past the last directory value in this item.
  *
  */
FileSystemItem::const_iterator FileSystemItem::end() const
{
  return this->m_values.end();
}

bool FileSystemItem::appendValue(const std::string& val)
{
  //First - are we allowed to change the number of values?
  if (!this->isExtensible())
  {
    return false; // The number of values is fixed
  }
  // See if we have it max number of values (if there is such a limit)
  std::size_t maxN = this->maxNumberOfValues();
  if (maxN && (this->numberOfValues() >= maxN))
  {
    return false;
  }
  auto def = static_cast<const FileSystemItemDefinition*>(this->m_definition.get());
  if (def->isValueValid(val))
  {
    this->m_values.push_back(val);
    this->m_isSet.push_back(true);
    return true;
  }
  return false;
}

bool FileSystemItem::removeValue(int element)
{
  //First - are we allowed to change the number of values?
  if (!this->isExtensible())
  {
    return false; // The number of values is fixed
  }
  // Would removing teh value take us under the number of required values?
  if (this->numberOfValues() <= this->numberOfRequiredValues())
  {
    return false;
  }
  this->m_values.erase(this->m_values.begin() + element);
  this->m_isSet.erase(this->m_isSet.begin() + element);
  return true;
}

bool FileSystemItem::setNumberOfValues(std::size_t newSize)
{
  // If the current size is the same just return
  if (this->numberOfValues() == newSize)
  {
    return true;
  }

  //Next - are we allowed to change the number of values?
  if (!this->isExtensible())
  {
    return false;
  }
  // Is the nre size smaller than the number of required values?
  if (newSize < this->numberOfRequiredValues())
  {
    return false;
  }
  // Is the new size > than the max number of values?
  std::size_t maxN = this->maxNumberOfValues();
  if (maxN && (newSize > maxN))
  {
    return false;
  }
  if (this->hasDefault())
  {
    this->m_values.resize(newSize, this->defaultValue());
    this->m_isSet.resize(newSize, true);
  }
  else
  {
    this->m_values.resize(newSize);
    this->m_isSet.resize(newSize, false); //Any added values are not set
  }
  return true;
}

bool FileSystemItem::hasDefault() const
{
  auto def = static_cast<const FileSystemItemDefinition*>(this->definition().get());
  if (!def)
  {
    return false;
  }
  return def->hasDefault();
}

bool FileSystemItem::setToDefault(std::size_t element)
{
  auto def = static_cast<const FileSystemItemDefinition*>(this->definition().get());
  if (!def->hasDefault())
  {
    return false; // Doesn't have a default value
  }

  this->setValue(element, def->defaultValue());
  return true;
}

bool FileSystemItem::isUsingDefault() const
{
  auto def = static_cast<const FileSystemItemDefinition*>(this->definition().get());
  if (!def->hasDefault())
  {
    return false; // Doesn't have a default value
  }

  std::size_t i, n = this->numberOfValues();
  std::string dval = def->defaultValue();
  assert(this->m_isSet.size() >= n);
  assert(this->m_values.size() >= n);
  for (i = 0; i < n; i++)
  {
    if (!(this->m_isSet[i] && (this->m_values[i] == dval)))
    {
      return false;
    }
  }
  return true;
}

bool FileSystemItem::isUsingDefault(std::size_t element) const
{
  auto def = static_cast<const FileSystemItemDefinition*>(this->definition().get());
  assert(this->m_isSet.size() > element);
  assert(this->m_values.size() > element);
  if (def->hasDefault() && this->m_isSet[element] && this->m_values[element] == def->defaultValue())
  {
    return true;
  }
  return false;
}

std::string FileSystemItem::defaultValue() const
{
  auto def = static_cast<const FileSystemItemDefinition*>(this->definition().get());
  if (!def)
  {
    return "";
  }
  return def->defaultValue();
}

void FileSystemItem::reset()
{
  auto def = static_cast<const FileSystemItemDefinition*>(this->definition().get());
  std::size_t i, n = this->numberOfRequiredValues();
  if (this->numberOfValues() != n)
  {
    this->setNumberOfValues(n);
  }

  if (!def->hasDefault())
  {
    for (i = 0; i < n; i++)
    {
      this->unset(i);
    }
  }
  else
  {
    std::string dval = def->defaultValue();
    for (i = 0; i < n; i++)
    {
      this->setValue(i, dval);
    }
  }
}

bool FileSystemItem::assign(ConstItemPtr& sourceItem, unsigned int options)
{
  // Assigns my contents to be same as sourceItem

  smtk::shared_ptr<const FileSystemItem> sourceDirItem =
    smtk::dynamic_pointer_cast<const FileSystemItem>(sourceItem);

  if (!sourceDirItem)
  {
    return false; // Source is not a directory item!
  }

  this->setNumberOfValues(sourceDirItem->numberOfValues());
  for (std::size_t i = 0; i < sourceDirItem->numberOfValues(); ++i)
  {
    if (sourceDirItem->isSet(i))
    {
      this->setValue(i, sourceDirItem->value(i));
    }
    else
    {
      this->unset(i);
    }
  }

  return Item::assign(sourceItem, options);
}
