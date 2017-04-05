//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DirectoryItem.h"

using namespace smtk::attribute;

DirectoryItemDefinition::
DirectoryItemDefinition(const std::string &myName) :
  FileSystemItemDefinition(myName)
{
}

DirectoryItemDefinition::~DirectoryItemDefinition()
{
}
Item::Type DirectoryItemDefinition::type() const
{
  return Item::DIRECTORY;
}

smtk::attribute::ItemPtr
DirectoryItemDefinition::buildItem(Attribute *owningAttribute,
                                   int itemPosition) const
{
  return smtk::attribute::ItemPtr(new DirectoryItem(owningAttribute,
                                                    itemPosition));
}

smtk::attribute::ItemPtr
DirectoryItemDefinition::buildItem(Item *owningItem,
                                   int itemPosition,
                                   int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new DirectoryItem(owningItem, itemPosition,
                                                    subGroupPosition));
}

smtk::attribute::ItemDefinitionPtr
smtk::attribute::DirectoryItemDefinition::
createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  (void)info;
  smtk::attribute::DirectoryItemDefinitionPtr instance =
    smtk::attribute::DirectoryItemDefinition::New(this->name());
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
    for (std::size_t i=0; i<m_valueLabels.size(); ++i)
      {
      instance->setValueLabel(i, m_valueLabels[i]);
      }
    }

  instance->setShouldExist(m_shouldExist);
  instance->setShouldBeRelative(m_shouldBeRelative);

  if (m_hasDefault)
    {
    instance->setDefaultValue(m_defaultValue);
    }
 return instance;
}
