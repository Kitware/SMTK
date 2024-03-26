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

#include "units/System.h"

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

  bool evaluate(
    ValueType& result,
    smtk::io::Logger& log,
    const std::size_t& element,
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
        smtk::io::Logger::Error, "Text was not convertible to a floating-point representation.");
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

    mp_doubleItemDef1 = mp_def->addItemDefinition<DoubleItemDefinition>("testDoubleItem1");
    smtkTest(mp_doubleItemDef1->setUnits("cm"), "Could not set units to cm");
    smtkTest(mp_doubleItemDef1->setDefaultValue(10.0), "Could not set default to 10.0");
    smtkTest(mp_doubleItemDef1->defaultValue() == 10.0, "Default is not 10.0");
    smtkTest(
      mp_doubleItemDef1->setDefaultValueAsString("10.0 m"), "Could not set default to 10.0 m");
    smtkTest(
      mp_doubleItemDef1->defaultValueAsString() == "10.0 m", "Default as string was not 10.0 m");
    smtkTest(mp_doubleItemDef1->defaultValue() == 1000.0, "Default is not 1000.0");
    smtkTest(
      !mp_doubleItemDef1->setDefaultValueAsString("10.0 m/s"), "Was able to set default to 10 m/s");

    mp_doubleVecItemDef = DoubleItemDefinition::New("testDoubleVecItem");
    mp_doubleVecItemDef->setNumberOfRequiredValues(3);
    mp_def->addItemDefinition(mp_doubleVecItemDef);

    // Simple test with no units system
    auto ddef = DoubleItemDefinition::New("NoUnitsDef");
    smtkTest(ddef->setUnits("cm"), "Could not set units to cm");
    smtkTest(
      ddef->setDefaultValue(10.0), "Could not set def with no units system's default to 10.0");
    smtkTest(ddef->defaultValue() == 10.0, "Default is not 10.0 for def with no units system");
    smtkTest(
      !ddef->setDefaultValueAsString("10.0 m"),
      "Could set def with no units system's default to 10.0 m");
    smtkTest(
      ddef->setDefaultValueAsString("20.0"),
      "Could not set def with no units system's default to 20.0");
    smtkTest(ddef->defaultValue() == 20.0, "Default is not 20.0 for def with no units system");
    smtkTest(
      ddef->setDefaultValueAsString("30.0 cm"),
      "Could not set def with no units system's default to 30.0 cm");
    smtkTest(ddef->defaultValue() == 30.0, "Default is not 30.0 for def with no units system");
  }

  DoubleItemPtr getDoubleItem()
  {
    if (!mp_att)
    {
      mp_att = mp_attRes->createAttribute(mp_def);
    }
    return mp_att->findDouble("testDoubleItem");
  }

  DoubleItemPtr getDoubleItem1()
  {
    if (!mp_att)
    {
      mp_att = mp_attRes->createAttribute(mp_def);
    }
    return mp_att->findDouble("testDoubleItem1");
  }

  DoubleItemPtr getDoubleVecItem()
  {
    if (!mp_att)
    {
      mp_att = mp_attRes->createAttribute(mp_def);
    }
    auto item = mp_att->findDouble("testDoubleVecItem");
    // Is the vector initialized?
    if (!item->isValid())
    {
      item->setValue(0, 0);
      item->setValue(1, 1);
      item->setValue(2, 2);
    }
    return item;
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
  AttributePtr mp_att;
  DefinitionPtr mp_def;
  DoubleItemDefinitionPtr mp_doubleItemDef;
  DoubleItemDefinitionPtr mp_doubleItemDef1;
  DoubleItemDefinitionPtr mp_doubleVecItemDef;

  DefinitionPtr mp_evaluatorDef;
};

void testBasicGettingValue()
{
  DoubleItemTest doubleItemTest;

  DoubleItemPtr item = doubleItemTest.getDoubleItem();

  smtkTest(item->value() == 0.0, "Expected an unset value to be zero-initialized.") smtkTest(
    smtk::io::Logger::instance().hasErrors() == true,
    "Expected the global logger to have an error.")

    smtk::io::Logger::instance()
      .reset();
  item->setValue(5.0);

  smtkTest(item->value() == 5.0, "Expected value to be 5.0") smtkTest(
    smtk::io::Logger::instance().hasErrors() == false, "Expected global logger to have no errors.")

    // Lets test an item with units
    DoubleItemPtr item1 = doubleItemTest.getDoubleItem1();
  smtkTest(item1->value() == 1000.0, "Expected default to be 1000.0 cm.");
  smtkTest(item1->setValue(20.0), "Could not set value to 20.0 cm.");
  smtkTest(item1->value() == 20.0, "Expected value to be 20.0 cm.");
  smtkTest(item1->setValueFromString(0, "150 mm"), "Could not set value to 150.0 mm.");
  smtkTest(item1->value() == 15.0, "Expected value to be 15.0 cm.");
  smtkTest(item1->valueAsString() == "150 mm", "Expected value to be 150 mm.");
  smtkTest(!item1->setValueFromString(0, "150 cm/s"), "Could set value to 150 cm/s");

  // Test Iterator Interface
  item = doubleItemTest.getDoubleVecItem();
  double answer = 0;
  for (auto it = item->begin(); it != item->end(); ++it)
  {
    smtkTest(*it == answer, "Expected iterator value: " << answer << " but found: " << *it);
    ++answer;
  }
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
  smtkTest(
    item->value(0, log) == 5.0,
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

    smtkTest(
      item->valueAsString() == "CANNOT_EVALUATE",
      "Expected evaluation to fail, so valueAsString() will return"
      " \"CANNOT_EVALUATE\".")

      doubleItemTest.mp_attRes->evaluatorFactory()
        .unregisterEvaluator<StringToDoubleEvaluator>();
    smtkTest(
      item->valueAsString() == "CANNOT_EVALUATE",
      "StringToDoubleEvaluator is no longer registered, so valueAsString()"
      " will return \"CANNOT_EVALUATE\".")
  }
}

// Test XML reading and writing for evaluators by writing a Attribute Resource,
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
