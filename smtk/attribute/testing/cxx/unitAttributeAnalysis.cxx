//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
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
void testLoadedAttributeResource(attribute::ResourcePtr& attRes, const std::string& prefix)
{
  attribute::Analyses& analyses = attRes->analyses();
  auto* a = analyses.find("a");
  auto* a1 = analyses.find("a1");
  auto* b = analyses.find("b");
  smtkTest((a != nullptr), prefix << "Could not find a!");
  smtkTest((a->isRequired()), prefix << "a is not Required!");
  smtkTest((a1 != nullptr), prefix << "Could not find a1!");
  smtkTest((!a1->isRequired()), prefix << "a1 is Required!");
  smtkTest((b != nullptr), prefix << "Could not find b!");
  // Lets test label method
  smtkTest(
    (a->displayedName() == "a"), prefix << "a's label() = " << a->label() << ", it should be a!");
  smtkTest(
    (a1->displayedName() == "A1 Test Label"),
    prefix << "a1's label() = " << a1->label() << ", it should be A1 Test Label!");
  smtkTest(
    (b->displayedName() == "b"), prefix << "b's label() = " << b->label() << ", it should be b!");

  smtkTest((a1->parent() == a), prefix << "a1's parent not a")
    smtkTest((b->parent() == nullptr), prefix << "b has a parent")
}
} // namespace

int unitAttributeAnalysis(int /*unused*/, char* /*unused*/[])
{
  // ----
  // I. Let's create an attribute resource and some analyses
  attribute::ResourcePtr attRes = attribute::Resource::create();
  attribute::Analyses& analyses = attRes->analyses();
  std::set<std::string> cats;
  cats.insert("foo");
  auto* analysis = analyses.create("a");
  // Set a to be required
  analysis->setRequired(true);
  analysis->setLocalCategories(cats);
  analysis = analyses.create("b");
  analysis->setLocalCategories(cats);
  analysis = analyses.create("c");
  analysis->setLocalCategories(cats);
  analysis = analyses.create("a1");
  analysis->setLocalCategories(cats);
  // Give A1 a Label
  analysis->setLabel("A1 Test Label");

  // We shouldn't be able to create 2 analyses with the same name
  analysis = analyses.create("a1");
  smtkTest((analysis == nullptr), "Was able to create 2 analyses named a1");

  // Lets try to set the parent to something that doesn't exists
  smtkTest(!(analyses.setAnalysisParent("z", "x")), "Succeded in setting z's parent to x");
  smtkTest(!(analyses.setAnalysisParent("z", "a")), "Succeded in setting z's parent to a");
  smtkTest(!(analyses.setAnalysisParent("a", "x")), "Succeded in setting a's parent to x");
  // This should work
  smtkTest(analyses.setAnalysisParent("a1", "a"), "Was not able to set a1's parent to a");
  // Lets test out the attribute definition building method
  // Definition should have 1 group (a), and 2 void (b, c) items. group a should have 1 void item (a1)
  auto def = analyses.buildAnalysesDefinition(attRes, "Analysis");
  smtkTest((def != nullptr), "Could not build definition");
  smtkTest((def->numberOfItemDefinitions() == 3), "Definition does not have 3 items") int pos =
    def->findItemPosition("a");
  smtkTest(pos > -1, "Could not find item a");
  auto gitem =
    std::dynamic_pointer_cast<smtk::attribute::GroupItemDefinition>(def->itemDefinition(pos));
  smtkTest((gitem != nullptr), "a is not a group item");
  smtkTest((!gitem->isOptional()), "a's group item is optional (aka a is not Required");
  pos = def->findItemPosition("b");
  smtkTest(pos > -1, "Could not find item b");
  auto vitem =
    std::dynamic_pointer_cast<smtk::attribute::VoidItemDefinition>(def->itemDefinition(pos));
  smtkTest((vitem != nullptr), "b is not a void item");
  pos = def->findItemPosition("c");
  smtkTest(pos > -1, "Could not find item c");
  vitem = std::dynamic_pointer_cast<smtk::attribute::VoidItemDefinition>(def->itemDefinition(pos));
  smtkTest((vitem != nullptr), "c is not a void item");
  pos = def->findItemPosition("a1");
  smtkTest(pos == -1, "Could find item a1 in Definition");
  smtkTest((gitem->numberOfItemDefinitions() == 1), "a's group item does not have 1 item") pos =
    gitem->findItemPosition("a1");
  smtkTest(pos == 0, "Could not find item a1 in group item a");
  vitem =
    std::dynamic_pointer_cast<smtk::attribute::VoidItemDefinition>(gitem->itemDefinition(pos));
  smtkTest((vitem != nullptr), "a1 is not a void item");
  // Should fail if we try to create another with the same type name
  def = analyses.buildAnalysesDefinition(attRes, "Analysis");
  smtkTest((def == nullptr), "Was able to build second definition");

  def = nullptr;
  vitem = nullptr;
  gitem = nullptr;

  testLoadedAttributeResource(attRes, "Analysis Test (Original)");
  io::AttributeWriter writer;
  io::AttributeReader reader;
  io::Logger logger;
  std::string writeRroot(SMTK_SCRATCH_DIR);
  std::string fname = writeRroot + "/unitAttributeAnalysisTest.sbi";
  std::string rname = writeRroot + "/unitAttributeAnalysisTest.smtk";

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
    opresult->findResource("resourcesCreated")->value());
  //Test the resource created using JSON
  testLoadedAttributeResource(attRes, "Analysis Test (JSON)");

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
  testLoadedAttributeResource(attRes, "Analysis Test (XML)");

  return 0;
}
