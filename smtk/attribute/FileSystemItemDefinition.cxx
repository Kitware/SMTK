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

#include <cassert>

using namespace smtk::attribute;

FileSystemItemDefinition::FileSystemItemDefinition(const std::string& myName)
  : ItemDefinition(myName)
{
}

FileSystemItemDefinition::~FileSystemItemDefinition() = default;

Item::Type FileSystemItemDefinition::type() const
{
  return Item::DirectoryType;
}

bool FileSystemItemDefinition::isValueValid(const std::string& val) const
{
  return !val.empty();
}

void FileSystemItemDefinition::setIsExtensible(bool mode)
{
  m_isExtensible = mode;
  if (mode && !this->usingCommonLabel())
  {
    // Need to clear individual labels - can only use common label with
    // extensible groups
    this->setCommonValueLabel("");
  }
}

bool FileSystemItemDefinition::setNumberOfRequiredValues(std::size_t esize)
{
  if (esize == m_numberOfRequiredValues)
  {
    return true;
  }
  std::size_t maxN = this->maxNumberOfValues();
  if (maxN && (esize > maxN))
  {
    return false;
  }

  m_numberOfRequiredValues = esize;
  if (!this->hasValueLabels())
  {
    return true;
  }
  if (!(m_useCommonLabel || m_isExtensible))
  {
    m_valueLabels.resize(esize);
  }
  return true;
}

void FileSystemItemDefinition::setValueLabel(std::size_t element, const std::string& elabel)
{
  if (m_numberOfRequiredValues == 0)
  {
    return;
  }
  if (m_valueLabels.size() != m_numberOfRequiredValues)
  {
    m_valueLabels.resize(m_numberOfRequiredValues);
  }
  m_useCommonLabel = false;
  assert(m_valueLabels.size() > element);
  m_valueLabels[element] = elabel;
}

void FileSystemItemDefinition::setCommonValueLabel(const std::string& elabel)
{
  if (m_valueLabels.size() != 1)
  {
    m_valueLabels.resize(1);
  }
  m_useCommonLabel = true;
  m_valueLabels[0] = elabel;
}

std::string FileSystemItemDefinition::valueLabel(std::size_t element) const
{
  if (m_useCommonLabel)
  {
    assert(m_valueLabels.size() > 0);
    return m_valueLabels[0];
  }
  if (!m_valueLabels.empty())
  {
    assert(m_valueLabels.size() > element);
    return m_valueLabels[element];
  }
  return ""; // If we threw execeptions this method could return const string &
}

bool FileSystemItemDefinition::setMaxNumberOfValues(std::size_t esize)
{
  if (esize && (esize < m_numberOfRequiredValues))
  {
    return false;
  }
  m_maxNumberOfValues = esize;
  return true;
}

void FileSystemItemDefinition::setDefaultValue(const std::string& val)
{
  m_defaultValue = val;
  m_hasDefault = true;
}
