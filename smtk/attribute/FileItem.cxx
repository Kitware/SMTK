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


#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/Attribute.h"
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
FileItem::FileItem(Item *owningItem,
                   int itemPosition,
                   int subGroupPosition): 
  Item(owningItem, itemPosition, subGroupPosition)
{
}

//----------------------------------------------------------------------------
bool FileItem::
setDefinition(smtk::ConstAttributeItemDefinitionPtr adef)
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
  int n = def->numberOfRequiredValues();
  if (n)
    {
    this->m_isSet.resize(n, false);
    this->m_values.resize(n);
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
int FileItem::numberOfRequiredValues() const
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
bool FileItem::setValue(int element, const std::string &val)
{
  const FileItemDefinition *def = 
    static_cast<const FileItemDefinition *>(this->definition().get());
  if ((def == NULL) || (def->isValueValid(val)))
    {
    this->m_values[element] = val;
    this->m_isSet[element] = true;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
std::string
FileItem::valueAsString(int element, 
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
  int n = def->numberOfRequiredValues();
  if (n)
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
FileItem::removeValue(int element)
{
  //First - are we allowed to change the number of values?
  const FileItemDefinition *def =
    static_cast<const FileItemDefinition *>(this->definition().get());
  int n = def->numberOfRequiredValues();
  if (n)
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
  if (n)
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
  int i, n = def->numberOfRequiredValues();
  if (!n)
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
