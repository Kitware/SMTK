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

#include <regex>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
FileItemDefinition::
FileItemDefinition(const std::string &myName): FileSystemItemDefinition(myName)
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
bool FileItemDefinition::isValueValid(const std::string &val) const
{
  // If the base class method's validity conditions are not satisfied, then the
  // value is not valid.
  if (FileSystemItemDefinition::isValueValid(val) == false)
    {
    return false;
    }

  // If file filters are provided, we check if the value has an acceptible
  // suffix.
  if (getFileFilters().empty())
    {
    return true;
    }

  // Compare it with the allowed suffixes
  std::regex re(";;");
  std::sregex_token_iterator it(getFileFilters().begin(),
                                getFileFilters().end(),
                                re, -1), last;
  for (; it != last; ++it)
    {
    std::size_t begin =
      it->str().find_first_not_of(" \n\r\t*.", it->str().find_last_of("(") + 1);
    std::size_t end = it->str().find_last_not_of(" \n\r\t",
                                                 it->str().find_last_of(")"));
    std::string suffixes = it->str().substr(begin, end - begin);

    // If the suffixes string is empty, we have a permissive filter (*.*). No
    // validity check is needed.
    if (suffixes.empty())
      {
      return true;
      }

    // If there is only one suffix, we perform a regular expression match with
    // it.
    if (!std::regex_search(suffixes, std::regex("\\s")))
      {
        if (std::regex_match(val, std::regex("^.*\\." + suffixes)))
          {
          return true;
          }
      }
    else
      {
      // There are multiple available suffixes, so we combine them into a single
      // regular expression.
      std::string condensedSuffixes =
        std::regex_replace(suffixes, std::regex("\\s+\\*\\."), "|");
      if (std::regex_match(val,
                           std::regex("^.*\\.(" + condensedSuffixes + ")")))
        {
        return true;
        }
      }
    }

  return false;
}

//----------------------------------------------------------------------------
smtk::attribute::ItemPtr
FileItemDefinition::buildItem(Attribute *owningAttribute,
                                   int itemPosition) const
{
  return smtk::attribute::ItemPtr(new FileItem(owningAttribute, itemPosition));
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr
FileItemDefinition::buildItem(Item *owningItem,
                              int itemPosition,
                              int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new FileItem(owningItem, itemPosition,
                                               subGroupPosition));
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
