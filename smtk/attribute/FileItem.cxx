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
bool FileItem::isValid() const
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
std::size_t FileItem::maxNumberOfValues() const
{
  const FileItemDefinition *def =
    static_cast<const FileItemDefinition*>(this->m_definition.get());
  if (def == NULL)
    {
    return 0;
    }
  return def->maxNumberOfValues();
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
bool FileItem::isExtensible() const
{
  auto def =
    static_cast<const FileItemDefinition*>(this->m_definition.get());
  if (!def)
    {
    return false;
    }
  return def->isExtensible();
}
//----------------------------------------------------------------------------
bool
FileItem::appendValue(const std::string &val)
{
  //First - are we allowed to change the number of values?
  if (!this->isExtensible())
    {
    return false; // The number of values is fixed
    }
  // See if we have it max number of values (if there is such a limit)
  std::size_t maxN = this->maxNumberOfValues();
  if ( maxN && (this->numberOfValues() >= maxN))
    {
    return false;
    }
  auto def =
    static_cast<const FileItemDefinition*>(this->m_definition.get());
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
 if (!this->isExtensible())
    {
    return false; // The number of values is fixed
    }
  // Would removing teh value take us under the number of required values?
  if (this->numberOfValues()  <= this->numberOfRequiredValues())
    {
    return false;
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
//----------------------------------------------------------------------------
bool FileItem::hasDefault() const
{
  auto def = static_cast<const FileItemDefinition *>(this->definition().get());
  if (!def)
    {
    return false;
    }
  return def->hasDefault();
}
//----------------------------------------------------------------------------
bool FileItem::setToDefault(std::size_t element)
{
  auto def = static_cast<const FileItemDefinition *>(this->definition().get());
  if (!def->hasDefault())
    {
    return false; // Doesn't have a default value
    }

  this->setValue(element, def->defaultValue());
  return true;
}
//----------------------------------------------------------------------------
bool FileItem::isUsingDefault() const
{
  auto def = static_cast<const FileItemDefinition *>(this->definition().get());
  if (!def->hasDefault())
    {
    return false; // Doesn't have a default value
    }

  std::size_t i, n = this->numberOfValues();
  std::string dval = def->defaultValue();
  for (i = 0; i < n; i++)
    {
    if (!(this->m_isSet[i] && (this->m_values[i] == dval)))
      {
      return false;
       }
    }
  return true;
}
//----------------------------------------------------------------------------
 bool FileItem::isUsingDefault(std::size_t element) const
{
  auto def = static_cast<const FileItemDefinition *>(this->definition().get());
  if (def->hasDefault() && this->m_isSet[element] && 
      this->m_values[element] == def->defaultValue())
    {
    return true; 
    }
  return false;
}
//----------------------------------------------------------------------------
std::string FileItem::defaultValue() const
{
  auto def = static_cast<const FileItemDefinition *>(this->definition().get());
  if (!def)
    {
      return "";
    }
  return def->defaultValue();
}
//----------------------------------------------------------------------------
void
FileItem::reset()
{
  auto def = static_cast<const FileItemDefinition *>(this->definition().get());
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
//----------------------------------------------------------------------------
bool FileItem::assign(ConstItemPtr &sourceItem, unsigned int options)
{
  // Assigns my contents to be same as sourceItem
  smtk::shared_ptr<const FileItem > sourceFileItem =
    smtk::dynamic_pointer_cast<const FileItem>(sourceItem);
  
  if (!sourceFileItem)
    {
    return false; // Source is not a file item
    }
  
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
  
  return Item::assign(sourceItem, options);
}

//----------------------------------------------------------------------------
void FileItem::addRecentValue(const std::string& val)
{ 
  if(std::find(this->m_recentValues.begin(), this->m_recentValues.end(), val)
     == this->m_recentValues.end())
    this->m_recentValues.push_back(val);
}

//----------------------------------------------------------------------------
