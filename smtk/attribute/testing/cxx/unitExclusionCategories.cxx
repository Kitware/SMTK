//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/VoidItemDefinition.h"
#include "smtk/attribute/operators/Read.h"
#include "smtk/attribute/operators/Write.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::attribute;
using namespace smtk::common;
using namespace smtk;

namespace
{

bool testCategories(
  const attribute::ResourcePtr& attRes,
  const std::string& prefix,
  const std::vector<std::pair<std::set<std::string>, std::vector<bool>>>& answers)
{
  std::cerr << prefix << " Testing Attribute Definition's Item Categories:\n";
  smtk::attribute::DefinitionPtr def = attRes->findDefinition("A");
  std::size_t i, n = def->numberOfItemDefinitions();
  bool status = true;
  int count = 0;
  for (auto test : answers)
  {
    bool configPass = true;
    std::cerr << "\t Testing Configuration: " << count;
    for (i = 0; i < n; i++)
    {
      auto idef = def->itemDefinition(static_cast<int>(i));
      if (idef->localCategories().passes(test.first) != test.second[i])
      {
        std::cerr << " " << i++;
        configPass = false;
      }
    }
    if (configPass)
    {
      std::cerr << " - Passed!\n";
    }
    else
    {
      std::cerr << " - Failed!\n";
      status = false;
    }
  }
  return status;
}

void setupAttributeResource(attribute::ResourcePtr& attRes)
{
  // So we are going to create a bunch of items that will test out the various combinations
  // of (a op1 b) op2 !(c op3 d) - where opx is either And or Or.  The items will be as follows:
  // 0: And And And
  // 1: And And Or
  // 2: And Or And
  // 3: And Or Or
  // 4: Or And And
  // 5: Or And Or
  // 6: Or Or And
  // 7: Or Or Or

  DefinitionPtr A = attRes->createDefinition("A");
  Categories::Expression testCats;
  testCats.insertInclusion("a");
  testCats.insertInclusion("b");
  testCats.insertExclusion("c");
  testCats.insertExclusion("d");

  StringItemDefinitionPtr sItemDef = A->addItemDefinition<StringItemDefinition>("s0");
  testCats.setInclusionMode(Categories::CombinationMode::And);
  testCats.setCombinationMode(Categories::CombinationMode::And);
  testCats.setExclusionMode(Categories::CombinationMode::And);
  sItemDef->localCategories() = testCats;

  sItemDef = A->addItemDefinition<StringItemDefinition>("s1");
  testCats.setInclusionMode(Categories::CombinationMode::And);
  testCats.setCombinationMode(Categories::CombinationMode::And);
  testCats.setExclusionMode(Categories::CombinationMode::Or);
  sItemDef->localCategories() = testCats;

  sItemDef = A->addItemDefinition<StringItemDefinition>("s2");
  testCats.setInclusionMode(Categories::CombinationMode::And);
  testCats.setCombinationMode(Categories::CombinationMode::Or);
  testCats.setExclusionMode(Categories::CombinationMode::And);
  sItemDef->localCategories() = testCats;

  sItemDef = A->addItemDefinition<StringItemDefinition>("s3");
  testCats.setInclusionMode(Categories::CombinationMode::And);
  testCats.setCombinationMode(Categories::CombinationMode::Or);
  testCats.setExclusionMode(Categories::CombinationMode::Or);
  sItemDef->localCategories() = testCats;

  sItemDef = A->addItemDefinition<StringItemDefinition>("s4");
  testCats.setInclusionMode(Categories::CombinationMode::Or);
  testCats.setCombinationMode(Categories::CombinationMode::And);
  testCats.setExclusionMode(Categories::CombinationMode::And);
  sItemDef->localCategories() = testCats;

  sItemDef = A->addItemDefinition<StringItemDefinition>("s5");
  testCats.setInclusionMode(Categories::CombinationMode::Or);
  testCats.setCombinationMode(Categories::CombinationMode::And);
  testCats.setExclusionMode(Categories::CombinationMode::Or);
  sItemDef->localCategories() = testCats;

  sItemDef = A->addItemDefinition<StringItemDefinition>("s6");
  testCats.setInclusionMode(Categories::CombinationMode::Or);
  testCats.setCombinationMode(Categories::CombinationMode::Or);
  testCats.setExclusionMode(Categories::CombinationMode::And);
  sItemDef->localCategories() = testCats;

  sItemDef = A->addItemDefinition<StringItemDefinition>("s7");
  testCats.setInclusionMode(Categories::CombinationMode::Or);
  testCats.setCombinationMode(Categories::CombinationMode::Or);
  testCats.setExclusionMode(Categories::CombinationMode::Or);
  sItemDef->localCategories() = testCats;
  attRes->finalizeDefinitions();
}
} // namespace

int unitExclusionCategories(int /*unused*/, char* /*unused*/[])
{
  //
  // I. Let's create an attribute resource and some definitions
  attribute::ResourcePtr attRes = attribute::Resource::create();
  setupAttributeResource(attRes);

  // Lets define what expected results will be for each item in the attribute
  std::vector<std::pair<std::set<std::string>, std::vector<bool>>> answers = {
    { { "a" }, { false, false, true, true, true, true, true, true } },
    { { "b" }, { false, false, true, true, true, true, true, true } },
    { { "c" }, { false, false, true, false, false, false, true, false } },
    { { "d" }, { false, false, true, false, false, false, true, false } },
    { { "a", "b" }, { true, true, true, true, true, true, true, true } },
    { { "a", "c" }, { false, false, true, false, true, false, true, true } },
    { { "a", "d" }, { false, false, true, false, true, false, true, true } },
    { { "b", "c" }, { false, false, true, false, true, false, true, true } },
    { { "b", "d" }, { false, false, true, false, true, false, true, true } },
    { { "c", "d" }, { false, false, false, false, false, false, false, false } },
    { { "a", "b", "c" }, { true, false, true, true, true, false, true, true } },
    { { "a", "b", "d" }, { true, false, true, true, true, false, true, true } },
    { { "a", "c", "d" }, { false, false, false, false, false, false, true, true } },
    { { "b", "c", "d" }, { false, false, false, false, false, false, true, true } },
    { { "a", "b", "c", "d" }, { false, false, true, true, false, false, true, true } }
  };
  smtkTest(
    testCategories(attRes, "First Pass - ", answers), "Failed checking Categories in First Pass");
  io::AttributeWriter writer;
  io::AttributeReader reader;
  io::Logger logger;
  std::string writeRroot(SMTK_SCRATCH_DIR);
  std::string fname = writeRroot + "/unitAttributeExclusionCategoriesTest.sbi";
  std::string rname = writeRroot + "/unitAttributeExclusionCategoriesTest.smtk";

  //Test JSON File I/O
  attRes->setLocation(rname);
  smtk::attribute::Write::Ptr writeOp = smtk::attribute::Write::create();
  writeOp->parameters()->associate(attRes);
  auto opresult = writeOp->operate();

  smtkTest(
    opresult->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "JSON Write operation failed\n"
      << writeOp->log().convertToString());
  attRes = nullptr;
  smtk::attribute::Read::Ptr readOp = smtk::attribute::Read::create();
  readOp->parameters()->findFile("filename")->setValue(rname);
  opresult = readOp->operate();
  smtkTest(
    opresult->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "JSON Read operation failed\n"
      << writeOp->log().convertToString());
  attRes = std::dynamic_pointer_cast<smtk::attribute::Resource>(
    opresult->findResource("resource")->value());
  //Test the resource created using JSON
  smtkTest(
    testCategories(attRes, "JSON Pass - ", answers), "Failed checking Categories in JSON Pass");

  //Test XML File I/O
  writer.write(attRes, fname, logger);
  smtkTest(
    !logger.hasErrors(),
    "Error Generated when XML writing file (" << fname << "):\n"
                                              << logger.convertToString());

  attRes = attribute::Resource::create();
  reader.read(attRes, fname, logger);
  smtkTest(
    !logger.hasErrors(),
    "Error Generated when XML reading file (" << fname << "):\n"
                                              << logger.convertToString());
  //Test the resource created using XML
  smtkTest(
    testCategories(attRes, "XML Pass - ", answers), "Failed checking Categories in XML Pass");

  return 0;
}
