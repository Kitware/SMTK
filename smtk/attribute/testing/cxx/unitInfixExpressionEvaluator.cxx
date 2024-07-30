//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
/*!\file unitInfixExpressionEvaluator.cxx - Unit tests for InfixExpressionEvaluator. */

#include "smtk/attribute/InfixExpressionEvaluator.h"

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <boost/variant.hpp>

// clang-format off

const std::string sbt = R"(
  <?xml version="1.0" encoding="utf-8" ?>
  <SMTK_AttributeResource Version="8">
    <Definitions>
      <AttDef Type="infixExpression" Label="InfixExpression"  Units="feet">
        <ItemDefinitions>
          <String Name="expression" Extensible="true"/>
        </ItemDefinitions>
      </AttDef>
    <AttDef Type="A" Label="A" BaseType="" Unique="false">
      <ItemDefinitions>
        <Double Name="d1" Label="Expression Double" Units="in">
          <ExpressionType>infixExpression</ExpressionType>
        </Double>
        <Double Name="d2" Label="Expression Double" Units="K">
          <ExpressionType>infixExpression</ExpressionType>
        </Double>
      </ItemDefinitions>
    </AttDef>
    </Definitions>
  </SMTK_AttributeResource>
)";

// clang-format on

smtk::attribute::ResourcePtr createResourceForTest()
{
  smtk::attribute::ResourcePtr attRes = smtk::attribute::Resource::create();
  smtk::io::Logger logger;
  smtk::io::AttributeReader reader;
  if (reader.readContents(attRes, sbt, logger))
  {
    std::cout << logger.convertToString() << std::endl;
  }

  // We need to register InfixExpressionExpressionEvalutor so that our tests
  // can evaluate dependent expressions.
  smtk::attribute::DefinitionPtr infixExpDef = attRes->findDefinition("infixExpression");
  attRes->evaluatorFactory().registerEvaluator<smtk::attribute::InfixExpressionEvaluator>(
    "InfixExpressionEvaluator");
  attRes->evaluatorFactory().addDefinitionForEvaluator(
    "InfixExpressionEvaluator", infixExpDef->type());

  return attRes;
}

void testSimpleEvaluation()
{
  smtk::attribute::ResourcePtr attRes = createResourceForTest();
  smtk::attribute::DefinitionPtr infixExpDef = attRes->findDefinition("infixExpression");
  smtk::attribute::AttributePtr infixExpAtt = attRes->createAttribute(infixExpDef);
  smtk::attribute::AttributePtr a = attRes->createAttribute("A");

  infixExpAtt->findString("expression")->setValue("2 + 2");

  smtk::attribute::InfixExpressionEvaluator infixEvaluator(infixExpAtt);
  smtk::attribute::Evaluator::ValueType result;
  smtk::io::Logger log;

  smtkTest(
    infixEvaluator.evaluate(
      result, log, 0, smtk::attribute::Evaluator::DependentEvaluationMode::EVALUATE_DEPENDENTS) ==
      true,
    "Failed to evalute 2 + 2.")
    smtkTest(log.numberOfRecords() == 0, "Expected log to have no records.")

      double computation = boost::get<double>(result);

  smtkTest(computation == 4.0, "Incorrectly computed 2 + 2.")

    //Lets try assigning the expression which is in inches to a double whose units are feet
    auto d1 = a->findDouble("d1");
  smtkTest(d1->setExpression(infixExpAtt), "Could not set expression to d1.");
  smtkTest(
    d1->value() == 48.0,
    "Item d1 should have return 48 (inches) but instead returned: " << d1->value() << ".");
  // We should be able to set the expression on d2 which whose units are Kelvin
  auto d2 = a->findDouble("d2");
  smtkTest(!d2->setExpression(infixExpAtt), "Was able to set expression to d2.");
}

void testOneChildExpression()
{
  smtk::attribute::ResourcePtr attRes = createResourceForTest();
  smtk::attribute::DefinitionPtr infixExpDef = attRes->findDefinition("infixExpression");
  smtk::attribute::AttributePtr expressionB = attRes->createAttribute("b", infixExpDef);
  smtk::attribute::AttributePtr expressionA = attRes->createAttribute("a", infixExpDef);

  expressionB->findString("expression")->setValue("5 + {a}");
  expressionA->findString("expression")->setValue("2 * 2");

  smtk::attribute::InfixExpressionEvaluator infixEvaluator(expressionB);
  smtk::attribute::Evaluator::ValueType result;
  smtk::io::Logger log;

  smtkTest(
    infixEvaluator.evaluate(
      result, log, 0, smtk::attribute::Evaluator::DependentEvaluationMode::EVALUATE_DEPENDENTS) ==
      true,
    "Failed to evalute b, where b = 5 + {a}, a = 2 * 2..")
    smtkTest(log.numberOfRecords() == 0, "Expected log to have no records.")
    //  smtkTest(infixEvaluator.getContext()->childExpressions.count("a") == true,
    //           "\"a\" should be a child expression of \"b\".")

    double computation = boost::get<double>(result);

  smtkTest(computation == 9.0, "Incorrectly computed b, where b = 5 + {a}, a = 2 * 2.")
}

void testSelfReferencingExpressionFails()
{
  smtk::attribute::ResourcePtr attRes = createResourceForTest();
  smtk::attribute::DefinitionPtr infixExpDef = attRes->findDefinition("infixExpression");
  smtk::attribute::AttributePtr infixExpAtt = attRes->createAttribute("a", infixExpDef);

  infixExpAtt->findString("expression")->setValue("{a} + 2");

  smtk::attribute::InfixExpressionEvaluator infixEvaluator(infixExpAtt);
  smtk::attribute::Evaluator::ValueType result;
  smtk::io::Logger log;

  smtkTest(
    infixEvaluator.evaluate(
      result, log, 0, smtk::attribute::Evaluator::DependentEvaluationMode::EVALUATE_DEPENDENTS) ==
      false,
    "a = {a} + 2 should not evaluate.")
    smtkTest(
      log.numberOfRecords() == 2,
      "Expected log to have 2 records: 1 from subevaluation and 1 from logError().")
  //  smtkTest(infixEvaluator.getContext()->childExpressions.size() == 0,
  //           "\"a\" should have no child expressions.")
}

void testCyclicReferenceExpressionFails()
{
  smtk::attribute::ResourcePtr attRes = createResourceForTest();
  smtk::attribute::DefinitionPtr infixExpDef = attRes->findDefinition("infixExpression");
  smtk::attribute::AttributePtr expressionB = attRes->createAttribute("b", infixExpDef);
  smtk::attribute::AttributePtr expressionA = attRes->createAttribute("a", infixExpDef);

  expressionB->findString("expression")->setValue("5 + {a}");
  expressionA->findString("expression")->setValue("{b} * 2");

  smtk::attribute::InfixExpressionEvaluator infixEvaluator(expressionB);
  smtk::attribute::Evaluator::ValueType result;
  smtk::io::Logger log;

  smtkTest(
    infixEvaluator.evaluate(
      result, log, 0, smtk::attribute::Evaluator::DependentEvaluationMode::EVALUATE_DEPENDENTS) ==
      false,
    "A child expression referencing a parent expression should not evaluate.")
    smtkTest(
      log.numberOfRecords() == 4,
      "Expected log to have 4 records: 1 from each subevaluation and 1 from each logError().")
  // smtkTest(infixEvaluator.getContext()->childExpressions.count("b"), )
}

void testReferencingNonexistentSubexressionFails()
{
  smtk::attribute::ResourcePtr attRes = createResourceForTest();
  smtk::attribute::DefinitionPtr infixExpDef = attRes->findDefinition("infixExpression");
  smtk::attribute::AttributePtr expressionA = attRes->createAttribute("a", infixExpDef);

  expressionA->findString("expression")->setValue("{b} * 2");

  smtk::attribute::InfixExpressionEvaluator infixEvaluator(expressionA);
  smtk::attribute::Evaluator::ValueType result;
  smtk::io::Logger log;

  smtkTest(
    infixEvaluator.evaluate(
      result, log, 0, smtk::attribute::Evaluator::DependentEvaluationMode::EVALUATE_DEPENDENTS) ==
      false,
    "A reference to a nonexistent subexpression should fail.")
    smtkTest(
      log.numberOfRecords() == 2,
      "Expected log to have 2 records: 1 from subevalution and 1 from logError().")
  //  smtkTest(infixEvaluator.getContext()->childExpressions.count("b") == 1,
  //           "\"b\" should be a child expression of \"a\".")
}

void testMultipleReferencesInParentExpression()
{
  smtk::attribute::ResourcePtr attRes = createResourceForTest();
  smtk::attribute::DefinitionPtr infixExpDef = attRes->findDefinition("infixExpression");
  smtk::attribute::AttributePtr expressionA = attRes->createAttribute("a", infixExpDef);
  smtk::attribute::AttributePtr expressionB = attRes->createAttribute("b1", infixExpDef);
  smtk::attribute::AttributePtr expressionC = attRes->createAttribute("c2", infixExpDef);

  expressionA->findString("expression")->setValue("{b1} - {c2}");
  expressionB->findString("expression")->setValue("1");
  expressionC->findString("expression")->setValue("1");

  smtk::attribute::InfixExpressionEvaluator infixEvaluator(expressionA);
  smtk::attribute::Evaluator::ValueType result;
  smtk::io::Logger log;

  smtkTest(
    infixEvaluator.evaluate(
      result, log, 0, smtk::attribute::Evaluator::DependentEvaluationMode::EVALUATE_DEPENDENTS),
    "{b1} - {c2} should evaluate successfully.")
    smtkTest(log.numberOfRecords() == 0, "Expected log to have no records.")
  //  smtkTest(infixEvaluator.getContext()->childExpressions.size() == 2,
  //           "Expected 2 child expressions")
  //  smtkTest(infixEvaluator.getContext()->childExpressions.count("b") == 1,
  //           "Expected b to be a child for {b} + {c}")
  //  smtkTest(infixEvaluator.getContext()->childExpressions.count("c") == 1,
  //           "Expected b to be a child for {b} + {c}")
}

// test we can use "-" and "_" in names
void testReferenceNames()
{
  smtk::attribute::ResourcePtr attRes = createResourceForTest();
  smtk::attribute::DefinitionPtr infixExpDef = attRes->findDefinition("infixExpression");
  smtk::attribute::AttributePtr expressionA = attRes->createAttribute("a", infixExpDef);
  smtk::attribute::AttributePtr expressionB = attRes->createAttribute("b_", infixExpDef);
  smtk::attribute::AttributePtr expressionC = attRes->createAttribute("C-dash2", infixExpDef);

  expressionA->findString("expression")->setValue("{b_} - {C-dash2}");
  expressionB->findString("expression")->setValue("1");
  expressionC->findString("expression")->setValue("2");

  smtk::attribute::InfixExpressionEvaluator infixEvaluator(expressionA);
  smtk::attribute::Evaluator::ValueType result;
  smtk::io::Logger log;

  smtkTest(
    infixEvaluator.evaluate(
      result, log, 0, smtk::attribute::Evaluator::DependentEvaluationMode::EVALUATE_DEPENDENTS),
    "{b_} - {C-dash2} should evaluate successfully.")
    smtkTest(log.numberOfRecords() == 0, "Expected log to have no records.")
    //  smtkTest(infixEvaluator.getContext()->childExpressions.size() == 2,
    //           "Expected 2 child expressions")
    //  smtkTest(infixEvaluator.getContext()->childExpressions.count("b") == 1,
    //           "Expected b to be a child for {b} + {c}")
    //  smtkTest(infixEvaluator.getContext()->childExpressions.count("c") == 1,
    //           "Expected b to be a child for {b} + {c}")
    double computation = boost::get<double>(result);

  smtkTest(computation == -1.0, "Incorrectly computed a.")
}

void testSetMultipleExpressionsOnSingleAttribute()
{
  smtk::attribute::ResourcePtr attRes = createResourceForTest();
  smtk::attribute::DefinitionPtr infixExpDef = attRes->findDefinition("infixExpression");
  smtk::attribute::AttributePtr expressionAtt = attRes->createAttribute("a", infixExpDef);

  expressionAtt->findString("expression")->setValue("3");
  expressionAtt->findString("expression")->setValue("9");
}

// Tests for doesEvaluate() and doesEvaluate(std::size_t).
void testDoesEvaluate()
{
  smtk::attribute::ResourcePtr attRes = createResourceForTest();
  smtk::attribute::DefinitionPtr infixExpDef = attRes->findDefinition("infixExpression");
  smtk::attribute::AttributePtr expressionAtt = attRes->createAttribute("a", infixExpDef);

  smtk::attribute::InfixExpressionEvaluator infixEvaluator(expressionAtt);

  expressionAtt->findString("expression")->setValue("");
  smtkTest(
    infixEvaluator.doesEvaluate() == false,
    "An empty string is not considered a proper evaluation.")
    smtkTest(infixEvaluator.doesEvaluate(0) == false, "Expected evaluation at index 0 to fail.")

      expressionAtt->findString("expression")
        ->setValue("1");
  smtkTest(infixEvaluator.doesEvaluate() == true, "A single well-formed expression is evaluatable.")
    smtkTest(infixEvaluator.doesEvaluate(0) == true, "Expected evaluation at index 0 to succeed.")

      expressionAtt->findString("expression")
        ->setValue("1 + ");
  smtkTest(
    infixEvaluator.doesEvaluate() == false, "A single ill-formed expression is not evaluatable.")

    expressionAtt->findString("expression")
      ->appendValue("2");
  smtkTest(
    infixEvaluator.doesEvaluate() == false,
    "Expected doesEvaluate() to return false if any expression is not"
    " evaluatable")

    smtkTest(
      infixEvaluator.doesEvaluate(5) == false,
      "Expected index 5, which deos not exist, to not be evaluatable")
}

void testNumberOfEvaluatableElements()
{
  smtk::attribute::ResourcePtr attRes = createResourceForTest();
  smtk::attribute::DefinitionPtr infixExpDef = attRes->findDefinition("infixExpression");
  smtk::attribute::AttributePtr expressionAtt = attRes->createAttribute("a", infixExpDef);

  smtk::attribute::InfixExpressionEvaluator infixEvaluator(expressionAtt);

  smtkTest(
    infixEvaluator.numberOfEvaluatableElements() == 1,
    "A default infix expression should have 1 evaluatable element.")

    expressionAtt->findString("expression")
      ->appendValue("");
  smtkTest(
    infixEvaluator.numberOfEvaluatableElements() == 2,
    "Expected to have 2 evalutable elements after appending a string.")
}

int unitInfixExpressionEvaluator(int /*argc*/, char** const /*argv*/)
{
  testSimpleEvaluation();
  testOneChildExpression();
  testSelfReferencingExpressionFails();
  testCyclicReferenceExpressionFails();
  testReferencingNonexistentSubexressionFails();
  testMultipleReferencesInParentExpression();
  testReferenceNames();
  testSetMultipleExpressionsOnSingleAttribute();
  testDoesEvaluate();
  testNumberOfEvaluatableElements();

  return 0;
}
