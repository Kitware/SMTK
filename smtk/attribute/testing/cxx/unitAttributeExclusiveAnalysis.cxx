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
void testLoadedAttributeResource(attribute::ResourcePtr& attRes, const std::string& prefix)
{
  attribute::Analyses& analyses = attRes->analyses();
  auto* a = analyses.find("a");
  auto* a1 = analyses.find("a1");
  auto* b = analyses.find("b");
  smtkTest((analyses.areTopLevelExclusive()), prefix << "Toplevel analyses are not exclusive!");
  smtkTest((a != nullptr), prefix << "Could not find a!");
  smtkTest((a1 != nullptr), prefix << "Could not find a1!");
  smtkTest((b != nullptr), prefix << "Could not find b!");
  smtkTest((a1->parent() == a), prefix << "a1's parent not a")
    smtkTest((b->parent() == nullptr), prefix << "b has a parent")
}
} // namespace

int unitAttributeExclusiveAnalysis(int /*unused*/, char* /*unused*/[])
{
  // ----
  // I. Let's create an attribute resource and some analyses
  attribute::ResourcePtr attRes = attribute::Resource::create();
  attribute::Analyses& analyses = attRes->analyses();
  // Make the top level to be exclusive
  analyses.setTopLevelExclusive(true);
  std::set<std::string> cats;
  cats.insert("foo");
  auto* analysis = analyses.create("a");
  analysis->setLocalCategories(cats);
  analysis = analyses.create("b");
  analysis->setLocalCategories(cats);
  analysis = analyses.create("c");
  // Lets make c's children exclusive
  analysis->setExclusive(true);
  analysis->setLocalCategories(cats);
  analysis = analyses.create("a1");
  analysis->setLocalCategories(cats);
  analysis = analyses.create("c1");
  analysis->setLocalCategories(cats);
  analysis = analyses.create("c2");
  analysis->setLocalCategories(cats);

  // We shouldn't be able to create 2 analyses with the same name
  analysis = analyses.create("a1");
  smtkTest((analysis == nullptr), "Was able to create 2 analyses named a1");

  // Lets try to set the parent to something that doesn't exists
  smtkTest(!(analyses.setAnalysisParent("z", "x")), "Succeded in setting z's parent to x");
  smtkTest(!(analyses.setAnalysisParent("z", "a")), "Succeded in setting z's parent to a");
  smtkTest(!(analyses.setAnalysisParent("a", "x")), "Succeded in setting a's parent to x");
  // These should work
  smtkTest(analyses.setAnalysisParent("a1", "a"), "Was not able to set a1's parent to a");
  smtkTest(analyses.setAnalysisParent("c1", "c"), "Was not able to set c1's parent to c");
  smtkTest(analyses.setAnalysisParent("c2", "c"), "Was not able to set c2's parent to c");
  // Lets test out the attribute definition building method
  // Definition should have 1 group (a), and 2 void (b, c) items. group a should have 1 void item (a1)
  auto def = analyses.buildAnalysesDefinition(attRes, "Analysis");
  smtkTest((def != nullptr), "Could not build definition");
  smtkTest((def->numberOfItemDefinitions() == 1), "Definition does not have 1 items");
  auto sitem =
    std::dynamic_pointer_cast<smtk::attribute::StringItemDefinition>(def->itemDefinition(0));
  smtkTest((sitem != nullptr), "Could not find top level string item");
  smtkTest((sitem->isDiscrete()), "Top level Item is not discrete");
  std::cerr << "Number of Top Level Analyses = " << sitem->numberOfDiscreteValues() << std::endl;
  std::cerr << "Names of Top Level Analyses are :";
  for (std::size_t i = 0; i < sitem->numberOfDiscreteValues(); ++i)
  {
    std::cerr << sitem->discreteEnum(i) << " ";
  }
  std::cerr << std::endl;

  smtkTest(
    (sitem->numberOfDiscreteValues() == 3),
    "Expected 3 top level analyses but found " << sitem->numberOfDiscreteValues());
  smtkTest((sitem->discreteEnum(0) == "a"), "First Analysis is not a");
  smtkTest((sitem->discreteEnum(1) == "b"), "Second Analysis is not b");
  smtkTest((sitem->discreteEnum(2) == "c"), "Third Analysis is not c");
  std::cerr << "Number of Top Level Children Definitions = "
            << sitem->numberOfChildrenItemDefinitions() << std::endl;
  smtkTest(
    (sitem->numberOfChildrenItemDefinitions() == 2),
    "Top level Item does not have 2 children item definitions");
  auto childrenDefs = sitem->childrenItemDefinitions();
  auto it = childrenDefs.find("a");
  smtkTest((it != childrenDefs.end()), "Could not find a's definition");
  auto gitem = std::dynamic_pointer_cast<smtk::attribute::GroupItemDefinition>(it->second);
  smtkTest((gitem != nullptr), "a is not a group item");

  smtkTest((gitem->numberOfItemDefinitions() == 1), "a's group item does not have 1 item");
  auto pos = gitem->findItemPosition("a1");
  smtkTest(pos == 0, "Could not find item a1 in group item a");
  auto vitem =
    std::dynamic_pointer_cast<smtk::attribute::VoidItemDefinition>(gitem->itemDefinition(pos));
  smtkTest((vitem != nullptr), "a1 is not a void item");

  it = childrenDefs.find("c");
  smtkTest((it != childrenDefs.end()), "Could not find c's definition");
  sitem = std::dynamic_pointer_cast<smtk::attribute::StringItemDefinition>(it->second);
  smtkTest((sitem != nullptr), "c is not a string item");
  std::cerr << "Number of c's children analyses = " << sitem->numberOfDiscreteValues() << std::endl;
  std::cerr << "Names of c's children analyses are :";
  for (std::size_t i = 0; i < sitem->numberOfDiscreteValues(); ++i)
  {
    std::cerr << sitem->discreteEnum(i) << " ";
  }
  std::cerr << std::endl;

  smtkTest((sitem->numberOfDiscreteValues() == 2), "c's item does not have 2 analyses");
  smtkTest((sitem->discreteEnum(0) == "c1"), "c's first analysis is not c1");
  smtkTest((sitem->discreteEnum(1) == "c2"), "c's second analysis is not c2");

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
  std::string fname = writeRroot + "/unitAttributeExclusiveAnalysisTest.sbi";
  std::string rname = writeRroot + "/unitAttributeExclusiveAnalysisTest.smtk";

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
