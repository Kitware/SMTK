//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
/*!\file UnitTestInfixExpressionGrammar.cxx - Unit tests for InfixExpressionGrammar, which is
        the public interface for processing infix expressions. */

#include "smtk/common/InfixExpressionError.h"
#include "smtk/common/InfixExpressionGrammar.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>
#include <string>

void testSimpleExpression()
{
  smtk::common::InfixExpressionGrammar infix;
  smtk::common::InfixExpressionError err = smtk::common::InfixExpressionError::ERROR_NONE;
  smtkTest(infix.evaluate("1 + ln(1)", err) == 1.0, "Failed to evaluate 1 + ln(1).")
    smtkTest(err == smtk::common::InfixExpressionError::ERROR_NONE, "Expected err to be ERROR_NONE")
}

void testFailsOnMathError()
{
  smtk::common::InfixExpressionGrammar infix;
  smtk::common::InfixExpressionError err = smtk::common::InfixExpressionError::ERROR_NONE;
  smtkTest(std::isnan(infix.evaluate("ln(-5)", err)), "ln(-5) should not evaluate.")
    smtkTest(err == smtk::common::InfixExpressionError::ERROR_MATH_ERROR,
      "Expected err to be ERROR_MATH_ERROR")
}

void testSubexpressionEvaluationFailedError()
{
  smtk::common::InfixExpressionGrammar infix;
  infix.setSubsymbolVisitor([](const std::string&) { return std::pair<double, bool>(0.0, false); });
  smtk::common::InfixExpressionError err = smtk::common::InfixExpressionError::ERROR_NONE;
  smtkTest(std::isnan(infix.evaluate("{abc}", err)), "{abc} should not evaluate.")
    smtkTest(err == smtk::common::InfixExpressionError::ERROR_SUBEVALUATION_FAILED,
      "Expected err to be ERROR_SUBEVALUATION_FAILED")
}

void testFailsForInvalidSyntax()
{
  smtk::common::InfixExpressionGrammar infix;
  smtk::common::InfixExpressionError err = smtk::common::InfixExpressionError::ERROR_NONE;
  smtkTest(
    std::isnan(infix.evaluate("1 +", err)), "\"1 +\" should not evaluate but it is invalid syntax")
    smtkTest(err == smtk::common::InfixExpressionError::ERROR_INVALID_SYNTAX,
      "Expected err to be ERROR_INVALID_SYNTAX")
}

void testFailsForInvalidFunction()
{
  smtk::common::InfixExpressionGrammar infix;
  smtk::common::InfixExpressionError err = smtk::common::InfixExpressionError::ERROR_NONE;
  smtkTest(std::isnan(infix.evaluate("foo(7)", err)),
    "\"foo(7)\" should not evaluate because foo() has not been defined.")
    smtkTest(err == smtk::common::InfixExpressionError::ERROR_UNKNOWN_FUNCTION,
      "Expected err to be ERROR_INVALID_SYNTAX")
}

void testAddFunction()
{
  smtk::common::InfixExpressionGrammar infix;
  infix.addFunction("foo", [](double) { return 1.0; });
  smtk::common::InfixExpressionError err = smtk::common::InfixExpressionError::ERROR_NONE;
  smtkTest(infix.evaluate("foo(7)", err) == 1.0,
    "\"foo(7)\" should evalute to 1.0 because foo() was added as a function.")
    smtkTest(err == smtk::common::InfixExpressionError::ERROR_NONE, "Expected err to be ERROR_NONE")
}

int UnitTestInfixExpressionGrammar(int, char** const)
{
  testSimpleExpression();
  testFailsOnMathError();
  testSubexpressionEvaluationFailedError();
  testFailsForInvalidSyntax();
  testFailsForInvalidFunction();
  testAddFunction();

  return 0;
}
