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

bool testEnumRelevance(
  const ItemPtr item,
  const std::set<std::string>& cats,
  const std::vector<std::string>& result)
{
  // Cast the item's definition to a ValueItemDefinition
  auto def = std::dynamic_pointer_cast<const ValueItemDefinition>(item->definition());
  if (!def)
  {
    std::cerr << " Testing Enums: Could not cast to ValueItemDefintion ";
    return false;
  }
  auto res = def->relevantEnums(true, cats, false, 0);
  if (res != result)
  {
    std::cerr << " Testing Enums: returned {";
    for (const auto& s : res)
    {
      std::cerr << "\"" << s << "\"";
    }
    std::cerr << "} should have been {";
    for (const auto& s : result)
    {
      std::cerr << "\"" << s << "\"";
    }
    std::cerr << "}";
    return false;
  }
  std::cerr << " Testing Enums: ";
  return true;
}

bool testCategories(
  const AttributePtr& att,
  const ItemPtr s[],
  const std::set<std::string>& cats,
  bool result[],
  const std::vector<std::string>& enumResults)
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
  for (int i = 0; i < 6; i++)
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
  if (testEnumRelevance(s[3], cats, enumResults))
  {
    std::cerr << ":passed ";
  }
  else
  {
    std::cerr << ":failed ";
    status = false;
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
  const AttributePtr& att,
  const ItemPtr s[],
  const std::set<std::string>& cats,
  bool result[])
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

  for (int i = 0; i < 6; i++)
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
  ItemPtr s[6];
  bool catResults[7], valResults[7];
  std::set<std::string> cats;
  bool status = true;
  s[0] = att->find("s0");
  s[1] = att->find("s1");
  s[2] = att->find("s2");
  s[3] = att->find("s3");
  s[4] = att->find("s4");
  s[5] = att->find("s5");

  std::cerr << "TestAtt Categories: ";
  att->categories().print();

  for (int i = 0; i < 6; i++)
  {
    smtkTest(s[i] != nullptr, "Could not find s" << i << "!");
    std::cerr << "s" << i << " Categories: ";
    s[i]->categories().print();
  }

  // Passing Category checks
  // s0 should pass if given A, a, or b
  // s1 should pass if given b, or c
  // s2 should pass if only A or (d and e)
  // s3 should pass if only A
  // s4 should pass if A and (a or b)
  // s5 should pass if A and d and e

  // Passing Validity checks
  // A should pass if  given A or a or Z - only s0 has a default value
  // s0 should pass always
  // s1 should pass if not given b or c
  // s2 should pass if not given {A or (d and e)}
  // s3 should pass if not given {A}
  // s4 should always pass since when its relevant it has a value
  // s5 should pass if not given {A or d or e}

  // s3's Relevant Enum Checks
  // e1 if a and b
  // e2 if c or d

  std::cerr << prefix << " Testing Categories with {} :";
  catResults[0] = false;
  catResults[1] = false;
  catResults[2] = false;
  catResults[3] = false;
  catResults[4] = false;
  catResults[5] = false;
  catResults[6] = false;
  status = (testCategories(att, s, cats, catResults, {})) ? status : false;
  std::cerr << prefix << " Testing Validity with {} :";
  valResults[0] = true;
  valResults[1] = true;
  valResults[2] = true;
  valResults[3] = true;
  valResults[4] = true;
  valResults[5] = true;
  valResults[6] = true;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  std::cerr << prefix << " Testing Categories with {Z} :";
  cats.insert("Z");
  catResults[0] = false;
  catResults[1] = false;
  catResults[2] = false;
  catResults[3] = false;
  catResults[4] = false;
  catResults[5] = false;
  catResults[6] = false;
  status = (testCategories(att, s, cats, catResults, {})) ? status : false;
  std::cerr << prefix << " Testing Validity with {Z} :";
  valResults[0] = true;
  valResults[1] = true;
  valResults[2] = true;
  valResults[3] = true;
  valResults[4] = true;
  valResults[5] = true;
  valResults[6] = true;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  std::cerr << prefix << " Testing Categories with {Z,A} :";
  cats.insert("A");
  catResults[0] = true;
  catResults[1] = true;
  catResults[2] = false;
  catResults[3] = true;
  catResults[4] = true;
  catResults[5] = false;
  catResults[6] = false;
  status = (testCategories(att, s, cats, catResults, {})) ? status : false;
  std::cerr << prefix << " Testing Validity with {Z,A} :";
  valResults[0] = false;
  valResults[1] = true;
  valResults[2] = true;
  valResults[3] = false;
  valResults[4] = false;
  valResults[5] = true;
  valResults[6] = true;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  std::cerr << prefix << " Testing Categories with {Z,b} :";
  cats.insert("b");
  cats.erase("A");
  catResults[0] = true;
  catResults[1] = true;
  catResults[2] = true;
  catResults[3] = false;
  catResults[4] = false;
  catResults[5] = false;
  catResults[6] = false;
  status = (testCategories(att, s, cats, catResults, {})) ? status : false;
  std::cerr << prefix << " Testing Validity with {Z,b} :";
  valResults[0] = false;
  valResults[1] = true;
  valResults[2] = false;
  valResults[3] = true;
  valResults[4] = true;
  valResults[5] = true;
  valResults[6] = true;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  std::cerr << prefix << " Testing Categories with {Z,a,b} :";
  cats.insert("a");
  catResults[0] = true;
  catResults[1] = true;
  catResults[2] = true;
  catResults[3] = false;
  catResults[4] = false;
  catResults[5] = false;
  catResults[6] = false;
  status = (testCategories(att, s, cats, catResults, { "e1" })) ? status : false;
  std::cerr << prefix << " Testing Validity with {Z,a,b} :";
  valResults[0] = false;
  valResults[1] = true;
  valResults[2] = false;
  valResults[3] = true;
  valResults[4] = true;
  valResults[5] = true;
  valResults[6] = true;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  std::cerr << prefix << " Testing Categories with {Z,c} :";
  cats.insert("c");
  cats.erase("a");
  cats.erase("b");
  catResults[0] = true;
  catResults[1] = false;
  catResults[2] = true;
  catResults[3] = false;
  catResults[4] = false;
  catResults[5] = false;
  catResults[6] = false;
  status = (testCategories(att, s, cats, catResults, { "e2" })) ? status : false;
  std::cerr << prefix << " Testing Validity with {Z,c} :";
  valResults[0] = false;
  valResults[1] = true;
  valResults[2] = false;
  valResults[3] = true;
  valResults[4] = true;
  valResults[5] = true;
  valResults[6] = true;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  std::cerr << prefix << " Testing Categories with {Z,d} :";
  cats.insert("d");
  cats.erase("c");
  catResults[0] = false;
  catResults[1] = false;
  catResults[2] = false;
  catResults[3] = false;
  catResults[4] = false;
  catResults[5] = false;
  catResults[6] = false;
  status = (testCategories(att, s, cats, catResults, { "e2" })) ? status : false;
  std::cerr << prefix << " Testing Validity with {Z,d} :";
  valResults[0] = true;
  valResults[1] = true;
  valResults[2] = true;
  valResults[3] = true;
  valResults[4] = true;
  valResults[5] = true;
  valResults[6] = true;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  std::cerr << prefix << " Testing Categories with {Z,d,e} :";
  cats.insert("e");
  catResults[0] = true;
  catResults[1] = false;
  catResults[2] = false;
  catResults[3] = true;
  catResults[4] = false;
  catResults[5] = false;
  catResults[6] = false;
  status = (testCategories(att, s, cats, catResults, { "e2" })) ? status : false;
  std::cerr << prefix << " Testing Validity with {Z,d,e} :";
  valResults[0] = false;
  valResults[1] = true;
  valResults[2] = true;
  valResults[3] = false;
  valResults[4] = true;
  valResults[5] = true;
  valResults[6] = true;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  std::cerr << prefix << " Testing Categories with {A, a} :";
  cats.clear();
  cats.insert("A");
  cats.insert("a");
  catResults[0] = true;
  catResults[1] = true;
  catResults[2] = false;
  catResults[3] = true;
  catResults[4] = true;
  catResults[5] = true;
  catResults[6] = false;
  status = (testCategories(att, s, cats, catResults, {})) ? status : false;
  std::cerr << prefix << " Testing Validity with {A, a} :";
  valResults[0] = false;
  valResults[1] = true;
  valResults[2] = true;
  valResults[3] = false;
  valResults[4] = false;
  valResults[5] = true;
  valResults[6] = true;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  std::cerr << prefix << " Testing Categories with {A, d, e} :";
  cats.clear();
  cats.insert("A");
  cats.insert("d");
  cats.insert("e");
  catResults[0] = true;
  catResults[1] = true;
  catResults[2] = false;
  catResults[3] = true;
  catResults[4] = true;
  catResults[5] = false;
  catResults[6] = true;
  status = (testCategories(att, s, cats, catResults, { "e2" })) ? status : false;
  std::cerr << prefix << " Testing Validity with {A, a} :";
  valResults[0] = false;
  valResults[1] = true;
  valResults[2] = true;
  valResults[3] = false;
  valResults[4] = false;
  valResults[5] = true;
  valResults[6] = false;
  status = (testValidity(att, s, cats, valResults)) ? status : false;

  return status;
}

void setupAttributeResource(attribute::ResourcePtr& attRes)
{
  DefinitionPtr A = attRes->createDefinition("A");
  A->localCategories().insertInclusion("A");
  StringItemDefinitionPtr sItemDef0 = A->addItemDefinition<StringItemDefinition>("s0");
  sItemDef0->setCategoryInheritanceMode(Categories::CombinationMode::Or);
  sItemDef0->localCategories().insertInclusion("a");
  sItemDef0->localCategories().insertInclusion("b");
  std::cerr << "S0 = " << sItemDef0->localCategories().expression() << std::endl;
  sItemDef0->setDefaultValue("foo");
  StringItemDefinitionPtr sItemDef1 = A->addItemDefinition<StringItemDefinition>("s1");
  sItemDef1->localCategories().insertInclusion("b");
  sItemDef1->localCategories().insertInclusion("c");
  sItemDef1->setCategoryInheritanceMode(Categories::CombinationMode::LocalOnly);
  StringItemDefinitionPtr sItemDef2 = A->addItemDefinition<StringItemDefinition>("s2");
  sItemDef2->setCategoryInheritanceMode(Categories::CombinationMode::Or);
  sItemDef2->localCategories().insertInclusion("d");
  sItemDef2->localCategories().insertInclusion("e");
  sItemDef2->localCategories().setInclusionMode(Categories::Set::CombinationMode::And);
  StringItemDefinitionPtr sItemDef3 = A->addItemDefinition<StringItemDefinition>("s3");
  sItemDef3->addDiscreteValue("a", "e1");
  sItemDef3->addDiscreteValue("b", "e2");
  smtk::attribute::Categories::Expression es;
  es.insertInclusion("a");
  es.insertInclusion("b");
  es.setInclusionMode(Categories::Set::CombinationMode::And);
  sItemDef3->setEnumCategories("e1", es);
  es.reset();
  es.insertInclusion("c");
  es.insertInclusion("d");
  es.setInclusionMode(Categories::Set::CombinationMode::Or);
  sItemDef3->setEnumCategories("e2", es);
  StringItemDefinitionPtr sItemDef4 = A->addItemDefinition<StringItemDefinition>("s4");
  sItemDef4->localCategories().insertInclusion("a");
  sItemDef4->localCategories().insertInclusion("b");
  sItemDef4->setDefaultValue("foo");
  StringItemDefinitionPtr sItemDef5 = A->addItemDefinition<StringItemDefinition>("s5");
  sItemDef5->localCategories().insertInclusion("d");
  sItemDef5->localCategories().insertInclusion("e");
  sItemDef5->localCategories().setInclusionMode(Categories::Set::CombinationMode::And);
  attRes->finalizeDefinitions();
  attRes->createAttribute("TestAtt", "A");
}
} // namespace

int unitPassCategories(int /*unused*/, char* /*unused*/[])
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
  smtkTest(testResource(attRes, "JSON Pass - "), "Failed checking Categories in JSON Pass");

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
  smtkTest(testResource(attRes, "XML Pass - "), "Failed checking Categories in XML Pass");

  return 0;
}
