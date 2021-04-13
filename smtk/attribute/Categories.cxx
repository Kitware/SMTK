//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Categories.h"

#include <iostream>

using namespace smtk::attribute;

std::string Categories::Set::combinationModeAsString(const Set::CombinationMode mode)
{
  if (mode == Set::CombinationMode::All)
  {
    return "All";
  }
  return "Any";
}

bool Categories::Set::combinationModeFromString(const std::string& val, Set::CombinationMode& mode)
{
  if (val == "All")
  {
    mode = Set::CombinationMode::All;
    return true;
  }
  if (val == "Any")
  {
    mode = Set::CombinationMode::Any;
    return true;
  }
  return false;
}
bool Categories::Set::passes(const std::string& category) const
{
  std::set<std::string> categories;
  categories.insert(category);
  return this->passes(categories);
}

bool Categories::Set::passes(const std::set<std::string>& categories) const
{
  bool result;
  if (m_combinationMode == Set::CombinationMode::All)
  {
    result = passesCheck(categories, m_includedCategories, m_includeMode) &&
      !passesCheck(categories, m_excludedCategories, m_excludeMode);
  }
  else
  {
    result = passesCheck(categories, m_includedCategories, m_includeMode) ||
      !passesCheck(categories, m_excludedCategories, m_excludeMode);
  }

  return result;
}

bool Categories::Set::passesCheck(
  const std::set<std::string>& categories,
  const std::set<std::string>& testSet,
  Set::CombinationMode comboMode)
{
  // If there are no values which means there are no categories
  // associated then fail
  if (testSet.empty())
  {
    return false;
  }

  if (comboMode == Set::CombinationMode::Any)
  {
    for (const auto& cat : testSet)
    {
      if (categories.find(cat) != categories.end())
      {
        return true;
      }
    }
    return false;
  }
  // Ok we are doing an All check
  for (const auto& cat : testSet)
  {
    if (categories.find(cat) == categories.end())
    {
      return false;
    }
  }
  return true;
}

bool Categories::Set::operator<(const Set& rhs) const
{
  if (m_combinationMode != rhs.m_combinationMode)
  {
    return m_combinationMode < rhs.m_combinationMode;
  }
  if (m_includeMode != rhs.m_includeMode)
  {
    return m_includeMode < rhs.m_includeMode;
  }
  if (m_excludeMode != rhs.m_excludeMode)
  {
    return m_excludeMode < rhs.m_excludeMode;
  }
  if (m_includedCategories != rhs.m_includedCategories)
  {
    return m_includedCategories < rhs.m_includedCategories;
  }
  return m_excludedCategories < rhs.m_excludedCategories;
}

void Categories::insert(const Set& set)
{
  // if the set is not empty, add it
  if (!set.empty())
  {
    m_sets.insert(set);
  }
}

void Categories::insert(const Categories& cats)
{
  for (const auto& set : cats.m_sets)
  {
    this->insert(set);
  }
}

bool Categories::passes(const std::string& category) const
{
  // If there are no sets which means there are no categories
  // associated then fail
  if (m_sets.empty())
  {
    return false;
  }

  for (const auto& set : m_sets)
  {
    if (set.passes(category))
    {
      return true;
    }
  }
  return false;
}

bool Categories::passes(const std::set<std::string>& categories) const
{
  // If there are no sets which means there are no categories
  // associated then fail
  if (m_sets.empty())
  {
    return false;
  }

  for (const auto& set : m_sets)
  {
    if (set.passes(categories))
    {
      return true;
    }
  }
  return false;
}

std::set<std::string> Categories::categoryNames() const
{
  std::set<std::string> result;
  for (const auto& set : m_sets)
  {
    result.insert(set.includedCategoryNames().begin(), set.includedCategoryNames().end());
  }
  return result;
}

void Categories::print() const
{
  std::cerr << "{";
  for (const auto& set : m_sets)
  {
    std::cerr << " { " << Set::combinationModeAsString(set.combinationMode())
              << " Inclusion:" << Set::combinationModeAsString(set.inclusionMode()) << "{";
    for (const auto& c : set.includedCategoryNames())
    {
      std::cerr << "\"" << c << "\"";
    }
    std::cerr << "}";

    std::cerr << " Exclusion:" << Set::combinationModeAsString(set.exclusionMode()) << "{";
    for (const auto& c : set.excludedCategoryNames())
    {
      std::cerr << "\"" << c << "\"";
    }
    std::cerr << "}}";
  }
  std::cerr << "}\n";
}
