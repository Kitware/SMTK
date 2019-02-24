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
  smtkTest((attRes->analysisParent("a1") == "a"), prefix << "a1's parent not a");
  smtkTest((attRes->analysisParent("b") == ""), prefix << "b has a parent");
}
}

int unitAttributeAnalysis(int, char* [])
{
  // ----
  // I. Let's create an attribute resource and some analyses
  attribute::ResourcePtr attRes = attribute::Resource::create();

  std::set<std::string> cats;
  cats.insert("foo");
  attRes->defineAnalysis("a", cats);
  attRes->defineAnalysis("b", cats);
  attRes->defineAnalysis("c", cats);
  attRes->defineAnalysis("a1", cats);

  // Lets try to set the parent to something that doesn't exists
  smtkTest(!(attRes->setAnalysisParent("z", "x")), "Succeded in setting z's parent to x");
  smtkTest(!(attRes->setAnalysisParent("z", "a")), "Succeded in setting z's parent to a");
  smtkTest(!(attRes->setAnalysisParent("a", "x")), "Succeded in setting a's parent to x");
  // This should work
  smtkTest(attRes->setAnalysisParent("a1", "a"), "Was not able to set a1's parent to a");
  // Lets test out the attribute definition building method
  // Definition should have 1 group (a), and 2 void (b, c) items. group a should have 1 void item (a1)
  auto def = attRes->buildAnalysesDefinition("Analysis");
  smtkTest((def != nullptr), "Could not build definition");
  smtkTest((def->numberOfItemDefinitions() == 3), "Definition does not have 3 items") int pos =
    def->findItemPosition("a");
  smtkTest(pos > -1, "Could not find item a");
  auto gitem =
    std::dynamic_pointer_cast<smtk::attribute::GroupItemDefinition>(def->itemDefinition(pos));
  smtkTest((gitem != nullptr), "a is not a group item");
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
  def = attRes->buildAnalysesDefinition("Analysis");
  smtkTest((def == nullptr), "CWas able to build second definition");

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

  smtkTest(opresult->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "JSON Write operation failed\n"
      << writeOp->log().convertToString());
  attRes = nullptr;
  smtk::attribute::Read::Ptr readOp = smtk::attribute::Read::create();
  readOp->parameters()->findFile("filename")->setValue(rname);
  opresult = readOp->operate();
  smtkTest(opresult->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "JSON Read operation failed\n"
      << writeOp->log().convertToString());
  attRes = std::dynamic_pointer_cast<smtk::attribute::Resource>(
    opresult->findResource("resource")->objectValue());
  //Test the resource created using JSON
  testLoadedAttributeResource(attRes, "Analysis Test (JSON)");

  //Test XML File I/O
  writer.write(attRes, fname, logger);
  smtkTest(!logger.hasErrors(), "Error Generated when XML writing file ("
      << fname << "):\n"
      << logger.convertToString());

  attRes = attribute::Resource::create();
  reader.read(attRes, fname, logger);
  smtkTest(!logger.hasErrors(), "Error Generated when XML reading file ("
      << fname << "):\n"
      << logger.convertToString());
  //Test the resource created using XML
  testLoadedAttributeResource(attRes, "Analysis Test (XML)");

  return 0;
}
