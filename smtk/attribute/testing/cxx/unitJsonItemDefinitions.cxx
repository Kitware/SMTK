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
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ValueItemDefinition.h"

#include "smtk/attribute/json/jsonComponentItemDefinition.h"
#include "smtk/attribute/json/jsonDoubleItemDefinition.h"
#include "smtk/attribute/json/jsonModelEntityItemDefinition.h"
#include "smtk/attribute/json/jsonReferenceItemDefinition.h"

#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "nlohmann/json.hpp"

using namespace smtk::attribute;
using json = nlohmann::json;

int unitJsonItemDefinitions(int /*unused*/, char** const /*unused*/)
{
  smtk::attribute::ResourcePtr resptr = smtk::attribute::Resource::create();
  smtk::attribute::Resource& resource(*resptr.get());

  smtk::attribute::DefinitionPtr def = resource.createDefinition("testDef");
  smtkTest(!!def, "Definition testDef not created.");

  /********************** ModelEntityItemDefinition ********************/
  smtk::attribute::ModelEntityItemDefinitionPtr meiDef = ModelEntityItemDefinition::New("mei-def");
  smtkTest(!!meiDef, "Failed to instantiate ModelEntityItemDefinition");
  smtkTest(
    meiDef->type() == Item::ModelEntityType,
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

  /********************** ReferenceItemDefinition ********************/
  smtk::attribute::ReferenceItemDefinitionPtr riDef = ReferenceItemDefinition::New("ri-def");
  smtkTest(!!riDef, "Failed to instantiate ReferenceItemDefinition");
  smtkTest(
    riDef->type() == Item::ReferenceType,
    "Failed to return ReferenceItemDefinition as definition type");
  riDef->setAcceptsEntries("smtk::model::Resource", "model", true);
  riDef->setNumberOfRequiredValues(2);
  riDef->setIsOptional(true);
  riDef->setIsExtensible(true);
  riDef->setMaxNumberOfValues(4);
  riDef->setValueLabel(0, "foo");
  riDef->setValueLabel(1, "bar");

  json riDefToJson = riDef;
  std::cout << "\nReferenceItem to_json result:\n" << riDefToJson.dump(2) << "\n\n";
  smtk::attribute::ReferenceItemDefinitionPtr riDef2 = ReferenceItemDefinition::New("ri-def");
  smtk::attribute::from_json(riDefToJson, riDef2, resptr);
  json riDefFromJson = riDef2;
  std::cout << "\nReferenceItem from_json result:\n" << riDefFromJson.dump(2) << "\n\n";

  test(riDefToJson == riDefFromJson, "Failed to serialize and deserialize ReferenceItemDef");

  /********************** ComponentItemDefinition ********************/
  smtk::attribute::ComponentItemDefinitionPtr compDef = ComponentItemDefinition::New("comp-def");
  compDef->setNumberOfRequiredValues(2);
  compDef->setIsOptional(true);
  compDef->setValueLabel(0, "foo");
  compDef->setValueLabel(1, "bar");
  // TODO: Test attribute definition
  json compDefToJson = compDef;
  //std::cout << " to_json result:\n" <<compDefToJson.dump(2) <<std::endl;
  smtk::attribute::ComponentItemDefinitionPtr compDef2 = ComponentItemDefinition::New("comp-def");
  smtk::attribute::from_json(compDefToJson, compDef2, resptr);
  json compDefFromJson = compDef2;
  //std::cout << " from_json result:\n" <<compDefFromJson.dump(2) <<std::endl;

  test(
    compDefToJson == compDefFromJson,
    "Failed to serialize and deserialize ComponentItemDefinition");

  /********************** DoubleItemDefinition ********************/
  smtk::attribute::DoubleItemDefinitionPtr doubleDef =
    smtk::attribute::DoubleItemDefinition::New("double-ref");
  // Default index is execlusive to default value
  doubleDef->addChildItemDefinition(compDef);
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
  smtk::attribute::from_json(doubleItemToJson, doubleDef2, resptr);
  json doubleItemFromJson = doubleDef2;
  std::cout << "DoubleItem from_jsom result:\n" << doubleItemFromJson.dump(2) << std::endl;
  test(
    doubleItemToJson == doubleItemFromJson,
    "Failed to serialize and deserialize "
    "DoubleItemDefinition");

  return 0;
}
