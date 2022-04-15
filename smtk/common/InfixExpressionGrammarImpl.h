//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_ExpressionGrammarImpl_h
#define smtk_common_ExpressionGrammarImpl_h
/*!\file InfixExpressionGrammarImpl.h - PEGTL structures for parsing mathematical expressions. */

#include <functional>
#include <iostream>
#include <map>
#include <sstream>

#include "smtk/CoreExports.h"

#include "smtk/common/InfixExpressionError.h"
#include "smtk/common/InfixExpressionEvaluation.h"

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/abnf.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

#ifdef ERROR_INVALID_FUNCTION
#undef ERROR_INVALID_FUNCTION
#endif

namespace smtk
{
namespace common
{

namespace expression_internal
{

using namespace tao::pegtl;

// clang-format off

struct ignored
    : space
{};

struct infix_operator
    : one< '+', '-', '*', '/', '^' >
{};

struct symbol
    : seq< abnf::ALPHA, star< sor< ranges< 'a', 'z', 'A', 'Z', '0', '9'>, one< '-', '_' > > > >
{};

struct function_name
    : seq< abnf::ALPHA, star < sor< ranges< 'a', 'z', 'A', 'Z', '0', '9' >, one< '-', '_' > > > >
{};

// Rules for floating-point numbers are taken from
// taocpp/json/include/tao/json/internal/grammar.hpp.
struct digits
    : plus< digit >
{};

struct sign
    : opt< one < '+', '-' > >
{};

struct exp
    : seq< one < 'e', 'E'>, opt< sign >, must< digits > >
{};

struct frac
    : if_must< one< '.' >, opt< digits > >
{};

struct number
    : seq< sign, digits, opt< frac >, opt< exp > >
{};

struct expression;

struct infix_function
    : if_must< function_name, star< ignored >, one< '(' >, star< ignored >, expression, star< ignored >, one< ')' > >
{};

struct paren
    : if_must< one< '(' >, expression, one< ')' > >
{};

struct subsymbol_reference
    : if_must< one< '{' >, symbol, one< '}' > >
{};

struct atomic
    : sor< number, paren, infix_function, subsymbol_reference >
{};

struct expression
    : list< atomic, infix_operator, ignored >
{};

struct expression_grammar :
    must< star< ignored >, expression, star< ignored >, eof >
{};

// clang-format on

// Our action base class inherits from |nothing| because every action must have
// apply() or apply0(). |nothing| tells PEGTL we're OK with this action doing
// nothing.
template<typename Rule>
struct ExpressionAction : nothing<Rule>
{
};

template<>
struct ExpressionAction<number>
{
  template<typename ActionInput>
  static void apply(
    const ActionInput& in,
    const InfixOperators& /* unused */,
    EvaluationStacks& s,
    const InfixFunctions& /* unused */,
    const SubsymbolVisitor& /* unused */,
    InfixExpressionError& /* unused: if we matched a number, this cannot be an invalid token */)
  {
    std::stringstream ss(in.string());
    double v;
    ss >> v;
    s.push(v);
  }
};

template<>
struct ExpressionAction<infix_operator>
{
  template<typename ActionInput>
  static void apply(
    const ActionInput& in,
    const InfixOperators& b,
    EvaluationStacks& s,
    const InfixFunctions& /* unused */,
    const SubsymbolVisitor& /* unused */,
    InfixExpressionError& err)
  {
    std::string str = in.string();
    const std::map<std::string, InfixOperator>::const_iterator it = b.ops().find(str);
    if (it != b.ops().end())
    {
      s.push(it->second);
    }
    else
    {
      // this is an invalid operator, but this rule would never be matched in
      // that case.
      err = InfixExpressionError::ERROR_UNKNOWN_OPERATOR;
      throw parse_error("Invalid operator.", in);
    }
  }
};

template<>
struct ExpressionAction<function_name>
{
  template<typename ActionInput>
  static void apply(
    const ActionInput& in,
    const InfixOperators& /* unused */,
    EvaluationStacks& s,
    const InfixFunctions& funcs,
    const SubsymbolVisitor& /* unused */,
    InfixExpressionError& err)
  {
    std::string str = in.string();
    std::size_t openingParenIdx = str.find_first_of('(');
    std::string functionName = str.substr(0, openingParenIdx);

    const std::map<std::string, InfixFunction>::const_iterator it =
      funcs.funcs().find(functionName);
    if (it != funcs.funcs().end())
    {
      s.setFunctionForNextOpen(it->second.f);
    }
    else
    {
      err = InfixExpressionError::ERROR_UNKNOWN_FUNCTION;
      throw parse_error("Invalid function.", in);
    }
  }
};

template<>
struct ExpressionAction<subsymbol_reference>
{
  template<typename ActionInput>
  static void apply(
    const ActionInput& in,
    const InfixOperators& /* unused */,
    EvaluationStacks& s,
    const InfixFunctions& /* unused */,
    const SubsymbolVisitor& subFunc,
    InfixExpressionError& err)
  {
    std::string str = in.string();

    // A substring that excludes the braces. I.e, "abc" is passed to subFunc
    // when we matched "{abc}".
    std::pair<double, bool> result = subFunc(str.substr(1, str.size() - 2));

    // TODO: if a subexpression evaluates to nan or inf, isn't that really a
    // math error?
    if (!result.second || std::isnan(result.first) || std::isinf(result.first))
    {
      err = InfixExpressionError::ERROR_SUBEVALUATION_FAILED;
      throw parse_error("Subexpression could not be evaluated.", in);
    }
    else
    {
      s.push(result.first);
    }
  }
};

template<>
struct ExpressionAction<one<'('>>
{
  static void apply0(
    const InfixOperators& /* unused */,
    EvaluationStacks& s,
    const InfixFunctions& /* unused */,
    const SubsymbolVisitor& /* unused */,
    InfixExpressionError& /* unused */)
  {
    s.open();
  }
};

template<>
struct ExpressionAction<one<')'>>
{
  static void apply0(
    const InfixOperators& /* unused */,
    EvaluationStacks& s,
    const InfixFunctions& /* unused */,
    const SubsymbolVisitor& /* unused */,
    InfixExpressionError& /* unused */)
  {
    s.close();
  }
};

} // namespace expression_internal

} // namespace common
} // namespace smtk

#endif // smtk_common_ExpressionGrammarImpl_h
