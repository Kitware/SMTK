//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/CompilerInformation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"

#include "smtk/Regex.h"

#include <sstream>

using namespace smtk::attribute;

FileItemDefinition::FileItemDefinition(const std::string& myName)
  : FileSystemItemDefinition(myName)
{
}

FileItemDefinition::~FileItemDefinition() = default;

Item::Type FileItemDefinition::type() const
{
  return Item::FileType;
}

int FileItemDefinition::filterId(const std::string& val) const
{
  // Compare the value with the allowed suffixes
  regex re(";;");
  sregex_token_iterator it(getFileFilters().begin(), getFileFilters().end(), re, -1), last;
  for (int id = 0; it != last; ++it, ++id)
  {
    std::size_t begin = it->str().find_first_not_of(" \n\r\t*.", it->str().find_last_of('(') + 1);
    std::size_t end = it->str().find_last_not_of(" \n\r\t", it->str().find_last_of(')'));
    std::string suffixes = it->str().substr(begin, end - begin);

    // If the suffixes string is empty, we have a permissive filter (*.*). No
    // validity check is needed.
    if (suffixes.empty())
    {
      return id;
    }

    // If there is only one suffix, we perform a regular expression match with
    // it.
    if (!regex_search(suffixes, regex("\\s")))
    {
      if (regex_match(val, regex("^.*\\." + suffixes)))
      {
        return id;
      }
    }
    else
    {
      // There are multiple available suffixes, so we combine them into a single
      // regular expression.
      std::string condensedSuffixes = regex_replace(suffixes, regex("\\s+\\*\\."), "|");
      if (regex_match(val, regex("^.*\\.(" + condensedSuffixes + ")")))
      {
        return id;
      }
    }
  }

  return -1;
}

std::string FileItemDefinition::getSummarizedFileFilters() const
{
  std::string summary;
  if (m_fileFilters.empty())
  {
    return summary;
  }
  int filterCount;
  auto agg = FileItemDefinition::aggregateFileFilters(m_fileFilters, filterCount);
  if (filterCount > 1)
  {
    summary = "All supported types " + agg + ";;" + m_fileFilters;
  }
  else
  {
    summary = m_fileFilters;
  }
  return summary;
}

std::string FileItemDefinition::aggregateFileFilters(const std::string& filtersStr)
{
  int dummyCount;
  return FileItemDefinition::aggregateFileFilters(filtersStr, dummyCount);
}

std::string FileItemDefinition::aggregateFileFilters(const std::string& filtersStr, int& count)
{
  count = 0;
  // Condense multiple file filters into a single expression
  regex re(";;");
  sregex_token_iterator it(filtersStr.begin(), filtersStr.end(), re, -1), last;
  std::set<std::string> filters;
  for (int id = 0; it != last; ++it, ++id)
  {
    std::size_t begin = it->str().find_first_not_of(" \n\r\t", it->str().find_last_of('(') + 1);
    std::size_t end = it->str().find_last_not_of(" \n\r\t", it->str().find_last_of(')'));
    std::string suffixes = it->str().substr(begin, end - begin);

    // If the suffixes string is empty, we have a permissive filter (*.*). All
    // entries are accepted.
    if (suffixes.empty())
    {
      return "(*.*)";
    }

    // Collect all of the suffixes using a set to guarantee uniqueness.
    // NOLINTNEXTLINE(readability-container-data-pointer): wrong `const`ness
    for (auto* i = std::strtok(&suffixes[0], " "); i != nullptr; i = std::strtok(nullptr, " "))
    {
      // If all entries are accepted, there is no need to aggregate.
      if (!strcmp(i, "*.*"))
      {
        return "(*.*)";
      }
      filters.insert(i);
    }
  }

  std::stringstream s;
  s << "(";
  bool first = true;
  for (const std::string& filter : filters)
  {
    if (first)
    {
      first = false;
    }
    else
    {
      s << " ";
    }
    s << filter;
    ++count;
  }
  s << ")";

  return s.str();
}

bool FileItemDefinition::isValueValid(const std::string& val) const
{
  // If the base class method's validity conditions are not satisfied, then the
  // value is not valid.
  if (!FileSystemItemDefinition::isValueValid(val))
  {
    return false;
  }

  // If file filters are provided, we check if the value has an acceptable
  // suffix.
  if (getFileFilters().empty())
  {
    return true;
  }

  return this->filterId(val) != -1;
}

smtk::attribute::ItemPtr FileItemDefinition::buildItem(Attribute* owningAttribute, int itemPosition)
  const
{
  return smtk::attribute::ItemPtr(new FileItem(owningAttribute, itemPosition));
}

smtk::attribute::ItemPtr
FileItemDefinition::buildItem(Item* owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new FileItem(owningItem, itemPosition, subGroupPosition));
}

smtk::attribute::ItemDefinitionPtr smtk::attribute::FileItemDefinition::createCopy(
  smtk::attribute::ItemDefinition::CopyInfo& info) const
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
    for (i = 0; i < m_valueLabels.size(); ++i)
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
