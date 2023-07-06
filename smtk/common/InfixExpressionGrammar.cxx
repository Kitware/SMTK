//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/InfixExpressionGrammar.h"

#include <cmath>

#include "smtk/common/InfixExpressionGrammarImpl.h"

using namespace tao::pegtl;

namespace smtk
{
namespace common
{

InfixExpressionGrammar::InfixExpressionGrammar()
  : m_subsymbolVisitor([](const std::string&) { return std::make_pair(std::nan(""), false); })
{
}

void InfixExpressionGrammar::addFunction(
  const std::string& name,
  const std::function<double(double)>& f)
{
  m_functions.insert(name, f);
}

const std::map<std::string, InfixFunction>& InfixExpressionGrammar::functions() const
{
  return m_functions.funcs();
}

void InfixExpressionGrammar::setSubsymbolVisitor(const SubsymbolVisitor& f)
{
  m_subsymbolVisitor = f;
}

SubsymbolVisitor InfixExpressionGrammar::subsymbolVisitor() const
{
  return m_subsymbolVisitor;
}

double InfixExpressionGrammar::evaluate(const std::string& expression, InfixExpressionError& err)
  const
{
  // Freshen up |err|.
  err = InfixExpressionError::ERROR_NONE;

  InfixOperators ops;
  EvaluationStacks s;

  err = testExpressionSyntax(expression, ops, s);

  if (err != InfixExpressionError::ERROR_NONE)
  {
    return std::nan("");
  }

  double result = s.finish();
  // It's important to note that we can check global |errno| to learn of a more
  // specific math-related error code (EDOM or ERANGE). ONLY AS LONG AS NOTHING
  // ELSE SETS |errno| BETWEEN THEN AND NOW.
  if (std::isnan(result) || std::isinf(result))
  {
    err = InfixExpressionError::ERROR_MATH_ERROR;
  }

  return result;
}

InfixExpressionError InfixExpressionGrammar::testExpressionSyntax(
  const std::string& expression) const
{
  InfixOperators ops;
  EvaluationStacks s;

  return testExpressionSyntax(expression, ops, s);
}

InfixExpressionError InfixExpressionGrammar::testExpressionSyntax(
  const std::string& expression,
  InfixOperators& ops,
  EvaluationStacks& stacks) const
{
  InfixExpressionError err = InfixExpressionError::ERROR_NONE;

  tao::pegtl::string_input<> in(expression, "ExpressionParser");

  try
  {
    tao::pegtl::
      parse<expression_internal::expression_grammar, expression_internal::ExpressionAction>(
        in, ops, stacks, m_functions, m_subsymbolVisitor, err);
  }
  catch (tao::pegtl::parse_error& /*parse_err*/)
  {
    //    const auto p = parse_err.positions.front();
    //    std::cout << "ExpressionParser: " << parse_err.what() << "\n"
    //              << in.line_at(p) << "\n"
    //              << std::string(p.byte_in_line, ' ') << "^\n"
    //              << std::endl;

    // We caught a parse_error and |err| was not set by any ExpressionAction,
    // making this a syntax error.
    if (err == InfixExpressionError::ERROR_NONE)
    {
      err = InfixExpressionError::ERROR_INVALID_SYNTAX;
    }
  }

  return err;
}

} // namespace common
} // namespace smtk
