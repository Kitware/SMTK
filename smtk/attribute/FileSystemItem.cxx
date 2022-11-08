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
#include "smtk/io/Logger.h"
#include <cassert>
#include <cstdio>
#include <iostream>

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
  if ((def == nullptr) || (!Item::setDefinition(adef)))
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
      m_values.resize(n, def->defaultValue());
      m_isSet.resize(n, true);
    }
    else
    {
      m_isSet.resize(n, false);
      m_values.resize(n);
    }
  }
  return true;
}

FileSystemItem::~FileSystemItem() = default;

bool FileSystemItem::isValidInternal(bool useCategories, const std::set<std::string>& categories)
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

bool FileSystemItem::isExtensible() const
{
  const auto* def = static_cast<const FileSystemItemDefinition*>(m_definition.get());
  if (!def)
  {
    return false;
  }
  return def->isExtensible();
}

std::size_t FileSystemItem::numberOfRequiredValues() const
{
  const FileSystemItemDefinition* def =
    static_cast<const FileSystemItemDefinition*>(m_definition.get());
  if (def == nullptr)
  {
    return 0;
  }
  return def->numberOfRequiredValues();
}

std::size_t FileSystemItem::maxNumberOfValues() const
{
  const FileSystemItemDefinition* def =
    static_cast<const FileSystemItemDefinition*>(m_definition.get());
  if (def == nullptr)
  {
    return 0;
  }
  return def->maxNumberOfValues();
}

bool FileSystemItem::shouldBeRelative() const
{
  const FileSystemItemDefinition* def =
    static_cast<const FileSystemItemDefinition*>(this->definition().get());
  if (def != nullptr)
  {
    return def->shouldBeRelative();
  }
  return true;
}

bool FileSystemItem::shouldExist() const
{
  const FileSystemItemDefinition* def =
    static_cast<const FileSystemItemDefinition*>(this->definition().get());
  if (def != nullptr)
  {
    return def->shouldExist();
  }
  return true;
}

bool FileSystemItem::setValue(std::size_t element, const std::string& val)
{
  const FileSystemItemDefinition* def =
    static_cast<const FileSystemItemDefinition*>(this->definition().get());
  if ((def == nullptr) || (def->isValueValid(val)))
  {
    assert(m_values.size() > element);
    assert(m_isSet.size() > element);
    m_values[element] = val;
    m_isSet[element] = true;
    return true;
  }
  return false;
}

std::string FileSystemItem::valueAsString(std::size_t element, const std::string& format) const
{
  if (!this->isSet(element))
  {
    return "";
  }

  // For the initial design we will use sprintf and force a limit of 300 char
  char dummy[300];
  assert(m_values.size() > element);
  if (!format.empty())
  {
    sprintf(dummy, format.c_str(), m_values[element].c_str());
  }
  else
  {
    sprintf(dummy, "%s", m_values[element].c_str());
  }
  return dummy;
}

/**\brief Return an iterator to the first directory value in this item.
  *
  */
FileSystemItem::const_iterator FileSystemItem::begin() const
{
  return m_values.begin();
}

/**\brief Return an iterator just past the last directory value in this item.
  *
  */
FileSystemItem::const_iterator FileSystemItem::end() const
{
  return m_values.end();
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
  const auto* def = static_cast<const FileSystemItemDefinition*>(m_definition.get());
  if (def->isValueValid(val))
  {
    m_values.push_back(val);
    m_isSet.push_back(true);
    return true;
  }
  return false;
}

bool FileSystemItem::removeValue(std::size_t i)
{
  // If i < the required number of values this is the same as unset - else if
  // its extensible remove it completely
  const auto* def = static_cast<const FileSystemItemDefinition*>(this->definition().get());
  if (i < def->numberOfRequiredValues())
  {
    this->unset(i);
    return true;
  }
  if (i >= this->numberOfValues())
  {
    return false; // i can't be greater than the number of values
  }
  m_values.erase(m_values.begin() + i);
  m_isSet.erase(m_isSet.begin() + i);
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
  // Is the new size smaller than the number of required values?
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
    m_values.resize(newSize, this->defaultValue());
    m_isSet.resize(newSize, true);
  }
  else
  {
    m_values.resize(newSize);
    m_isSet.resize(newSize, false); //Any added values are not set
  }
  return true;
}

bool FileSystemItem::hasDefault() const
{
  const auto* def = static_cast<const FileSystemItemDefinition*>(this->definition().get());
  if (!def)
  {
    return false;
  }
  return def->hasDefault();
}

bool FileSystemItem::setToDefault(std::size_t element)
{
  const auto* def = static_cast<const FileSystemItemDefinition*>(this->definition().get());
  if (!def->hasDefault())
  {
    return false; // Doesn't have a default value
  }

  this->setValue(element, def->defaultValue());
  return true;
}

bool FileSystemItem::isUsingDefault() const
{
  const auto* def = static_cast<const FileSystemItemDefinition*>(this->definition().get());
  if (!def->hasDefault())
  {
    return false; // Doesn't have a default value
  }

  std::size_t i, n = this->numberOfValues();
  std::string dval = def->defaultValue();
  assert(m_isSet.size() >= n);
  assert(m_values.size() >= n);
  for (i = 0; i < n; i++)
  {
    if (!(m_isSet[i] && (m_values[i] == dval)))
    {
      return false;
    }
  }
  return true;
}

bool FileSystemItem::isUsingDefault(std::size_t element) const
{
  const auto* def = static_cast<const FileSystemItemDefinition*>(this->definition().get());
  assert(m_isSet.size() > element);
  assert(m_values.size() > element);
  return def->hasDefault() && m_isSet[element] && m_values[element] == def->defaultValue();
}

std::string FileSystemItem::defaultValue() const
{
  const auto* def = static_cast<const FileSystemItemDefinition*>(this->definition().get());
  if (!def)
  {
    return "";
  }
  return def->defaultValue();
}

void FileSystemItem::reset()
{
  const auto* def = static_cast<const FileSystemItemDefinition*>(this->definition().get());
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

bool FileSystemItem::assign(
  const smtk::attribute::ConstItemPtr& sourceItem,
  const CopyAssignmentOptions& options,
  smtk::io::Logger& logger)
{
  // Assigns my contents to be same as sourceItem

  smtk::shared_ptr<const FileSystemItem> sourceFSItem =
    smtk::dynamic_pointer_cast<const FileSystemItem>(sourceItem);

  if (!sourceFSItem)
  {
    smtkErrorMacro(logger, "Source Item: " << name() << " is not a FileSytemItem");
    return false;
  }

  this->setNumberOfValues(sourceFSItem->numberOfValues());
  // Were we able to allocate enough space to fit all of the source's values?
  std::size_t myNumVals, sourceNumVals, numVals;
  myNumVals = this->numberOfValues();
  sourceNumVals = sourceFSItem->numberOfValues();
  if (myNumVals < sourceNumVals)
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
        "FileSytemItem: " << name() << "'s number of values (" << myNumVals
                          << ") can not hold source FileSytemItem's number of values ("
                          << sourceNumVals << ") and Partial Copying was not permitted");
      return false;
    }
  }
  else
  {
    numVals = sourceNumVals;
  }

  for (std::size_t i = 0; i < numVals; ++i)
  {
    if (sourceFSItem->isSet(i))
    {
      if (!this->setValue(i, sourceFSItem->value(i)))
      {
        if (options.itemOptions.allowPartialValues())
        {
          smtkInfoMacro(
            logger,
            "Could not assign Value:" << sourceFSItem->value(i)
                                      << " to DateTimeItem: " << sourceItem->name());
          this->unset(i);
        }
        else
        {
          smtkErrorMacro(
            logger,
            "Could not assign Value:" << sourceFSItem->value(i)
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
