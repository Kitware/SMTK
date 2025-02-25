//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/Categories.h"

#include "smtk/common/categories/Actions.h"
#include "smtk/common/categories/Evaluators.h"
#include "smtk/common/categories/Grammar.h"

#include "smtk/io/Logger.h"
#include <algorithm>
#include <iostream>
#include <sstream>

namespace
{
std::string assembleSubExpression(const std::set<std::string>& cats, const std::string& op)
{
  std::string exp;
  if (!cats.empty())
  {
    bool first = true;
    exp = "(";
    for (const std::string& cat : cats)
    {
      if (first)
      {
        exp.append("'").append(cat).append("'");
        first = false;
      }
      else
      {
        exp.append(" ").append(op).append(" '").append(cat).append("'");
      }
    }
    exp.append(")");
  }
  return exp;
}
} // namespace
using namespace smtk::common;

Categories::Expression::Expression()
{
  // By default set the expression to reject all
  this->setAllReject();
  m_isSet = false;
}

const std::string& Categories::Expression::expression() const
{
  return m_expression;
}

bool Categories::Expression::buildEvaluator()
{
  // If we don't have an expression string can we build one?
  if (m_expression.empty())
  {
    std::string includeSubExpression = assembleSubExpression(
      m_includedCategories, Categories::combinationModeAsSymbol(m_includeMode));

    std::string excludeSubExpression = assembleSubExpression(
      m_excludedCategories, Categories::combinationModeAsSymbol(m_excludeMode));

    if (includeSubExpression.empty())
    {
      if (m_combinationMode == Set::CombinationMode::And)
      {
        // Since there are no include categories and the combination mode is And
        // the result is that we reject all
        this->setAllReject();
        return true;
      }
      if (excludeSubExpression.empty())
      {
        // In this case we have an empty exclude set  and the combination mode is Or
        // the result is that we pass all
        this->setAllPass();
        return true;
      }
      // In this case the expression is just the complement of the exclusion sub-expression
      m_expression = "!";
      m_expression.append(excludeSubExpression);
    }
    else if (excludeSubExpression.empty())
    {
      if (m_combinationMode == Set::CombinationMode::Or)
      {
        // In this case we have an empty exclude set  and the combination mode is Or
        // the result is that we pass all
        this->setAllPass();
        return true;
      }
      // In this case the expression is just the inclusion sub-expression
      m_expression = includeSubExpression;
    }
    else
    {
      // Combine the 2 sub expressions
      m_expression = includeSubExpression;
      m_expression.append(" ")
        .append(Categories::combinationModeAsSymbol(m_combinationMode))
        .append(" !")
        .append(excludeSubExpression);
    }
  }
  // Ok we have a non-empty expression string to evaluate
  tao::pegtl::string_input<> in(m_expression, "(expression grammar)");
  smtk::common::categories::Evaluators evals;
  try
  {
    tao::pegtl::
      parse<smtk::common::categories::ExpressionGrammar, smtk::common::categories::Action>(
        in, evals);
  }
  catch (tao::pegtl::parse_error& err)
  {
    const auto p = err.positions.front();
#if TAO_PEGTL_VERSION_MAJOR <= 2 && TAO_PEGTL_VERSION_MINOR <= 7
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "smtk::common::Categories::Expression: " << err.what() << "\n"
                                               << in.line_as_string(p) << "\n"
                                               << std::string(p.byte_in_line, ' ') << "^\n");
#else
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "smtk::common::Categories::Expression: " << err.what() << "\n"
                                               << in.line_at(p) << "\n"
                                               << std::string(p.byte_in_line, ' ') << "^\n");
#endif
    return false;
  }

  if (evals.isValid())
  {
    m_evaluator = evals.top();
    m_categoryNames = evals.categoryNames();
    m_isSet = true;
    return true;
  }
  return false;
}

bool Categories::Expression::setExpression(const std::string& expString)
{
  // Save the old expression in case it needs to be restored.
  std::string originalExpression = m_expression;
  m_expression = expString;

  if (!this->buildEvaluator())
  {
    // There was a problem with the new expression - restore the original
    m_expression = originalExpression;
    return false;
  }
  // Clear the sets
  m_includedCategories = {};
  m_excludedCategories = {};
  return true;
}

void Categories::Expression::setAllPass()
{
  m_expression = "";

  // Clear the sets
  m_includedCategories = {};
  m_excludedCategories = {};

  m_allPass = true;
  m_isSet = true;
  m_evaluator = [this](const std::set<std::string>&) { return m_allPass; };
}

void Categories::Expression::setAllReject()
{
  m_expression = "";

  // Clear the sets
  m_includedCategories = {};
  m_excludedCategories = {};

  m_allPass = false;
  m_isSet = true;
  m_evaluator = [this](const std::set<std::string>&) { return m_allPass; };
}

bool Categories::Expression::passes(const std::set<std::string>& cats) const
{
  return m_evaluator(cats);
}

bool Categories::Expression::passes(const std::string& cat) const
{
  std::set<std::string> categories;
  categories.insert(cat);
  return this->passes(categories);
}

///\brief Compares with other set - returns -1 if this < rhs, 0 if they are equal, and 1 if this > rhs
int Categories::Expression::compare(const Set& rhs) const
{
  if (const auto* expr = dynamic_cast<const Expression*>(&rhs))
  {
    if (m_expression != expr->m_expression)
    {
      return (m_expression < expr->m_expression) ? -1 : 1;
    }
    if (m_allPass != expr->m_allPass)
    {
      return (m_allPass < expr->m_allPass) ? -1 : 1;
    }
  }
  return Categories::Set::compare(rhs);
}

void Categories::Expression::updatedSetInfo()
{
  // If both sets are empty there is nothing to update yet
  if (m_includedCategories.empty() && m_excludedCategories.empty())
  {
    return;
  }

  // Save the old expression in case it needs to be restored.
  std::string originalExpression = m_expression;
  m_expression = "";

  if (!this->buildEvaluator())
  {
    m_expression = originalExpression;
  }
}

std::string Categories::Expression::convertToString(const std::string& prefix) const
{
  std::stringstream ss;
  ss << prefix;
  if (m_isSet)
  {
    if (m_expression.empty())
    {
      if (m_allPass)
      {
        ss << "All_Pass";
      }
      else
      {
        ss << "All_Reject";
      }
    }
    else
    {
      ss << m_expression;
    }
  }
  else
  {
    ss << "Not_Set";
  }
  ss << std::endl;
  return ss.str();
}

bool Categories::Set::setCombinationMode(const Set::CombinationMode& newMode)
{
  if (newMode != Set::CombinationMode::LocalOnly)
  {
    m_combinationMode = newMode;
    this->updatedSetInfo();
    return true;
  }
  return false;
}

void Categories::Set::setInclusionMode(const Set::CombinationMode& newMode)
{
  if (m_includeMode == newMode)
  {
    return;
  }
  m_includeMode = newMode;
  this->updatedSetInfo();
}

void Categories::Set::setExclusionMode(const Set::CombinationMode& newMode)
{
  if (m_excludeMode == newMode)
  {
    return;
  }
  m_excludeMode = newMode;
  this->updatedSetInfo();
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

bool Categories::Stack::append(CombinationMode mode, const Expression& exp)
{
  // if the mode is LocalOnly - clear the stack
  if (mode == CombinationMode::LocalOnly)
  {
    m_stack.clear();
  }

  // check to see if the category expression has not been set - if so ignore it
  if (!exp.isSet())
  {
    // If the mode was not LocalOnly then return false since this result in a nop
    return (mode == CombinationMode::LocalOnly);
  }

  // If the stack is empty then the mode is always LocalOnly since it marks the end
  // of the stack for testing categories
  if (m_stack.empty())
  {
    std::pair<CombinationMode, Expression> p(CombinationMode::LocalOnly, exp);
    m_stack.push_back(p);
    return true;
  }
  // Else append the mode/categorySet
  std::pair<CombinationMode, Expression> newPair(mode, exp);
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
    ss << prefix << it->second.expression();
  }
  return ss.str();
}

std::set<std::string> Categories::Stack::categoryNames() const
{
  std::set<std::string> result;
  for (auto it = m_stack.cbegin(); it != m_stack.cend(); it++)
  {
    result.insert(it->second.categoryNames().begin(), it->second.categoryNames().end());
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

std::string Categories::combinationModeAsSymbol(const CombinationMode mode)
{
  if (mode == CombinationMode::And)
  {
    return "&";
  }
  if (mode == CombinationMode::Or)
  {
    return "|";
  }
  return "";
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

bool Categories::combinationModeStringToSymbol(const std::string& val, std::string& symbol)
{
  if ((val == "And") || (val == "All"))
  {
    symbol = "&";
    return true;
  }
  if ((val == "Or") || (val == "Any"))
  {
    symbol = "|";
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
