//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
/*!\file unitDoubleItem.cxx - Unit tests for DoubleItem as a proxy for testing
    ValueItemTemplate. */

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/json/jsonResource.h"

#include "smtk/attribute/Evaluator.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <nlohmann/json.hpp>

#include <string>

using namespace smtk::attribute;

// An Evaluator for testing which converts strings to double.
class StringToDoubleEvaluator : public Evaluator
{
public:
  StringToDoubleEvaluator(ConstAttributePtr att)
    : Evaluator(att)
  {
  }

  bool canEvaluate(smtk::io::Logger& /*log*/) override { return true; }

  bool evaluate(ValueType& result, smtk::io::Logger& log, const std::size_t& element,
    const DependentEvaluationMode& evaluationMode) override
  {
    // Evaluating dependents does not make sense for this Evaluator.
    (void)evaluationMode;

    ConstAttributePtr att = attribute().lock();
    const std::string text = att->findString("text")->value(element);

    // stod() can also throw std::out_of_range but checking for that is overkill
    // in this testing context.
    double conversion;
    try
    {
      conversion = std::stod(text);
    }
    catch (const std::invalid_argument&)
    {
      log.addRecord(
        smtk::io::Logger::ERROR, "Text was not convertible to a floating-point representation.");
      return false;
    }

    result = conversion;
    return true;
  }

  // Ignoring a proper implementation of doesEvaluate(std::size_t),
  // doesEvaluate(), and numberOfEvaluatableElements()because it is beyond the
  // scope of these tests.
  bool doesEvaluate(std::size_t /*element*/) override { return true; }
  bool doesEvaluate() override { return true; }

  std::size_t numberOfEvaluatableElements() override { return 1; }
};

// Mocks an Attribute Resource with a single DoubleItem.
//
// Instantiate the class, then modify the DoubleItemDefinition via
// mp_doubleItemDef and get a DoubleItem to test by calling getDoubleItem().
class DoubleItemTest
{
public:
  DoubleItemTest()
  {
    mp_attRes = Resource::create();
    mp_def = mp_attRes->createDefinition("def");
    mp_doubleItemDef = DoubleItemDefinition::New("testDoubleItem");
    mp_def->addItemDefinition(mp_doubleItemDef);
  }

  DoubleItemPtr getDoubleItem()
  {
    return mp_attRes->createAttribute(mp_def)->findDouble("testDoubleItem");
  }

  void SetUpDoubleItemTestExpressions()
  {
    mp_evaluatorDef = mp_attRes->createDefinition("doubleItemTestExpression");
    StringItemDefinitionPtr textItemDef = StringItemDefinition::New("text");
    textItemDef->setIsExtensible(true);
    mp_evaluatorDef->addItemDefinition(textItemDef);

    mp_attRes->evaluatorFactory().registerEvaluator<StringToDoubleEvaluator>(
      "StringToDoubleEvaluator");
    mp_attRes->evaluatorFactory().addDefinitionForEvaluator(
      "StringToDoubleEvaluator", mp_evaluatorDef->type());

    mp_doubleItemDef->setExpressionDefinition(mp_evaluatorDef);
  }

  ResourcePtr mp_attRes;
  DefinitionPtr mp_def;
  DoubleItemDefinitionPtr mp_doubleItemDef;

  DefinitionPtr mp_evaluatorDef;
};

void testBasicGettingValue()
{
  DoubleItemTest doubleItemTest;

  DoubleItemPtr item = doubleItemTest.getDoubleItem();

  smtkTest(item->value() == 0.0, "Expected an unset value to be zero-initialized.")
    smtkTest(smtk::io::Logger::instance().hasErrors() == true,
      "Expected the global logger to have an error.")

      smtk::io::Logger::instance()
        .reset();
  item->setValue(5.0);

  smtkTest(item->value() == 5.0, "Expected value to be 5.0") smtkTest(
    smtk::io::Logger::instance().hasErrors() == false, "Expected global logger to have no errors.")
}

void testGettingValueWithExpression()
{
  DoubleItemTest doubleItemTest;
  doubleItemTest.SetUpDoubleItemTestExpressions();

  AttributePtr expressionAtt =
    doubleItemTest.mp_attRes->createAttribute(doubleItemTest.mp_evaluatorDef);
  expressionAtt->findString("text")->setValue("5.0");

  DoubleItemPtr item = doubleItemTest.getDoubleItem();
  smtkTest(
    item->setExpression(expressionAtt) == true, "Expected to set expression on the DoubleItem.")

    smtk::io::Logger log;
  smtkTest(item->value(0, log) == 5.0,
    "Expected the string \"5.0\" to evaluate to 5.0 for the set expression.")
    smtkTest(log.hasErrors() == false, "Expected logger to have no new errors.")

    // Causes StringToDoubleEvaluator to fail.
    expressionAtt->findString("text")
      ->setValue("abc123");
  smtkTest(item->value(0, log) == 0.0, "Expected a failed evaluation to return 0.0.")
    smtkTest(log.hasErrors() == true, "Expected logger to have errors for failed evaluation.")

      log.reset();
  doubleItemTest.mp_attRes->evaluatorFactory().unregisterEvaluator<StringToDoubleEvaluator>();

  // StringToDoubleEvaluator has been unregistered, so no Evaluator can be created.
  smtkTest(item->value(0, log) == 0.0, "Expected failure to create an Evaluator to return 0.0.")
    smtkTest(
      log.hasErrors() == true, "Expected logger to have errors for failure to create an Evaluator.")
}

void testValueAsString()
{
  {
    DoubleItemTest doubleItemTest;

    DoubleItemPtr item = doubleItemTest.getDoubleItem();

    // 0.5 has a simple mantissa and exponent for verification.
    double val = 0.5;
    item->setValue(val);

    smtkTest(item->valueAsString() == "0.5", "Expected \"0.5\" from valueAsString().")
  }

  {
    DoubleItemTest doubleItemTest;
    doubleItemTest.SetUpDoubleItemTestExpressions();

    AttributePtr expressionAtt =
      doubleItemTest.mp_attRes->createAttribute(doubleItemTest.mp_evaluatorDef);
    expressionAtt->findString("text")->setValue("0.5");

    DoubleItemPtr item = doubleItemTest.getDoubleItem();
    smtkTest(
      item->setExpression(expressionAtt) == true, "Expected to set expression on the DoubleItem.")

      smtkTest(item->valueAsString() == "0.5", "Expected \"0.5\" from valueAsString().")

        expressionAtt->findString("text")
          ->setValue("abc123");

    smtkTest(item->valueAsString() == "CANNOT_EVALUATE",
      "Expected evaluation to fail, so valueAsString() will return"
      " \"CANNOT_EVALUATE\".")

      doubleItemTest.mp_attRes->evaluatorFactory()
        .unregisterEvaluator<StringToDoubleEvaluator>();
    smtkTest(item->valueAsString() == "CANNOT_EVALUATE",
      "StringToDoubleEvaluator is no longer registered, so valueAsString()"
      " will return \"CANNOT_EVALUATE\".")
  }
}

// Test XML reading and writing for evalutors by writing a Attribute Resource,
// reading it, then writing it again to get the same result.
void testEvaluatorXMLIO()
{
  DoubleItemTest doubleItemTest;
  doubleItemTest.SetUpDoubleItemTestExpressions();

  smtk::io::AttributeWriter writer;
  smtk::io::Logger log;
  std::string sbi1Contents;
  writer.writeContents(doubleItemTest.mp_attRes, sbi1Contents, log, false);

  smtk::io::AttributeReader reader;
  smtk::attribute::ResourcePtr importedResource = smtk::attribute::Resource::create();
  // Register the Evaluator in the same way as the test fixture.
  importedResource->evaluatorFactory().registerEvaluator<StringToDoubleEvaluator>(
    "StringToDoubleEvaluator");
  reader.readContents(importedResource, sbi1Contents, log);

  std::string sbi2Contents;
  writer.writeContents(importedResource, sbi2Contents, log, false);

  smtkTest(sbi1Contents == sbi2Contents, "Expected SBI exports to be equal.")
}

// Test JSON reading and writing for evalutors by writing a Attribute Resource,
// reading it, then writing it again to get the same result.
void testEvaluatorJSONIO()
{
  DoubleItemTest doubleItemTest;
  doubleItemTest.SetUpDoubleItemTestExpressions();

  nlohmann::json json1Contents;
  smtk::attribute::to_json(json1Contents, doubleItemTest.mp_attRes);

  smtk::attribute::ResourcePtr importedResource = smtk::attribute::Resource::create();
  // Register the Evaluator in the same way as the test fixture.
  importedResource->evaluatorFactory().registerEvaluator<StringToDoubleEvaluator>(
    "StringToDoubleEvaluator");
  smtk::attribute::from_json(json1Contents, importedResource);

  nlohmann::json json2Contents;
  smtk::attribute::to_json(json2Contents, importedResource);

  smtkTest(
    nlohmann::json::diff(json1Contents, json2Contents).empty(), "Expected SMTK export to be equal.")
}

int unitDoubleItem(int /*argc*/, char** const /*argv*/)
{
  smtk::io::Logger::instance().reset();
  testBasicGettingValue();
  smtk::io::Logger::instance().reset();

  testGettingValueWithExpression();
  testValueAsString();

  testEvaluatorXMLIO();
  testEvaluatorJSONIO();

  return 0;
}
