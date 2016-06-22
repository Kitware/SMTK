//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"

using namespace smtk::attribute;

//----------------------------------------------------------------------------
FileItemDefinition::
FileItemDefinition(const std::string &myName):
  ItemDefinition(myName), m_shouldExist(false), m_shouldBeRelative(false),
  m_useCommonLabel(false), m_numberOfRequiredValues(1), m_hasDefault(false),
  m_isExtensible(false), m_maxNumberOfValues(0)
{
}

//----------------------------------------------------------------------------
FileItemDefinition::~FileItemDefinition()
{
}
//----------------------------------------------------------------------------
Item::Type FileItemDefinition::type() const
{
  return Item::FILE;
}

//----------------------------------------------------------------------------
bool
FileItemDefinition::isValueValid(const std::string &/*val*/) const
{
  return true;
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr FileItemDefinition::buildItem(Attribute *owningAttribute,
                                      int itemPosition) const
{
  return smtk::attribute::ItemPtr(new FileItem(owningAttribute,
                                              itemPosition));
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr FileItemDefinition::buildItem(Item *owningItem,
                                                      int itemPosition,
                                                      int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new FileItem(owningItem,
                                              itemPosition,
                                              subGroupPosition));
}
//----------------------------------------------------------------------------
void FileItemDefinition::setIsExtensible(bool mode)
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
bool FileItemDefinition::setNumberOfRequiredValues(std::size_t esize)
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
void FileItemDefinition::setValueLabel(std::size_t element, const std::string &elabel)
{
  if (this->isExtensible())
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
void FileItemDefinition::setCommonValueLabel(const std::string &elabel)
{
  if (this->m_valueLabels.size() != 1)
    {
    this->m_valueLabels.resize(1);
    }
  this->m_useCommonLabel = true;
  this->m_valueLabels[0] = elabel;
}

//----------------------------------------------------------------------------
std::string FileItemDefinition::valueLabel(std::size_t element) const
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
bool  FileItemDefinition::setMaxNumberOfValues(std::size_t esize)
{
  if (esize && (esize < this->m_numberOfRequiredValues))
    {
    return false;
    }
  this->m_maxNumberOfValues = esize;
  return true;
}
//----------------------------------------------------------------------------
void FileItemDefinition::setDefaultValue(const std::string& val)
{
  this->m_defaultValue = val;
  this->m_hasDefault = true;
}
//----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr
smtk::attribute::FileItemDefinition::
createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  (void)info;

  std::size_t i;

  smtk::attribute::FileItemDefinitionPtr instance =
    smtk::attribute::FileItemDefinition::New(this->name());
  ItemDefinition::copyTo(instance);

  instance->setIsExtensible(m_isExtensible);
  instance->setNumberOfRequiredValues(m_numberOfRequiredValues);
  instance->setMaxNumberOfValues(m_maxNumberOfValues);

  // Add label(s)
  if (m_useCommonLabel)
    {
    instance->setCommonValueLabel(m_valueLabels[0]);
    }
  else if (this->hasValueLabels())
    {
    for (i=0; i<m_valueLabels.size(); ++i)
      {
      instance->setValueLabel(i, m_valueLabels[i]);
      }
    }

  instance->setShouldExist(m_shouldExist);
  instance->setShouldBeRelative(m_shouldBeRelative);
  instance->setFileFilters(m_fileFilters);

  if (m_hasDefault)
    {
    instance->setDefaultValue(m_defaultValue);
    }

  return instance;
}
//----------------------------------------------------------------------------
