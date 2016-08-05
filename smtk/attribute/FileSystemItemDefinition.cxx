//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/FileSystemItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileSystemItem.h"

using namespace smtk::attribute;

//----------------------------------------------------------------------------
FileSystemItemDefinition::
FileSystemItemDefinition(const std::string &myName):
  ItemDefinition(myName), m_shouldExist(false), m_shouldBeRelative(false),
  m_useCommonLabel(false), m_numberOfRequiredValues(1), m_hasDefault(false),
  m_isExtensible(false), m_maxNumberOfValues(0)
{
}

//----------------------------------------------------------------------------
FileSystemItemDefinition::~FileSystemItemDefinition()
{
}
//----------------------------------------------------------------------------
Item::Type FileSystemItemDefinition::type() const
{
  return Item::DIRECTORY;
}

//----------------------------------------------------------------------------
bool
FileSystemItemDefinition::isValueValid(const std::string &/*val*/) const
{
  return true;
}
//----------------------------------------------------------------------------
void FileSystemItemDefinition::setIsExtensible(bool mode)
{
  this->m_isExtensible = mode;
  if (mode && !this->usingCommonLabel())
    {
    // Need to clear individual labels - can only use common label with
    // extensible groups
    this->setCommonValueLabel("");
    }
}
//----------------------------------------------------------------------------
bool FileSystemItemDefinition::setNumberOfRequiredValues(std::size_t esize)
{
  if (esize == this->m_numberOfRequiredValues)
    {
    return true;
    }
  std::size_t maxN = this->maxNumberOfValues();
  if (maxN && (esize > maxN))
    {
    return false;
    }

  this->m_numberOfRequiredValues = esize;
  if (!this->hasValueLabels())
    {
    return true;
    }
  if (!(this->m_useCommonLabel || this->m_isExtensible))
    {
    this->m_valueLabels.resize(esize);
    }
  return true;
}
//----------------------------------------------------------------------------
void FileSystemItemDefinition::setValueLabel(std::size_t element, const std::string &elabel)
{
  if (this->m_numberOfRequiredValues == 0)
    {
    return;
    }
  if (this->m_valueLabels.size() != this->m_numberOfRequiredValues)
    {
    this->m_valueLabels.resize(this->m_numberOfRequiredValues);
    }
  this->m_useCommonLabel = false;
  this->m_valueLabels[element] = elabel;
}
//----------------------------------------------------------------------------
void FileSystemItemDefinition::setCommonValueLabel(const std::string &elabel)
{
  if (this->m_valueLabels.size() != 1)
    {
    this->m_valueLabels.resize(1);
    }
  this->m_useCommonLabel = true;
  this->m_valueLabels[0] = elabel;
}

//----------------------------------------------------------------------------
std::string FileSystemItemDefinition::valueLabel(std::size_t element) const
{
  if (this->m_useCommonLabel)
    {
    return this->m_valueLabels[0];
    }
  if (this->m_valueLabels.size())
    {
    return this->m_valueLabels[element];
    }
  return ""; // If we threw execeptions this method could return const string &
}
//----------------------------------------------------------------------------
bool  FileSystemItemDefinition::setMaxNumberOfValues(std::size_t esize)
{
  if (esize && (esize < this->m_numberOfRequiredValues))
    {
    return false;
    }
  this->m_maxNumberOfValues = esize;
  return true;
}
//----------------------------------------------------------------------------
void FileSystemItemDefinition::setDefaultValue(const std::string& val)
{
  this->m_defaultValue = val;
  this->m_hasDefault = true;
}
