//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//=========================================================================

#include "smtk/extension/qt/qtAttributeTableModel.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/common/testing/cxx/helpers.h"

int UnitTestAttributeTableSharedColumn(int /*unused*/, char** const /*unused*/)
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

  auto first = resource->createAttribute("first", definition);
  auto second = resource->createAttribute("second", definition);
  auto firstSelector = first->findString("selector");
  auto secondSelector = second->findString("selector");
  firstSelector->setDiscreteIndex(0);
  secondSelector->setDiscreteIndex(1);
  first->itemAtPathAs<smtk::attribute::DoubleItem>("selector/aValue")->setValue(1.25);
  second->itemAtPathAs<smtk::attribute::DoubleItem>("selector/bValue")->setValue(2.5);

  smtk::extension::qtAttributeTableModel model;
  model.setAttributeResource(resource);
  model.setColumnDisplay(
    smtk::extension::qtAttributeTableModel::ColumnDisplay::All,
    {},
    { { "value", "Value", { "selector/aValue", "selector/bValue" } } });
  model.setDefinition(definition);
  model.setAttributes({ first, second });

  // Attribute name, controlling selector, and one shared value column. The
  // candidate children must not also appear as ordinary columns.
  smtkTest(model.columnCount() == 3, "Unexpected shared-column schema.");
  smtkTest(model.isSharedColumn(model.index(0, 2)), "The logical column is not shared.");
  smtkTest(
    model.data(model.index(0, 2), Qt::DisplayRole).toDouble() == 1.25,
    "The first row did not resolve aValue.");
  smtkTest(
    model.data(model.index(1, 2), Qt::DisplayRole).toDouble() == 2.5,
    "The second row did not resolve bValue.");

  smtkTest(
    model.setData(model.index(1, 2), 4.5, Qt::EditRole),
    "Editing the active shared-column candidate failed.");
  smtkTest(
    second->itemAtPathAs<smtk::attribute::DoubleItem>("selector/bValue")->value() == 4.5,
    "The shared-column edit targeted the wrong child.");

  firstSelector->setDiscreteIndex(2);
  model.refreshAttribute(first);
  smtkTest(
    model.data(model.index(0, 2), Qt::DisplayRole).toString() == QStringLiteral("—"),
    "A row with no active candidate did not display as inactive.");
  smtkTest(
    !(model.flags(model.index(0, 2)) & Qt::ItemIsEditable),
    "A row with no active candidate remained editable.");

  return 0;
}
