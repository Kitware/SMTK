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

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace smtk::attribute;

bool Categories::Set::setCombinationMode(const Set::CombinationMode& newMode)
{
  if (newMode != Set::CombinationMode::LocalOnly)
  {
    m_combinationMode = newMode;
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
  if (m_combinationMode == CombinationMode::And)
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

  if (comboMode == CombinationMode::Or)
  {
    return std::any_of(testSet.begin(), testSet.end(), [&categories](const std::string& cat) {
      return categories.find(cat) != categories.end();
    });
  }
  // Ok we are doing an And check
  return std::all_of(testSet.begin(), testSet.end(), [&categories](const std::string& cat) {
    return categories.find(cat) != categories.end();
  });
}

int Categories::Set::compare(const Set& rhs) const
{
  if (m_combinationMode != rhs.m_combinationMode)
  {
    return (m_combinationMode < rhs.m_combinationMode) ? -1 : 1;
  }
  if (m_includeMode != rhs.m_includeMode)
  {
    return (m_includeMode < rhs.m_includeMode) ? -1 : 1;
  }
  if (m_excludeMode != rhs.m_excludeMode)
  {
    return (m_excludeMode < rhs.m_excludeMode) ? -1 : 1;
    ;
  }
  if (m_includedCategories != rhs.m_includedCategories)
  {
    return (m_includedCategories < rhs.m_includedCategories) ? -1 : 1;
  }
  if (m_excludedCategories != rhs.m_excludedCategories)
  {
    return (m_excludedCategories < rhs.m_excludedCategories) ? -1 : 1;
  }
  return 0; // they are identical
}

std::string Categories::Set::convertToString(const std::string& prefix) const
{
  std::stringstream ss;
  ss << prefix << "{";
  ss << Categories::combinationModeAsString(m_combinationMode)
     << " Inclusion:" << Categories::combinationModeAsString(m_includeMode) << "{";
  for (const auto& c : m_includedCategories)
  {
    ss << "\"" << c << "\"";
  }
  ss << "}";

  ss << " Exclusion:" << Categories::combinationModeAsString(m_excludeMode) << "{";
  for (const auto& c : m_excludedCategories)
  {
    ss << "\"" << c << "\"";
  }
  ss << "}}\n";
  return ss.str();
}

bool Categories::Stack::append(CombinationMode mode, const Set& categorySet)
{
  // if the mode is LocalOnly - clear the stack
  if (mode == CombinationMode::LocalOnly)
  {
    m_stack.clear();
  }

  // check to see if the category set represents nothing - if it is don't add it
  if (categorySet.empty())
  {
    // If the mode was not LocalOnly then return false since this result in a nop
    return (mode == CombinationMode::LocalOnly);
  }

  // If the stack is empty then the mode is always LocalOnly since it marks the end
  // of the stack for testing categories
  if (m_stack.empty())
  {
    std::pair<CombinationMode, Set> p(CombinationMode::LocalOnly, categorySet);
    m_stack.push_back(p);
    return true;
  }
  // Else append the mode/categorySet
  std::pair<CombinationMode, Set> newPair(mode, categorySet);
  m_stack.push_back(newPair);
  return true;
}

bool Categories::Stack::passes(const std::string& category) const
{
  std::set<std::string> categories;
  categories.insert(category);
  return this->passes(categories);
}

bool Categories::Stack::passes(const std::set<std::string>& cats) const
{
  bool lastResult = false;
  for (auto it = m_stack.crbegin(); it != m_stack.crend(); ++it)
  {
    lastResult = it->second.passes(cats);
    if (lastResult)
    {
      if ((it->first == CombinationMode::Or) || (it->first == CombinationMode::LocalOnly))
      {
        return true;
      }
    }
    else if ((it->first == CombinationMode::And) || (it->first == CombinationMode::LocalOnly))
    {
      return false;
    }
  }
  return lastResult;
}

std::string Categories::Stack::convertToString(const std::string& prefix) const
{
  std::stringstream ss;
  for (auto it = m_stack.cbegin(); it != m_stack.cend(); it++)
  {
    ss << prefix << Categories::combinationModeAsString(it->first) << "\n";
    ss << prefix << it->second.convertToString();
  }
  return ss.str();
}

std::set<std::string> Categories::Stack::categoryNames() const
{
  std::set<std::string> result;
  for (auto it = m_stack.cbegin(); it != m_stack.cend(); it++)
  {
    result.insert(
      it->second.includedCategoryNames().begin(), it->second.includedCategoryNames().end());
    result.insert(
      it->second.excludedCategoryNames().begin(), it->second.excludedCategoryNames().end());
  }
  return result;
}

bool Categories::Stack::operator<(const Stack& rhs) const
{
  if (m_stack.size() != rhs.m_stack.size())
  {
    return m_stack.size() < rhs.m_stack.size();
  }
  auto rit = rhs.m_stack.crbegin();
  for (auto it = m_stack.crbegin(); it != m_stack.crend(); it++, rit++)
  {
    if (it->first != rit->first)
    {
      return it->first < rit->first;
    }
    int result = it->second.compare(rit->second);
    if (result != 0)
    {
      return result < 0;
    }
  }
  return false; // They are the same
}

std::string Categories::combinationModeAsString(const CombinationMode mode)
{
  if (mode == CombinationMode::And)
  {
    return "And";
  }
  if (mode == CombinationMode::Or)
  {
    return "Or";
  }
  return "LocalOnly";
}

bool Categories::combinationModeFromString(const std::string& val, CombinationMode& mode)
{
  if ((val == "And") || (val == "All"))
  {
    mode = CombinationMode::And;
    return true;
  }
  if ((val == "Or") || (val == "Any"))
  {
    mode = CombinationMode::Or;
    return true;
  }
  if (val == "LocalOnly")
  {
    mode = CombinationMode::LocalOnly;
    return true;
  }
  return false;
}

bool Categories::insert(const Stack& stack)
{
  // if the stack is not empty, add it
  if (!stack.empty())
  {
    m_stacks.insert(stack);
    return true;
  }
  return false;
}

void Categories::insert(const Categories& cats)
{
  for (const auto& stack : cats.m_stacks)
  {
    this->insert(stack);
  }
}

bool Categories::passes(const std::string& category) const
{
  // If there are no stacks which means there are no categories
  // associated then fail
  if (m_stacks.empty())
  {
    return false;
  }

  return std::any_of(m_stacks.begin(), m_stacks.end(), [&category](const Stack& stack) {
    return stack.passes(category);
  });
}

bool Categories::passes(const std::set<std::string>& categories) const
{
  // If there are no stacks which means there are no categories
  // associated then fail
  if (m_stacks.empty())
  {
    return false;
  }

  return std::any_of(m_stacks.begin(), m_stacks.end(), [&categories](const Stack& stack) {
    return stack.passes(categories);
  });
}

std::set<std::string> Categories::categoryNames() const
{
  std::set<std::string> result;
  for (const auto& stack : m_stacks)
  {
    std::set<std::string> sinfo = stack.categoryNames();
    result.insert(sinfo.begin(), sinfo.end());
  }
  return result;
}

std::string Categories::convertToString() const
{
  std::stringstream ss;
  ss << "{";
  for (const auto& stack : m_stacks)
  {
    ss << " { " << stack.convertToString("\t") << "}\n";
  }
  ss << "}\n";
  return ss.str();
}

void Categories::print() const
{
  std::cerr << this->convertToString();
}
