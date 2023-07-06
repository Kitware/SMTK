//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
/*!\file UnitTestInfixExpressionGrammarImpl.cxx - Unit tests for infix expression parsing
         and semantic actions. */

#include "smtk/common/InfixExpressionError.h"
#include "smtk/common/InfixExpressionEvaluation.h"
#include "smtk/common/InfixExpressionGrammarImpl.h"

#include "smtk/io/Logger.h"

#include <cmath>
#include <iostream>
#include <string>

#include "smtk/common/testing/cxx/helpers.h"

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>

int tryParse(const std::string& expression)
{
  tao::pegtl::string_input<> in(expression, "ExpressionParser");

  smtk::common::InfixOperators ops;
  smtk::common::EvaluationStacks s;
  smtk::common::InfixFunctions funcs;
  smtk::common::InfixExpressionError infixErr = smtk::common::InfixExpressionError::ERROR_NONE;
  smtk::common::SubsymbolVisitor subsymbolFunc = [](const std::string&) {
    return std::make_pair(0.0, true);
  };

  try
  {
    tao::pegtl::parse<
      smtk::common::expression_internal::expression_grammar,
      smtk::common::expression_internal::ExpressionAction>(
      in, ops, s, funcs, subsymbolFunc, infixErr);
  }
  catch (tao::pegtl::parse_error& err)
  {
    const auto p = err.positions.front();
    std::cout << "ExpressionParser: " << err.what() << "\n"
#if TAO_PEGTL_VERSION_MAJOR <= 2 && TAO_PEGTL_VERSION_MINOR <= 7
              << in.line_as_string(p) << "\n"
#else
              << in.line_at(p) << "\n"
#endif
              << std::string(p.byte_in_line, ' ') << "^\n"
              << std::endl;
    return 1;
  }

  return 0;
}

double evaluateExpression(const std::string& expression)
{
  tao::pegtl::string_input<> in(expression, "ExpressionParser");

  smtk::common::InfixOperators ops;
  smtk::common::EvaluationStacks s;
  smtk::common::InfixFunctions funcs;
  smtk::common::InfixExpressionError infixErr = smtk::common::InfixExpressionError::ERROR_NONE;
  smtk::common::SubsymbolVisitor subsymbolFunc = [](const std::string&) {
    return std::make_pair(0.0, true);
  };

  try
  {
    tao::pegtl::parse<
      smtk::common::expression_internal::expression_grammar,
      smtk::common::expression_internal::ExpressionAction>(
      in, ops, s, funcs, subsymbolFunc, infixErr);
  }
  catch (tao::pegtl::parse_error& err)
  {
    const auto p = err.positions.front();
    std::cout << "ExpressionParser: " << err.what() << "\n"
#if TAO_PEGTL_VERSION_MAJOR <= 2 && TAO_PEGTL_VERSION_MINOR <= 7
              << in.line_as_string(p) << "\n"
#else
              << in.line_at(p) << "\n"
#endif
              << std::string(p.byte_in_line, ' ') << "^\n"
              << std::endl;
    return std::nan("");
  }

  return s.finish();
}

int UnitTestInfixExpressionGrammarImpl(int, char** const)
{
  if (tao::pegtl::analyze<smtk::common::expression_internal::expression_grammar>() != 0)
  {
    return 1;
  }

  if (tryParse("5") == 1)
  {
    return 1;
  }

  if (tryParse("   2   ") == 1)
  {
    return 1;
  }

  if (tryParse("cos(5)") == 1)
  {
    return 1;
  }

  if (tryParse("cos    (5)") == 1)
  {
    return 1;
  }

  if (tryParse("cos  (   5  )") == 1)
  {
    return 1;
  }

  if (tryParse("5 + 5") == 1)
  {
    return 1;
  }

  if (evaluateExpression("5 + 5") != 10.0)
  {
    return 1;
  }

  if (tryParse("5 ^ 3") == 1)
  {
    return 1;
  }

  if (evaluateExpression("5 ^ 3") != 125.0)
  {
    return 1;
  }

  if (tryParse("sin(0 * 0)") == 1)
  {
    return 1;
  }

  if (evaluateExpression("sin(0 * 0)") != 0.0)
  {
    return 1;
  }

  if (tryParse("{foo}") == 1)
  {
    return 1;
  }

  if (tryParse("ln(12 + sin(1)) + {bar}") == 1)
  {
    return 1;
  }

  if (evaluateExpression("{foo}") != 0.0)
  {
    return 1;
  }

  if (tryParse("{foo} + 12") == 1)
  {
    return 1;
  }

  if (evaluateExpression("{foo} + 12") != 12.0)
  {
    return 1;
  }

  if (evaluateExpression("{a} + {b}") != 0.0)
  {
    return 1;
  }

  if (tryParse("0.01") == 1)
  {
    return 1;
  }

  if (tryParse("-0.01") == 1)
  {
    return 1;
  }

  if (tryParse("1e-1") == 1)
  {
    return 1;
  }

  if (tryParse("-1e-1") == 1)
  {
    return 1;
  }

  if (tryParse("0.01e12") == 1)
  {
    return 1;
  }

  if (tryParse("0.5 - 2e-1") == 1)
  {
    return 1;
  }

  if (tryParse("1.") == 1)
  {
    return 1;
  }

  return 0;
}
