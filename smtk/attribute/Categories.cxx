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

bool Categories::Set::passes(const std::string& category) const
{
  // If there are no values which means there are no categories
  // associated then fail
  if (m_categoryNames.empty())
  {
    return false;
  }

  if (m_mode == Set::CombinationMode::All)
  {
    if (m_categoryNames.size() != 1)
    {
      // this set fails in that it has more than 1 category and they all needed to match
      return false;
    }
    // See if the single entry matches the input
    return (*(m_categoryNames.begin()) == category);
  }

  // Ok we have an Any check so see if teh category is in the set
  return (m_categoryNames.find(category) != m_categoryNames.end());
}

bool Categories::Set::passes(const std::set<std::string>& categories) const
{
  // If there are no values which means there are no categories
  // associated then fail
  if (m_categoryNames.empty())
  {
    return false;
  }

  if (m_mode == Set::CombinationMode::Any)
  {
    for (const auto& cat : m_categoryNames)
    {
      if (categories.find(cat) != categories.end())
      {
        return true;
      }
    }
    return false;
  }
  // Ok we are doing an All check
  for (const auto& cat : m_categoryNames)
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
  if (m_mode != rhs.m_mode)
  {
    return m_mode < rhs.m_mode;
  }
  return m_categoryNames < rhs.m_categoryNames;
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
    result.insert(set.categoryNames().begin(), set.categoryNames().end());
  }
  return result;
}

void Categories::print() const
{
  std::cerr << "{";
  for (const auto& set : m_sets)
  {
    std::cerr << " {";
    if (set.mode() == smtk::attribute::Categories::Set::CombinationMode::All)
    {
      std::cerr << "All,{";
    }
    else
    {
      std::cerr << "Any,{";
    }
    for (const auto& c : set.categoryNames())
    {
      std::cerr << "\"" << c << "\"";
    }
    std::cerr << "}}";
  }
  std::cerr << "}\n";
}
