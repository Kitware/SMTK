//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_InfixExpressionEvaluation_h
#define smtk_common_InfixExpressionEvaluation_h
/*!\file InfixExpressionEvaluation.h - Data structures for evaluating infix mathematical
 * expressions. */

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro
#include "smtk/SystemConfig.h"

#include <cmath>
#include <functional>
#include <map>
#include <string>
#include <vector>

// EvaluationOrder, InfixOperators, EvaluationStack, and EvaluationStacks are
// taken from taocpp/PEGTL/src/example/pegtl/calculator.cpp.

namespace smtk
{
namespace common
{

// A function to be called when a subsymbol is matched.
// Should return the value of the subsymbol or false in second if the
// subsymbol cannot be evaluated.
using SubsymbolVisitor = std::function<std::pair<double, bool>(const std::string& symbol)>;

enum class EvaluationOrder : int
{
};

struct InfixOperator
{
  EvaluationOrder p;
  std::function<double(double, double)> f;
};

// Unlike InfixOperator, InfixFunctions does not have EvaluationOrder. We assume all functions have the
// same precedence on a given level of associativity.
struct SMTKCORE_EXPORT InfixFunction
{
  std::function<double(double)> f;
};

class SMTKCORE_EXPORT EvaluationStack
{
public:
  void push(const InfixOperator& b)
  {
    while (!m_operators.empty() && m_operators.back().p <= b.p)
    {
      reduce();
    }
    m_operators.push_back(b);
  }

  void push(const double l) { m_operands.push_back(l); }

  double finish()
  {
    while (!m_operators.empty())
    {
      reduce();
    }

    const double result = m_operands.back();
    m_operands.clear();
    return result;
  }

private:
  std::vector<InfixOperator> m_operators;
  std::vector<double> m_operands;

  void reduce()
  {
    const double r = m_operands.back();
    m_operands.pop_back();
    const double l = m_operands.back();
    m_operands.pop_back();

    const InfixOperator o = m_operators.back();
    m_operators.pop_back();

    m_operands.push_back(o.f(l, r));
  }
};

class SMTKCORE_EXPORT EvaluationStacks
{
public:
  EvaluationStacks()
    : m_functionForNextOpen([](double d) { return d; })
  {
    open();
  }

  void open()
  {
    // The result of this StackLevel is itself unless |m_functionForNextOpen|
    // is set before the next call to open().
    m_stacks.emplace_back(EvaluationStack(), m_functionForNextOpen);
    m_functionForNextOpen = [](double d) { return d; };
  }

  void setFunctionForNextOpen(const std::function<double(double)>& functionForNextOpen)
  {
    m_functionForNextOpen = functionForNextOpen;
  }

  template<typename T>
  void push(const T& t)
  {
    m_stacks.back().first.push(t);
  }

  void close()
  {
    // finish()es the stack and calls this StackLevel's function on the result.
    const double result = m_stacks.back().second(m_stacks.back().first.finish());
    m_stacks.pop_back();
    m_stacks.back().first.push(result);
  }

  double finish() { return m_stacks.back().first.finish(); }

private:
  using StackLevel = std::pair<EvaluationStack, std::function<double(double)>>;

  std::vector<StackLevel> m_stacks;
  std::function<double(double)> m_functionForNextOpen;
};

class SMTKCORE_EXPORT InfixOperators
{
public:
  InfixOperators()
  {
    insert("*", EvaluationOrder(5), [](const double l, const double r) { return l * r; });
    insert("/", EvaluationOrder(5), [](const double l, const double r) { return l / r; });
    insert("+", EvaluationOrder(6), [](const double l, const double r) { return l + r; });
    insert("-", EvaluationOrder(6), [](const double l, const double r) { return l - r; });
    insert("^", EvaluationOrder(4), [](const double l, const double r) { return std::pow(l, r); });
  }

  void insert(
    const std::string& name,
    const EvaluationOrder p,
    const std::function<double(double, double)>& f)
  {
    m_ops.insert(std::make_pair(name, InfixOperator{ p, f }));
  }

  const std::map<std::string, InfixOperator>& ops() const noexcept { return m_ops; }

private:
  std::map<std::string, InfixOperator> m_ops;
};

class SMTKCORE_EXPORT InfixFunctions
{
public:
  InfixFunctions()
  {
    insert("sin", [](const double x) { return std::sin(x); });
    insert("cos", [](const double x) { return std::cos(x); });
    insert("tan", [](const double x) { return std::tan(x); });
    insert("asin", [](const double x) { return std::asin(x); });
    insert("acos", [](const double x) { return std::acos(x); });
    insert("atan", [](const double x) { return std::atan(x); });
    insert("sinh", [](const double x) { return std::sinh(x); });
    insert("cosh", [](const double x) { return std::cosh(x); });
    insert("tanh", [](const double x) { return std::tanh(x); });
    insert("asinh", [](const double x) { return std::asinh(x); });
    insert("acosh", [](const double x) { return std::acosh(x); });
    insert("atanh", [](const double x) { return std::atanh(x); });
    insert("ln", [](const double x) { return std::log(x); });
    insert("log10", [](const double x) { return std::log10(x); });
    insert("exp", [](const double x) { return std::exp(x); });
    insert("sqrt", [](const double x) { return std::sqrt(x); });
  }

  void insert(const std::string& name, const std::function<double(double)>& f)
  {
    m_functions.insert(std::make_pair(name, InfixFunction{ f }));
  }

  const std::map<std::string, InfixFunction>& funcs() const noexcept { return m_functions; }

private:
  std::map<std::string, InfixFunction> m_functions;
};

} // namespace common
} // namespace smtk

#endif // smtk_common_InfixExpressionEvaluation_h
