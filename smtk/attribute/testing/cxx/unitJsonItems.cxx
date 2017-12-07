//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/VoidItemDefinition.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/json/jsonDirectoryItem.h"
#include "smtk/attribute/json/jsonItem.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "json.hpp"

using namespace smtk::attribute;
using json = nlohmann::json;

int unitJsonItems(int, char** const)
{
  // Reference childrenItemsTest
  smtk::attribute::CollectionPtr sysptr = smtk::attribute::Collection::create();
  smtk::attribute::Collection& collection(*sysptr.get());
  std::cout << "Collection Created\n";

  // Lets create an attribute to represent an expression
  smtk::attribute::DefinitionPtr expDef = collection.createDefinition("ExpDef");
  expDef->setBriefDescription("Sample Expression");
  expDef->setDetailedDescription("Sample Expression for testing\nThere is not much here!");
  // StringItemDefinition
  smtk::attribute::StringItemDefinitionPtr eitemdef =
    expDef->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("Expression String");
  eitemdef->setDefaultValue("sample");
  eitemdef->setIsOptional(true);
  smtk::attribute::StringItemDefinitionPtr eitemdef2 =
    expDef->addItemDefinition<smtk::attribute::StringItemDefinition>("Aux String");
  eitemdef2->setDefaultValue("sample2");
  // DirectoryItemDefinition
  smtk::attribute::DirectoryItemDefinitionPtr dirDef =
    expDef->addItemDefinition<smtk::attribute::DirectoryItemDefinition>("Directory Item");
  dirDef->setIsOptional(true);

  smtk::attribute::DefinitionPtr base = collection.createDefinition("BaseDef");
  // Lets add some item definitions
  smtk::attribute::IntItemDefinitionPtr iitemdef =
    base->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("TEMPORAL");
  iitemdef->setLabel("Time");
  iitemdef->addDiscreteValue(0, "Seconds");
  iitemdef->addDiscreteValue(1, "Minutes");
  iitemdef->addDiscreteValue(2, "Hours");
  iitemdef->addDiscreteValue(3, "Days");
  iitemdef->setDefaultDiscreteIndex(0);
  iitemdef->addCategory("Time");
  iitemdef = base->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("IntItem2");
  iitemdef->setDefaultValue(10);
  iitemdef->addCategory("Heat");

  // Lets test creating an attribute by passing in the expression definition explicitly
  smtk::attribute::AttributePtr expAtt1 = collection.createAttribute("Exp1", expDef);

  /**********************       item          ********************/
  smtk::attribute::ItemPtr expressionString1 = expAtt1->find("Expression String");
  json jsonExpSToJ = expressionString1;
  expressionString1->setIsEnabled(true);
  json jsonExpSToJFalse = expressionString1;
  smtk::attribute::from_json(jsonExpSToJ, expressionString1);
  json jsonExpSFromJ = expressionString1;
  // After deserialization, json should match the orignal state and differ with the changed state
  test(jsonExpSFromJ == jsonExpSToJ, "Failed to serialize and deserialize "
                                     "item");
  test(jsonExpSFromJ != jsonExpSToJFalse, "Failed to serialize and deserialize "
                                          "item");

  // TODO: Add unit test for other items
  /**********************       directoryItem          ********************/
  smtk::attribute::DirectoryItemPtr dir =
    smtk::dynamic_pointer_cast<smtk::attribute::DirectoryItem>(expAtt1->find("Directory Item"));
  json DirToJ = dir;
  dir->setIsEnabled(true);
  json DirFromJ = dir;
  smtk::attribute::from_json(DirFromJ, dir);
  json DirToJModified = dir;
  // After deserialization, json should match the orignal state and differ with the changed state
  test(DirFromJ == DirToJModified, "Failed to serialize and deserialize "
                                   "DirectoryItem");
  test(DirToJ != DirToJModified, "Failed to serialize and deserialize "
                                 "DirectoryItem");

  return 0;
}
