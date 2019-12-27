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
  const AttributePtr& att, const ItemPtr s[], const std::set<std::string>& cats, bool result[])
{
  bool status = true;
  if (att->categories().passes(cats) == result[0])
  {
    std::cerr << att->name() << ":passed ";
  }
  else
  {
    std::cerr << att->name() << ":failed ";
    status = false;
  }
  for (int i = 0; i < 3; i++)
  {
    if (s[i]->categories().passes(cats) == result[i + 1])
    {
      std::cerr << "s" << i << ":passed ";
    }
    else
    {
      std::cerr << "s" << i << ":failed ";
      status = false;
    }
  }
  if (status)
  {
    std::cerr << " - PASSED\n";
  }
  else
  {
    std::cerr << " - FAILED\n";
  }
  return status;
}

bool testValidity(
  const AttributePtr& att, const ItemPtr s[], const std::set<std::string>& cats, bool result[])
{
  bool status = true;
  if (att->isValid(cats) == result[0])
  {
    std::cerr << att->name() << ":passed ";
  }
  else
  {
    std::cerr << att->name() << ":failed ";
    status = false;
  }

  for (int i = 0; i < 3; i++)
  {
    if (s[i]->isValid(cats) == result[i + 1])
    {
      std::cerr << "s" << i << ":passed ";
    }
    else
    {
      std::cerr << "s" << i << ":failed ";
      status = false;
    }
  }
  if (status)
  {
    std::cerr << " - PASSED\n";
  }
  else
  {
    std::cerr << " - FAILED\n";
  }
  return status;
}

bool testResource(const attribute::ResourcePtr& attRes, const std::string& prefix)
{
  // Lets find the Attribute and its Items
  AttributePtr att = attRes->findAttribute("TestAtt");
  smtkTest(att != nullptr, "Could not find attribute!");
  ItemPtr s[3];
  bool catResults[4], valResults[4];
  std::set<std::string> cats;
  bool status = true;
  s[0] = att->find("s0");
  s[1] = att->find("s1");
  s[2] = att->find("s2");

  std::cerr << "TestAtt Categories: ";
  att->categories().print();

  for (int i = 0; i < 3; i++)
  {
    smtkTest(s[i] != nullptr, "Could not find s" << i << "!");
    std::cerr << "s" << i << " Categories: ";
    s[i]->categories().print();
  }

  // Passing Category checks
  // s0 should pass if given A, a, b, or nothing
  // s1 should pass if given b, c or nothing
  // s2 should pass if only given nothing or A or (d and e)

  // Passing Validity checks
  // A should pass if  given A or a or Z - only s0 has a default value
  // s0 should pass always
  // s1 should pass if not given b or c
  // s2 should pass if not given {A or (d and e)}

  std::cerr << prefix << " Testing Categories with {} :";
  catResults[0] = true;
  catResults[1] = true;
  catResults[2] = true;
  catResults[3] = true;
  status = (testCategories(att, s, cats, catResults)) ? status : false;
  std::cerr << prefix << " Testing Validity with {} :";
  valResults[0] = false;
  valResults[1] = true;
  valResults[2] = false;
  valResults[3] = false;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  std::cerr << prefix << " Testing Categories with {Z} :";
  cats.insert("Z");
  catResults[0] = false;
  catResults[1] = false;
  catResults[2] = false;
  catResults[3] = false;
  status = (testCategories(att, s, cats, catResults)) ? status : false;
  std::cerr << prefix << " Testing Validity with {Z} :";
  valResults[0] = true;
  valResults[1] = true;
  valResults[2] = true;
  valResults[3] = true;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  std::cerr << prefix << " Testing Categories with {Z,A} :";
  cats.insert("A");
  catResults[0] = true;
  catResults[1] = true;
  catResults[2] = false;
  catResults[3] = true;
  status = (testCategories(att, s, cats, catResults)) ? status : false;
  std::cerr << prefix << " Testing Validity with {Z,A} :";
  valResults[0] = false;
  valResults[1] = true;
  valResults[2] = true;
  valResults[3] = false;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  std::cerr << prefix << " Testing Categories with {Z,b} :";
  cats.insert("b");
  cats.erase("A");
  catResults[0] = true;
  catResults[1] = true;
  catResults[2] = true;
  catResults[3] = false;
  status = (testCategories(att, s, cats, catResults)) ? status : false;
  std::cerr << prefix << " Testing Validity with {Z,b} :";
  valResults[0] = false;
  valResults[1] = true;
  valResults[2] = false;
  valResults[3] = true;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  std::cerr << prefix << " Testing Categories with {Z,c} :";
  cats.insert("c");
  cats.erase("b");
  catResults[0] = true;
  catResults[1] = false;
  catResults[2] = true;
  catResults[3] = false;
  status = (testCategories(att, s, cats, catResults)) ? status : false;
  std::cerr << prefix << " Testing Validity with {Z,c} :";
  valResults[0] = false;
  valResults[1] = true;
  valResults[2] = false;
  valResults[3] = true;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  std::cerr << prefix << " Testing Categories with {Z,d} :";
  cats.insert("d");
  cats.erase("c");
  catResults[0] = false;
  catResults[1] = false;
  catResults[2] = false;
  catResults[3] = false;
  status = (testCategories(att, s, cats, catResults)) ? status : false;
  std::cerr << prefix << " Testing Validity with {Z,d} :";
  valResults[0] = true;
  valResults[1] = true;
  valResults[2] = true;
  valResults[3] = true;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  std::cerr << prefix << " Testing Categories with {Z,d,e} :";
  cats.insert("e");
  catResults[0] = true;
  catResults[1] = false;
  catResults[2] = false;
  catResults[3] = true;
  status = (testCategories(att, s, cats, catResults)) ? status : false;
  std::cerr << prefix << " Testing Validity with {Z,d,e} :";
  valResults[0] = false;
  valResults[1] = true;
  valResults[2] = true;
  valResults[3] = false;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  return status;
}

void setupAttributeResource(attribute::ResourcePtr& attRes)
{
  DefinitionPtr A = attRes->createDefinition("A");
  A->localCategories().insert("A");
  StringItemDefinitionPtr sItemDef0 = A->addItemDefinition<StringItemDefinition>("s0");
  sItemDef0->localCategories().insert("a");
  sItemDef0->localCategories().insert("b");
  sItemDef0->setDefaultValue("foo");
  StringItemDefinitionPtr sItemDef1 = A->addItemDefinition<StringItemDefinition>("s1");
  sItemDef1->localCategories().insert("b");
  sItemDef1->localCategories().insert("c");
  sItemDef1->setIsOkToInherit(false);
  StringItemDefinitionPtr sItemDef2 = A->addItemDefinition<StringItemDefinition>("s2");
  sItemDef2->localCategories().insert("d");
  sItemDef2->localCategories().insert("e");
  sItemDef2->localCategories().setMode(Categories::Set::CombinationMode::All);
  attRes->finalizeDefinitions();
  attRes->createAttribute("TestAtt", "A");
}
}

int unitPassCategories(int /*unused*/, char* /*unused*/ [])
{
  //
  // I. Let's create an attribute resource and some definitions
  attribute::ResourcePtr attRes = attribute::Resource::create();
  setupAttributeResource(attRes);

  smtkTest(testResource(attRes, "First Pass - "), "Failed checking Categories in First Pass");
  io::AttributeWriter writer;
  io::AttributeReader reader;
  io::Logger logger;
  std::string writeRroot(SMTK_SCRATCH_DIR);
  std::string fname = writeRroot + "/unitAttributePassCategoriesTest.sbi";
  std::string rname = writeRroot + "/unitAttributePassCategoriesTest.smtk";

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
  smtkTest(testResource(attRes, "JSON Pass - "), "Failed checking Categories in JSON Pass");

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
  smtkTest(testResource(attRes, "XML Pass - "), "Failed checking Categories in XML Pass");

  return 0;
}
