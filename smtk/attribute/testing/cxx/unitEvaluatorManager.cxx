//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/SharedFromThis.h"

#include "smtk/attribute/Evaluator.h"
#include "smtk/attribute/EvaluatorManager.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/plugin/Registry.h"

#include "smtk/resource/Manager.h"

class FooEvaluator : public smtk::attribute::Evaluator
{
public:
  smtkTypenameMacro(FooEvaluator)

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

  std::size_t numberOfEvaluatableElements() override { return 0; }
};

int unitEvaluatorManager(int /*argc*/, char** const /*argv*/)
{
  auto evaluatorManager = smtk::attribute::EvaluatorManager::create();
  auto resourceManager = smtk::resource::Manager::create();
  evaluatorManager->registerResourceManager(resourceManager);

  auto attributeRegistry = smtk::plugin::addToManagers<smtk::attribute::Registrar>(resourceManager);

  smtkTest(
    evaluatorManager->registerEvaluator<FooEvaluator>("FooEvaluator"),
    "Expected insert observer for Evaluator registration.")

    smtk::attribute::ResourcePtr managedAttRes =
      resourceManager->create<smtk::attribute::Resource>();
  resourceManager->add(managedAttRes);
  smtk::attribute::DefinitionPtr fooExpDef = managedAttRes->createDefinition("fooExpDef");

  smtkTest(
    managedAttRes->evaluatorFactory().addDefinitionForEvaluator(
      "FooEvaluator", fooExpDef->type()) == true,
    "Expected to add definition FooExpDef because managedAttRes should have had"
    "FooEvaluator registered upen creation with the Resource Manager.")

    //  smtk::attribute::AttributePtr fooExpAtt = managedAttRes->createAttribute(fooExpDef);

    //  smtkTest(managedAttRes->evaluatorFactory().createEvaluator(fooExpAtt) != nullptr, "Expected a nonnull Evaluator")

    smtkTest(
      evaluatorManager->unregisterEvaluator<FooEvaluator>(),
      "Expected FooEvaluator to unregister succesfully")

    //  smtkTest(managedAttRes->evaluatorFactory().createEvaluator(fooExpAtt) == nullptr,
    //           "Expected a null Evaluator after unregistering FooEvaluator")

    return 0;
}
