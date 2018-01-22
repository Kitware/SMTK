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
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/ValueItemDefinition.h"

#include "smtk/attribute/json/jsonDoubleItemDefinition.h"
#include "smtk/attribute/json/jsonModelEntityItemDefinition.h"
#include "smtk/attribute/json/jsonRefItemDefinition.h"

#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "nlohmann/json.hpp"

using namespace smtk::attribute;
using json = nlohmann::json;

int unitJsonItemDefinitions(int, char** const)
{
  using ItemExpressionDefInfo = std::pair<smtk::attribute::ValueItemDefinitionPtr, std::string>;

  using AttRefDefInfo = std::pair<smtk::attribute::RefItemDefinitionPtr, std::string>;

  smtk::attribute::CollectionPtr collptr = smtk::attribute::Collection::create();
  smtk::attribute::Collection& collection(*collptr.get());

  smtk::attribute::DefinitionPtr def = collection.createDefinition("testDef");
  smtkTest(!!def, "Definition testDef not created.");

  std::vector<ItemExpressionDefInfo> expressDefInfo;
  std::vector<AttRefDefInfo> refDefInfo;

  /********************** ModelEntityItemDefinition ********************/
  smtk::attribute::ModelEntityItemDefinitionPtr meiDef = ModelEntityItemDefinition::New("mei-def");
  smtkTest(!!meiDef, "Failed to instantiate ModelEntityItemDefinition");
  smtkTest(meiDef->type() == Item::ModelEntityType,
    "Failed to return ModelEntityItemDefinition as definition type");
  meiDef->setMembershipMask(smtk::model::MODEL_ENTITY);
  meiDef->setNumberOfRequiredValues(2);
  meiDef->setIsOptional(true);
  meiDef->setIsExtensible(true);
  meiDef->setMaxNumberOfValues(4);
  meiDef->setValueLabel(0, "foo");
  meiDef->setValueLabel(1, "bar");

  json meiDefToJson = meiDef;
  smtk::attribute::ModelEntityItemDefinitionPtr meiDef2 = ModelEntityItemDefinition::New("mei-def");
  smtk::attribute::from_json(meiDefToJson, meiDef2);
  json meiDefFromJson = meiDef2;

  test(meiDefToJson == meiDefFromJson, "Failed to serialize and deserialize ModelEntityItemDef");

  /********************** RefItemDefinition ********************/
  smtk::attribute::RefItemDefinitionPtr refDef = RefItemDefinition::New("ref-def");
  refDef->setNumberOfRequiredValues(2);
  refDef->setIsOptional(true);
  refDef->setValueLabel(0, "foo");
  refDef->setValueLabel(1, "bar");
  // TODO: Test attribute definition
  json refDefToJson = refDef;
  //std::cout << " to_json result:\n" <<refDefToJson.dump(2) <<std::endl;
  smtk::attribute::RefItemDefinitionPtr refDef2 = RefItemDefinition::New("ref-def");
  smtk::attribute::from_json(refDefToJson, refDef2, collptr, refDefInfo);
  json refDefFromJson = refDef2;
  //std::cout << " from_json result:\n" <<refDefFromJson.dump(2) <<std::endl;

  test(refDefToJson == refDefFromJson, "Failed to serialize and deserialize RefItemDefinition");

  /********************** DoubleItemDefinition ********************/
  smtk::attribute::DoubleItemDefinitionPtr doubleDef =
    smtk::attribute::DoubleItemDefinition::New("double-ref");
  // Default index is execlusive to default value
  doubleDef->addChildItemDefinition(refDef);
  /*******************************************/
  doubleDef->addChildItemDefinition(meiDef2);
  /*******************************************/
  doubleDef->addDiscreteValue(0.0, "zero");
  bool addConditionalItemResult = doubleDef->addConditionalItem("zero", "ref-def");
  std::cout << "addConditionalItemResult: " << addConditionalItemResult << std::endl;
  doubleDef->addDiscreteValue(1.0, "one");
  doubleDef->setDefaultDiscreteIndex(0);
  doubleDef->setExpressionDefinition(def);

  //  doubleDef->setMinRange(0, true);
  //  doubleDef->setMaxRange(42, true);
  //  doubleDef->setDefaultValue(1);

  json doubleItemToJson = doubleDef;
  std::cout << "DoubleItem to_json result:\n" << doubleItemToJson.dump(2) << std::endl;
  smtk::attribute::DoubleItemDefinitionPtr doubleDef2 =
    smtk::attribute::DoubleItemDefinition::New("double-ref");
  smtk::attribute::from_json(doubleItemToJson, doubleDef2, collptr, expressDefInfo, refDefInfo);
  json doubleItemFromJson = doubleDef2;
  std::cout << "DoubleItem from_jsom result:\n" << doubleItemFromJson.dump(2) << std::endl;
  test(doubleItemToJson == doubleItemFromJson, "Failed to serialize and deserialize "
                                               "DoubleItemDefinition");

  return 0;
}
