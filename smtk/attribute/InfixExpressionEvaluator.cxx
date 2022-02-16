//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "InfixExpressionEvaluator.h"

#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/SymbolDependencyStorage.h"

#include "smtk/common/InfixExpressionGrammar.h"

#include <string>
#include <unordered_set>

smtk::attribute::InfixExpressionEvaluator::InfixExpressionEvaluator(ConstAttributePtr att)
  : Evaluator(att)
{
}

bool smtk::attribute::InfixExpressionEvaluator::evaluate(
  smtk::attribute::Evaluator::ValueType& result,
  smtk::io::Logger& log,
  const std::size_t& element,
  const DependentEvaluationMode& evaluationMode)
{
  smtk::attribute::ConstAttributePtr att = attribute().lock();

  if (!att)
  {
    return false;
  }

  smtk::attribute::ResourcePtr attRes =
    std::dynamic_pointer_cast<smtk::attribute::Resource>(att->resource());
  if (!attRes)
  {
    return false;
  }

  smtk::common::InfixExpressionGrammar grammar;

  const std::string attSymbol = att->name();
  SymbolDependencyStorage& ctxtStorage = attRes->queries().cache<SymbolDependencyStorage>();

  // Collects symbols used by this expression.
  std::unordered_set<std::string> symbolsUsed;

  // Tells |grammar| to attempt to create an Evaluator and evaluate() child
  // expressions.
  smtk::common::SubsymbolVisitor childExpressionVisitor =
    [&symbolsUsed, &ctxtStorage, attSymbol, attRes, &log, element](const std::string& symbol) {
      // Are we attempting to reference ourself?
      if (attSymbol == symbol)
      {
        log.addRecord(
          smtk::io::Logger::Error, "Cannot write " + attSymbol + " in terms of itself.");
        return std::pair<double, bool>(0.0, false);
      }

      // Are we attempting to reference a dependent expression?
      // If we can reach |symbol| from |attSymbol|, this would be a cycle.
      if (ctxtStorage.isDependentOn(attSymbol, symbol))
      {
        log.addRecord(
          smtk::io::Logger::Error,
          "Cannot use " + symbol + " in expression " + attSymbol + " because the expression " +
            symbol + " already uses " + attSymbol + ".");
        return std::pair<double, bool>(0.0, false);
      }

      symbolsUsed.insert(symbol);

      // |attSymbol| is dependent on |symbol|.
      ctxtStorage.addDependency(symbol, attSymbol);

      smtk::attribute::AttributePtr childAtt = attRes->findAttribute(symbol);
      if (!childAtt)
      {
        log.addRecord(
          smtk::io::Logger::Error, "Cannot find referenced attribute with name " + symbol);
        return std::pair<double, bool>(0.0, false);
      }

      std::unique_ptr<smtk::attribute::Evaluator> childEvaluator =
        attRes->createEvaluator(childAtt);
      if (childEvaluator)
      {
        // Recursively evaluates this child so we can learn its result. evaluationMode
        // is set to DO_NOT_EVALUATE_DEPENDENTS to prevent infinite recursion.
        ValueType result;
        if (childEvaluator->evaluate(
              result, log, element, DependentEvaluationMode::DO_NOT_EVALUATE_DEPENDENTS))
        {
          try
          {
            // TODO: The result of evaluate() could be an int, but there are no
            // Evaluators that currently return an int, so this is OK for now.
            return std::pair<double, bool>(boost::get<double>(result), true);
          }
          catch (const boost::bad_get&)
          {
            // This tells us that the result of |childEvaluator| was not
            // compatible with InfixExpressionEvaluator.
            log.addRecord(
              smtk::io::Logger::Error,
              "Result type of child expression evaluation was not "
              "compatible with an infix expression.");
            return std::pair<double, bool>(0.0, false);
          }
        }
        else
        {
          log.addRecord(smtk::io::Logger::Error, "Evaluation failed for " + symbol + ".");
          return std::pair<double, bool>(0.0, false);
        }
      }

      log.addRecord(
        smtk::io::Logger::Error, "Referenced attribute " + symbol + " is not evaluatable");
      return std::pair<double, bool>(0.0, false);
    };
  grammar.setSubsymbolVisitor(childExpressionVisitor);

  smtk::attribute::ConstStringItemPtr expressionStringItem = att->findString("expression");
  if (!expressionStringItem->isSet(element))
  {
    log.addRecord(
      smtk::io::Logger::Error,
      "Missing element " + std::to_string(element + 1) + " needed for evaluation.");
    return false;
  }

  const std::string expressionStr = att->findString("expression")->value(element);

  smtk::common::InfixExpressionError err = smtk::common::InfixExpressionError::ERROR_NONE;
  const double evaluationResult = grammar.evaluate(expressionStr, err);

  if (err == smtk::common::InfixExpressionError::ERROR_NONE)
  {
    result = evaluationResult;

    // Need to know: what symbols list "a" as their dependent? if "a" no longer uses them,
    // we need to remove the dependency.
    ctxtStorage.pruneOldSymbols(symbolsUsed, attSymbol);

    if (evaluationMode == DependentEvaluationMode::EVALUATE_DEPENDENTS)
    {
      // allDependentSymbols() returns dependents in a level-order traversal order,
      // so these evaluators will run in an order in keeping with the dependency ordering.
      for (const std::string& dependent : ctxtStorage.allDependentSymbols(attSymbol))
      {
        smtk::attribute::AttributePtr dependentAtt = attRes->findAttribute(dependent);
        if (!att)
          continue;

        std::unique_ptr<smtk::attribute::Evaluator> dependentEvaluator =
          attRes->createEvaluator(dependentAtt);
        if (!dependentEvaluator)
          continue;

        ValueType dependentResult;
        // evaluationMode is set to DO_NOT_EVALUATE_DEPENDENTS to prevent infinite recursion.
        dependentEvaluator->evaluate(
          dependentResult, log, element, DependentEvaluationMode::DO_NOT_EVALUATE_DEPENDENTS);
      }
    }

    return true;
  }
  else
  {
    logError(err, log);
  }

  return false;
}

// InfixExpressionEvaluates chooses not to place anything in the Logger at this
// time.
bool smtk::attribute::InfixExpressionEvaluator::canEvaluate(smtk::io::Logger& /*log*/)
{
  smtk::attribute::ConstAttributePtr att = attribute().lock();
  return !!att && att->findString("expression") && att->findDouble("value");
}

bool smtk::attribute::InfixExpressionEvaluator::doesEvaluate(std::size_t element)
{
  if (element >= numberOfEvaluatableElements())
    return false;

  ValueType result;
  smtk::io::Logger log;
  evaluate(result, log, element, DependentEvaluationMode::DO_NOT_EVALUATE_DEPENDENTS);
  return !log.hasErrors();
}

bool smtk::attribute::InfixExpressionEvaluator::doesEvaluate()
{
  for (std::size_t i = 0; i < numberOfEvaluatableElements(); ++i)
  {
    if (!doesEvaluate(i))
      return false;
  }

  return true;
}

std::size_t smtk::attribute::InfixExpressionEvaluator::numberOfEvaluatableElements()
{
  smtk::attribute::ConstAttributePtr att = attribute().lock();
  if (!att)
    return 0;

  smtk::attribute::ConstStringItemPtr expressionStringItem = att->findString("expression");
  if (!expressionStringItem)
    return 0;

  return expressionStringItem->numberOfValues();
}

void smtk::attribute::InfixExpressionEvaluator::logError(
  const smtk::common::InfixExpressionError& err,
  smtk::io::Logger& log) const
{
  std::stringstream stream;
  switch (err)
  {
    case smtk::common::InfixExpressionError::ERROR_NONE:
      return;
    case smtk::common::InfixExpressionError::ERROR_INVALID_TOKEN:
      stream << "Invalid token in expression.";
      break;
    case smtk::common::InfixExpressionError::ERROR_INVALID_SYNTAX:
      stream << "Invalid expression syntax.";
      break;
    case smtk::common::InfixExpressionError::ERROR_UNKNOWN_FUNCTION:
      stream << "Unknown function used in expression.";
      break;
    case smtk::common::InfixExpressionError::ERROR_UNKNOWN_OPERATOR:
      stream << "Unknown operator used in expression.";
      break;
    case smtk::common::InfixExpressionError::ERROR_MATH_ERROR:
      stream << "Math domain or range error in expression.";
      break;
    case smtk::common::InfixExpressionError::ERROR_SUBEVALUATION_FAILED:
      stream << "Could not evaluate subexpression.";
      break;
  }

  log.addRecord(smtk::io::Logger::Error, stream.str());
}
