//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include <algorithm> // for std::find
#include <iostream>
#include <stdio.h>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
FileItem::FileItem(Attribute *owningAttribute,
                   int itemPosition):
  Item(owningAttribute, itemPosition)
{
}

//----------------------------------------------------------------------------
FileItem::FileItem(Item *inOwningItem,
                   int itemPosition,
                   int inSubGroupPosition):
  Item(inOwningItem, itemPosition, inSubGroupPosition)
{
}

//----------------------------------------------------------------------------
bool FileItem::
setDefinition(smtk::attribute::ConstItemDefinitionPtr adef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const FileItemDefinition *def =
    dynamic_cast<const FileItemDefinition *>(adef.get());

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
    this->m_recentValues.clear();
    }
  return true;
}

//----------------------------------------------------------------------------
FileItem::~FileItem()
{
}
//----------------------------------------------------------------------------
Item::Type FileItem::type() const
{
  return FILE;
}

//----------------------------------------------------------------------------
std::size_t FileItem::numberOfRequiredValues() const
{
  const FileItemDefinition *def =
    static_cast<const FileItemDefinition*>(this->m_definition.get());
  if (def == NULL)
    {
    return 0;
    }
  return def->numberOfRequiredValues();
}
//----------------------------------------------------------------------------
bool FileItem::shouldBeRelative() const
{
  const FileItemDefinition *def =
    static_cast<const FileItemDefinition *>(this->definition().get());
  if (def != NULL)
    {
    return def->shouldBeRelative();
    }
  return true;
}
//----------------------------------------------------------------------------
bool FileItem::shouldExist() const
{
  const FileItemDefinition *def =
    static_cast<const FileItemDefinition *>(this->definition().get());
  if (def != NULL)
    {
    return def->shouldExist();
    }
  return true;
}
//----------------------------------------------------------------------------
bool FileItem::setValue(std::size_t element, const std::string &val)
{
  const FileItemDefinition *def =
    static_cast<const FileItemDefinition *>(this->definition().get());
  if ((def == NULL) || (def->isValueValid(val)))
    {
    this->m_values[element] = val;
    this->m_isSet[element] = true;
    if(std::find(this->m_recentValues.begin(), this->m_recentValues.end(), val)
       == this->m_recentValues.end())
      this->m_recentValues.push_back(val);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
std::string
FileItem::valueAsString(std::size_t element,
                        const std::string &format) const
{
  // For the initial design we will use sprintf and force a limit of 300 char
  char dummy[300];
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
//----------------------------------------------------------------------------
bool
FileItem::appendValue(const std::string &val)
{
  //First - are we allowed to change the number of values?
  const FileItemDefinition *def =
    static_cast<const FileItemDefinition *>(this->definition().get());
  if (def->numberOfRequiredValues() != 0)
    {
    return false; // The number of values is fixed
    }

  if (def->isValueValid(val))
    {
    this->m_values.push_back(val);
    this->m_isSet.push_back(true);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool
FileItem::removeValue(std::size_t element)
{
  //First - are we allowed to change the number of values?
  const FileItemDefinition *def =
    static_cast<const FileItemDefinition *>(this->definition().get());
  if ( def->numberOfRequiredValues() != 0 )
    {
    return false; // The number of values is fixed
    }
  this->m_values.erase(this->m_values.begin()+element);
  this->m_isSet.erase(this->m_isSet.begin()+element);
  return true;
}
//----------------------------------------------------------------------------
bool FileItem::setNumberOfValues(std::size_t newSize)
{
  // If the current size is the same just return
  if (this->numberOfValues() == newSize)
    {
    return true;
    }

  //Next - are we allowed to change the number of values?
  const FileItemDefinition *def =
    static_cast<const FileItemDefinition *>(this->definition().get());
  std::size_t n = def->numberOfRequiredValues();
  if (n != 0)
    {
    return false; // The number of values is fixed
    }
  this->m_values.resize(newSize);
  this->m_isSet.resize(newSize, false); //Any added values are not set
  return true;
}
//----------------------------------------------------------------------------
void
FileItem::reset()
{
  const FileItemDefinition *def
    = static_cast<const FileItemDefinition *>(this->definition().get());
  // Was the initial size 0?
  std::size_t i, n = def->numberOfRequiredValues();
  if (n == 0)
    {
    this->m_values.clear();
    this->m_isSet.clear();
    return;
    }
  for (i = 0; i < n; i++)
    {
    this->unset(i);
    }
}
//----------------------------------------------------------------------------
void FileItem::copyFrom(ItemPtr sourceItem, CopyInfo& info)
{
  // Assigns my contents to be same as sourceItem
  Item::copyFrom(sourceItem, info);

  FileItemPtr sourceFileItem =
    smtk::dynamic_pointer_cast<FileItem>(sourceItem);
  // copy all recentValues list
  this->m_recentValues.clear();
  this->m_recentValues.insert(m_recentValues.end(),
                              sourceFileItem->recentValues().begin(),
                              sourceFileItem->recentValues().end());

  for (std::size_t i=0; i<sourceFileItem->numberOfValues(); ++i)
    {
    if (sourceFileItem->isSet(i))
      {
      this->setValue(i, sourceFileItem->value(i));
      }
    else
      {
      this->unset(i);
      }
    }
}

//----------------------------------------------------------------------------
void FileItem::addRecentValue(const std::string& val)
{ 
  if(std::find(this->m_recentValues.begin(), this->m_recentValues.end(), val)
     == this->m_recentValues.end())
    this->m_recentValues.push_back(val);
}

//----------------------------------------------------------------------------
