//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_common_InfixExpressionGrammar_h
#define __smtk_common_InfixExpressionGrammar_h
/*!\file InfixExpressionGrammar.h - Public interface to evaluating infix mathematical expressions. */

#include <functional>
#include <map>
#include <string>

#include "smtk/CoreExports.h"

#include "smtk/common/InfixExpressionError.h"
#include "smtk/common/InfixExpressionEvaluation.h"

namespace smtk
{
namespace common
{

class SMTKCORE_EXPORT InfixExpressionGrammar
{
public:
  InfixExpressionGrammar();
  ~InfixExpressionGrammar() = default;

  InfixExpressionGrammar(const InfixExpressionGrammar& other) = default;

  void addFunction(const std::string& name, const std::function<double(double)>& f);
  const std::map<std::string, InfixFunction>& functions() const;

  void setSubsymbolVisitor(const SubsymbolVisitor& f);
  SubsymbolVisitor subsymbolVisitor() const;

  // Parses and evaluates |expression|, sets |err| to the status code of the evaluation.
  // Calls testExpressionSyntax(), so any code from there can be set, in addition to
  // ERROR_MATH_ERROR.
  double evaluate(const std::string& expression, InfixExpressionError& err) const;

  // Tests |expression| for possible errors, without computing its result.
  // Returns:
  //      ERROR_NONE if successful.
  //      ERROR_INVALID_SYNTAX if |expression| is not a valid infix expression.
  //      ERROR_INVALID_FUNCTION if |expression| uses a function not in |m_functions|.
  //      ERROR_SUBEVALUATION_FAILED if |m_subexpressionFunctor| fails.
  InfixExpressionError testExpressionSyntax(const std::string& expression) const;

private:
  InfixExpressionError testExpressionSyntax(
    const std::string& expression,
    InfixOperators& ops,
    EvaluationStacks& stacks) const;

  InfixFunctions m_functions;
  SubsymbolVisitor m_subsymbolVisitor;
};

} // namespace common
} // namespace smtk

#endif // __smtk_common_InfixExpressionGrammar_h
