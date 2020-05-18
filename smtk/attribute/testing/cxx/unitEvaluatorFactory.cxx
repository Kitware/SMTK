//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
/*!\file unitEvaluatorFactory.cxx - Unit tests for EvaluatorFactory as it is used in
    smtk::attribute::Resource. */

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Evaluator.h"
#include "smtk/attribute/EvaluatorFactory.h"
#include "smtk/attribute/Resource.h"

#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <map>
#include <string>
#include <vector>

class FooEvaluator : public smtk::attribute::Evaluator
{
public:
  FooEvaluator(smtk::attribute::ConstAttributePtr att)
    : Evaluator(att)
  {
  }

  bool canEvaluate(smtk::io::Logger& /*log*/) override { return true; }

  bool evaluate(
    ValueType& /*result*/,
    smtk::io::Logger& /*log*/,
    const std::size_t& /*element*/,
    const DependentEvaluationMode& /*evaluationMode*/) override
  {
    return true;
  }

  bool doesEvaluate(std::size_t /*element*/) override { return true; }
  bool doesEvaluate() override { return true; }

  size_t numberOfEvaluatableElements() override { return 0; }
};

class BarEvaluator : public smtk::attribute::Evaluator
{
public:
  BarEvaluator(smtk::attribute::ConstAttributePtr att)
    : Evaluator(att)
  {
  }

  bool canEvaluate(smtk::io::Logger& /*log*/) override { return true; }

  bool evaluate(
    ValueType& /*result*/,
    smtk::io::Logger& /*log*/,
    const std::size_t& /*element*/,
    const DependentEvaluationMode& /*evaluationMode*/) override
  {
    return true;
  }

  bool doesEvaluate(std::size_t /*element*/) override { return true; }
  bool doesEvaluate() override { return true; }

  size_t numberOfEvaluatableElements() override { return 0; }
};

void multipleDefinitionsForSameEvaluator()
{
  smtk::attribute::ResourcePtr attRes = smtk::attribute::Resource::create();

  smtk::attribute::DefinitionPtr fooDef1 = attRes->createDefinition("fooDef1");
  smtk::attribute::DefinitionPtr fooDef2 = attRes->createDefinition("fooDef2");

  smtk::attribute::EvaluatorFactory& factory = attRes->evaluatorFactory();

  factory.registerEvaluator<FooEvaluator>("FooEvaluator");
  smtkTest(
    factory.addDefinitionForEvaluator("FooEvaluator", fooDef1->type()) == true,
    "Expected to add Definition \"fooDef1\" for Evaluator \"FooEvaluator\".")
    smtkTest(
      factory.addDefinitionForEvaluator("FooEvaluator", fooDef2->type()) == true,
      "Expected to add Definition \"fooDef2 for Evaluator \"FooEvaluator\".")

      smtk::attribute::AttributePtr fooAtt1 = attRes->createAttribute(fooDef1);
  smtkTest(
    attRes->createEvaluator(fooAtt1) != nullptr,
    "Expected an Evaluator for \"fooAtt1\" because its Definition was associated"
    "to \"FooEvaluator\".")

    smtk::attribute::AttributePtr fooAtt2 = attRes->createAttribute(fooDef2);
  smtkTest(
    attRes->createEvaluator(fooAtt2) != nullptr,
    "Expected an Evaluator for \"fooAtt2\" because its Definition was associated"
    "to \"FooEvaluator\".")

    smtk::attribute::DefinitionPtr fooDef3 = attRes->createDefinition("fooDef3");
}

void doesNotCreateEvaluatorForUnknownDefinition()
{
  smtk::attribute::ResourcePtr attRes = smtk::attribute::Resource::create();
  smtk::attribute::DefinitionPtr fooDef = attRes->createDefinition("fooDef");

  smtk::attribute::AttributePtr fooAtt = attRes->createAttribute(fooDef);
  smtkTest(
    attRes->createEvaluator(fooAtt) == nullptr,
    "Expected null Evaluator for \"fooAtt\" because its Definition is not"
    "known to the factory.")
}

void derivedDefinitions()
{
  smtk::attribute::ResourcePtr attRes = smtk::attribute::Resource::create();

  smtk::attribute::DefinitionPtr baseDef = attRes->createDefinition("baseDef");
  smtk::attribute::DefinitionPtr derivedDef = attRes->createDefinition("derivedDef", baseDef);

  smtk::attribute::EvaluatorFactory& factory = attRes->evaluatorFactory();

  factory.registerEvaluator<FooEvaluator>("FooEvaluator");
  smtkTest(
    factory.addDefinitionForEvaluator("FooEvaluator", baseDef->type()) == true,
    "Expected to add Definition \"baseDef\" for Evaluator \"FooEvaluator\".")

    smtk::attribute::AttributePtr derivedAtt = attRes->createAttribute(derivedDef);
  smtkTest(
    attRes->createEvaluator(derivedAtt) != nullptr,
    "Expected an Evaluator for \"derivedAtt\" since it has Definition type \"derivedDef\","
    " which is derived from \"baseDef\". \"baseDef\" is known to the factory.")
}

void getsAliasesAndDefinitions()
{
  smtk::attribute::ResourcePtr attRes = smtk::attribute::Resource::create();

  smtk::attribute::EvaluatorFactory& factory = attRes->evaluatorFactory();
  factory.registerEvaluator<BarEvaluator>("BarEvaluator");
  factory.addDefinitionForEvaluator("BarEvaluator", "a");
  factory.addDefinitionForEvaluator("BarEvaluator", "b");
  factory.registerEvaluator<BarEvaluator>("FooEvaluator");
  factory.addDefinitionForEvaluator("FooEvaluator", "c");
  factory.addDefinitionForEvaluator("FooEvaluator", "d");

  std::map<std::string, std::vector<std::string>> expectedAliasesAndDefinitions;
  expectedAliasesAndDefinitions["BarEvaluator"] = { "a", "b" };
  expectedAliasesAndDefinitions["FooEvaluator"] = { "c", "d" };

  smtkTest(
    factory.aliasesToDefinitions() == expectedAliasesAndDefinitions,
    "Expected aliasesToDefinitions as above.")
}

void testDefinitionAlreadyRegistered()
{
  smtk::attribute::ResourcePtr attRes = smtk::attribute::Resource::create();

  smtk::attribute::EvaluatorFactory& factory = attRes->evaluatorFactory();
  factory.registerEvaluator<BarEvaluator>("BarEvaluator");
  factory.registerEvaluator<FooEvaluator>("FooEvaluator");

  smtkTest(
    factory.isDefinitionRegistered("abc") == false, "Expected \"abc\" to not be registered yet")

    smtkTest(
      factory.addDefinitionForEvaluator("BarEvaluator", "abc") == true,
      "Expected to add definition because \"abc\" is not registered yet.")
      smtkTest(
        factory.isDefinitionRegistered("abc") == true, "Expected \"abc\" to not be registered yet")
        smtkTest(
          factory.addDefinitionForEvaluator("FooEvaluator", "abc") == false,
          "\"abc\" should not register because it is already registered to \"BarEvaluator\".")

          smtkTest(
            factory.addDefinitionForEvaluator("BarEvaluator", "abc") == false,
            "Already registered \"abc\" to \"BarEvaluator\".")

            smtkTest(
              factory.isDefinitionRegistered("xyz") == false,
              "Expected \"xyz\" tp not have been registered.")
}

//void testUnregistersEvaluators()
//{
//  smtk::attribute::ResourcePtr attRes = smtk::attribute::Resource::create();

//  smtk::attribute::EvaluatorFactory& factory = attRes->evaluatorFactory();
//  factory.registerEvaluator<BarEvaluator>("BarEvaluator");

//  factory.registerEvaluator<FooEvaluator>("FooEvaluator");
//  smtk::attribute::DefinitionPtr fooDef = attRes->createDefinition("fooDef");
//  smtk::attribute::AttributePtr fooAtt = attRes->createAttribute(fooDef);
//  factory.addDefinitionForEvaluator("FooEvaluator", fooDef->type());

//  smtkTest(factory.unregisterEvaluator("BarEvaluator") == true,
//           "Expected to successfully unregister \"BarEvaluator\" by its alias.")
//      //factory.unregisterEvaluator<FooEvaluator>();
//  smtkTest(factory.createEvaluator(fooAtt) != nullptr,
//           "Expected to create an Evaluator for \"fooAtt\" since \"FooEvaluator\""
//           " is still registered to the factory.")
//  smtkTest(factory.unregisterEvaluator<FooEvaluator>() == true,
//           "Expected to successfully unregister \"FooEvaluator\" via template method.")

//  smtkTest(factory.createEvaluator(fooAtt) == nullptr,
//           "Expected to not create an Evaluator for \"fooAtt\" since we just unregistered"
//           " its Evaluator, \"FooEvaluator\".")
//}

int unitEvaluatorFactory(int /*argc*/, char** const /*argv*/)
{
  multipleDefinitionsForSameEvaluator();
  doesNotCreateEvaluatorForUnknownDefinition();
  derivedDefinitions();
  getsAliasesAndDefinitions();
  testDefinitionAlreadyRegistered();
  //testUnregistersEvaluators();

  return 0;
}
