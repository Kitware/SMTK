//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//=========================================================================

#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/utility/Queries.h"

#include "smtk/common/testing/cxx/helpers.h"

int unitExclusiveConditionalItems(int /*unused*/, char** const /*unused*/)
{
  auto resource = smtk::attribute::Resource::create();
  auto definition = resource->createDefinition("Test");

  auto selector =
    definition->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("selector");
  selector->addDiscreteValue("a", "A");
  selector->addDiscreteValue("b", "B");
  selector->addDiscreteValue("none", "None");
  selector->addItemDefinition<smtk::attribute::DoubleItemDefinitionPtr>("aValue");
  selector->addItemDefinition<smtk::attribute::DoubleItemDefinitionPtr>("bValue");
  selector->addConditionalItem("A", "aValue");
  selector->addConditionalItem("B", "bValue");

  auto result = smtk::attribute::utility::validateExclusiveConditionalItems(
    definition, { "selector/aValue", "selector/bValue" });
  smtkTest(result.valid(), result.error);
  smtkTest(result.controllingDefinition == selector, "Incorrect controlling definition.");
  smtkTest(result.controllingPath == "selector", "Incorrect controlling path.");
  smtkTest(result.itemDefinitions.size() == 2, "Incorrect number of resolved definitions.");

  result = smtk::attribute::utility::validateExclusiveConditionalItems(
    definition, { "selector/aValue", "missing/value" });
  smtkTest(!result.valid(), "An unresolved path was accepted.");

  auto overlapping =
    definition->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("overlapping");
  overlapping->addDiscreteValue("both", "Both");
  overlapping->addItemDefinition<smtk::attribute::DoubleItemDefinitionPtr>("first");
  overlapping->addItemDefinition<smtk::attribute::DoubleItemDefinitionPtr>("second");
  overlapping->addConditionalItem("Both", "first");
  overlapping->addConditionalItem("Both", "second");

  result = smtk::attribute::utility::validateExclusiveConditionalItems(
    definition, { "overlapping/first", "overlapping/second" });
  smtkTest(!result.valid(), "Simultaneously active children were accepted.");

  result = smtk::attribute::utility::validateExclusiveConditionalItems(
    definition, { "selector/aValue", "overlapping/first" });
  smtkTest(!result.valid(), "Children with different controlling parents were accepted.");

  return 0;
}
