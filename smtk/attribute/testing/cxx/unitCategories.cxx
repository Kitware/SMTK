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
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
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
bool compareSets(const std::set<std::string>& test, const std::set<std::string>& truth)
{
  if (test == truth)
  {
    std::cerr << "Passed\n";
    return true;
  }
  bool first = true;
  for (const auto& s : truth)
  {
    if (test.find(s) == test.end())
    {
      if (first)
      {
        std::cerr << " Missing:";
        first = false;
      }
      std::cerr << " " << s;
    }
  }
  first = true;
  for (const auto& s : test)
  {
    if (truth.find(s) == truth.end())
    {
      if (first)
      {
        std::cerr << " Extra:";
        first = false;
      }
      std::cerr << " " << s;
    }
  }
  std::cerr << std::endl;
  return false;
}

bool testCategories(
  const ItemDefinitionPtr& idef,
  const std::string& prefix,
  const std::map<std::string, std::set<std::string>>& answers)
{
  bool status = true;
  auto gidef = std::dynamic_pointer_cast<GroupItemDefinition>(idef);
  if (gidef != nullptr)
  {
    int i, n = static_cast<int>(gidef->numberOfItemDefinitions());
    for (i = 0; i < n; i++)
    {
      if (!testCategories(gidef->itemDefinition(i), prefix, answers))
      {
        status = false;
      }
    }
  }
  else
  {
    auto videf = std::dynamic_pointer_cast<ValueItemDefinition>(idef);
    if (videf != nullptr)
    {
      for (const auto& cidef : videf->childrenItemDefinitions())
      {
        if (!testCategories(cidef.second, prefix, answers))
        {
          status = false;
        }
      }
      // Additional Checks for s1
      if (videf->name() == "s1")
      {
        const std::vector<std::set<std::string>> enumCats = { { "ec1" }, { "ec2" }, {} };
        const std::vector<bool> enumAL = { true, false, true };
        std::size_t i, n = videf->numberOfDiscreteValues();
        if (n != 3)
        {
          std::cerr << prefix << " Testing Item Definition:" << idef->name()
                    << " number of enums = " << n << " not 3\n";
          status = false;
        }
        for (i = 0; i < n; i++)
        {
          std::string e = videf->discreteEnum(i);
          std::cerr << prefix << " Testing Item Definition:" << idef->name() << " enum = " << e
                    << "'s Categories: ";
          if (!compareSets(videf->enumCategories(e).includedCategoryNames(), enumCats[i]))
          {
            status = false;
          }
          if (videf->hasEnumAdvanceLevel(e) != enumAL[i])
          {
            std::cerr << prefix << " Testing Item Definition:" << idef->name() << " enum = " << e
                      << "'s Advance Level: Failed!\n";
            status = false;
          }

          if (videf->hasEnumAdvanceLevel(e) && (videf->enumAdvanceLevel(e) != 10))
          {
            std::cerr << prefix << " Testing Item Definition:" << idef->name() << " enum = " << e
                      << "'s Advance Level Value: " << videf->enumAdvanceLevel(e)
                      << " should be 10!\n";
            status = false;
          }
        }
      }
    }
  }

  std::cerr << prefix << " Testing Item Definition:" << idef->name() << "'s Categories: ";
  if (!compareSets(idef->categories().categoryNames(), answers.at(idef->name())))
  {
    status = false;
  }
  return status;
}

bool testCategories(
  const DefinitionPtr& def,
  const std::string& prefix,
  const std::map<std::string, std::set<std::string>>& answers)
{
  int i, n = static_cast<int>(def->numberOfItemDefinitions());
  bool status = true;
  for (i = 0; i < n; i++)
  {
    if (!testCategories(def->itemDefinition(i), prefix, answers))
    {
      status = false;
    }
  }

  std::cerr << prefix << " Testing Definition:" << def->type() << "'s Categories: ";
  if (!compareSets(def->categories().categoryNames(), answers.at(def->type())))
  {
    status = false;
  }
  return status;
}

bool testAttriubute(
  const attribute::ResourcePtr& attRes,
  const std::string& attName,
  bool relevance,
  bool validity)
{
  attribute::AttributePtr att = attRes->findAttribute(attName);
  if (!att)
  {
    std::cerr << "\t Attribute: " << attName << " could not be found!\n";
    return false;
  }
  if (att->isRelevant() != relevance)
  {
    std::cerr << "\t Attribute: " << attName << " had incorrect relevance! Should have been "
              << relevance << std::endl;
    return false;
  }
  if (att->isValid() != validity)
  {
    std::cerr << "\t Attribute: " << attName << " had incorrect validity! Should have been "
              << validity << std::endl;
    return false;
  }
  std::cerr << "\t Attribute: " << attName << " Passed!\n";
  return true;
}

bool testCategories(
  const attribute::ResourcePtr& attRes,
  const std::string& prefix,
  const std::map<std::string, std::set<std::string>>& answers)
{
  std::vector<smtk::attribute::DefinitionPtr> defs;
  attRes->definitions(defs);
  bool status = true;
  for (const auto& def : defs)
  {
    if (!testCategories(def, prefix, answers))
    {
      status = false;
    }
  }

  std::cerr << prefix << " Testing Resource's Categories: ";
  if (!compareSets(attRes->categories(), answers.at("resource")))
  {
    status = false;
  }

  std::cerr << prefix << " Testing Resource's Attributes:\n";
  status = status && testAttriubute(attRes, "a", true, false);
  status = status && testAttriubute(attRes, "b", true, false);
  status = status && testAttriubute(attRes, "c", true, false);
  status = status && testAttriubute(attRes, "d", true, false);
  status = status && testAttriubute(attRes, "e", false, true);
  status = status && testAttriubute(attRes, "f", false, true);
  status = status && testAttriubute(attRes, "g", false, true);

  attRes->setActiveCategoriesEnabled(false);

  std::cerr << prefix << " Testing Resource's Attributes with Active Categories Disabled:\n";
  status = status && testAttriubute(attRes, "a", true, false);
  status = status && testAttriubute(attRes, "b", true, false);
  status = status && testAttriubute(attRes, "c", true, false);
  status = status && testAttriubute(attRes, "d", true, false);
  status = status && testAttriubute(attRes, "e", true, true);
  status = status && testAttriubute(attRes, "f", true, true);
  status = status && testAttriubute(attRes, "g", true, false);

  attRes->setActiveCategoriesEnabled(true);

  return status;
}

void setupAttributeResource(attribute::ResourcePtr& attRes)
{
  DefinitionPtr A = attRes->createDefinition("A");
  A->localCategories().insertInclusion("A");
  GroupItemDefinitionPtr gItemDef0 = A->addItemDefinition<GroupItemDefinition>("g1");
  gItemDef0->localCategories().insertInclusion("g1");
  gItemDef0->setCategoryInheritanceMode(Categories::CombinationMode::Or);
  // Lets create a Discrete String Item with Enums having category and advance level info
  // assigned to some of them
  StringItemDefinitionPtr sItemDef0 = gItemDef0->addItemDefinition<StringItemDefinition>("s1");
  sItemDef0->setCategoryInheritanceMode(Categories::CombinationMode::Or);
  sItemDef0->localCategories().insertInclusion("s1");
  sItemDef0->addDiscreteValue("a", "e1");
  sItemDef0->addDiscreteValue("b", "e2");
  sItemDef0->addDiscreteValue("c", "e3");
  sItemDef0->addEnumCategory("e1", "ec1");
  sItemDef0->addEnumCategory("e2", "ec2");
  sItemDef0->setEnumAdvanceLevel("e1", 10);
  sItemDef0->setEnumAdvanceLevel("e3", 10);
  StringItemDefinitionPtr sItemDef1 = sItemDef0->addItemDefinition<StringItemDefinition>("s2");
  sItemDef1->localCategories().insertInclusion("s2");
  sItemDef1->setCategoryInheritanceMode(Categories::CombinationMode::LocalOnly);
  VoidItemDefinitionPtr vItemDef = sItemDef1->addItemDefinition<VoidItemDefinition>("v1");
  vItemDef->localCategories().insertInclusion("v1");
  vItemDef->setCategoryInheritanceMode(Categories::CombinationMode::Or);
  GroupItemDefinitionPtr gItemDef1 = gItemDef0->addItemDefinition<GroupItemDefinition>("g2");
  gItemDef1->localCategories().insertInclusion("g2");
  gItemDef1->setCategoryInheritanceMode(Categories::CombinationMode::LocalOnly);
  StringItemDefinitionPtr sItemDef3 = gItemDef1->addItemDefinition<StringItemDefinition>("s3");
  sItemDef3->localCategories().insertInclusion("s3");
  DefinitionPtr B = attRes->createDefinition("B", A);
  B->localCategories().insertInclusion("B");
  B->setCategoryInheritanceMode(Categories::CombinationMode::Or);
  vItemDef = B->addItemDefinition<VoidItemDefinition>("v2");
  vItemDef->localCategories().insertInclusion("v2");
  vItemDef->setCategoryInheritanceMode(Categories::CombinationMode::Or);
  DefinitionPtr C = attRes->createDefinition("C", A);
  C->localCategories().insertInclusion("C");
  C->setCategoryInheritanceMode(Categories::CombinationMode::Or);
  DefinitionPtr D = attRes->createDefinition("D", A);
  D->setCategoryInheritanceMode(Categories::CombinationMode::Or);
  vItemDef = D->addItemDefinition<VoidItemDefinition>("v3");
  vItemDef->localCategories().insertInclusion("v3");
  vItemDef->setCategoryInheritanceMode(Categories::CombinationMode::Or);
  DefinitionPtr E = attRes->createDefinition("E");
  E->localCategories().insertInclusion("E");
  E->setCategoryInheritanceMode(Categories::CombinationMode::Or);
  DefinitionPtr F = attRes->createDefinition("F");
  vItemDef = F->addItemDefinition<VoidItemDefinition>("v4");
  F->setCategoryInheritanceMode(Categories::CombinationMode::Or);
  DefinitionPtr G = attRes->createDefinition("G", A);
  G->setCategoryInheritanceMode(Categories::CombinationMode::LocalOnly);
  vItemDef = G->addItemDefinition<VoidItemDefinition>("v5");
  vItemDef->localCategories().insertInclusion("v5");
  vItemDef->setCategoryInheritanceMode(Categories::CombinationMode::Or);
  attRes->finalizeDefinitions();

  attRes->createAttribute("a", "A");
  attRes->createAttribute("b", "B");
  attRes->createAttribute("c", "C");
  attRes->createAttribute("d", "D");
  attRes->createAttribute("e", "E");
  attRes->createAttribute("f", "F");
  attRes->createAttribute("g", "G");

  std::set<std::string> cats;
  // Let set the resource's active categories to A which means
  // attributes a, b, c, d are relevant (and invalid) while
  // e and f are not relevant but valid.
  cats.insert("A");
  attRes->setActiveCategories(cats);
  attRes->setActiveCategoriesEnabled(true);
}
} // namespace

int unitCategories(int /*unused*/, char* /*unused*/[])
{
  std::cerr << std::boolalpha; // To print out booleans
  //
  // I. Let's create an attribute resource and some definitions
  attribute::ResourcePtr attRes = attribute::Resource::create();
  setupAttributeResource(attRes);

  // Lets define what the categories should be for this resource
  std::map<std::string, std::set<std::string>> answers = {
    { "A", { "A", "g1", "g2", "s1", "s2", "s3", "v1" } },
    { "B", { "A", "g1", "g2", "s1", "s2", "s3", "v1", "B", "v2" } },
    { "C", { "A", "g1", "g2", "s1", "s2", "s3", "v1", "C" } },
    { "D", { "A", "g1", "g2", "s1", "s2", "s3", "v1", "v3" } },
    { "E", { "E" } },
    { "F", {} },
    { "G", { "v5" } },
    { "g1", { "A", "g1", "g2", "s1", "s2", "s3", "v1" } },
    { "g2", { "g2", "s3" } },
    { "s1", { "A", "g1", "s1", "s2", "v1" } },
    { "s2", { "s2", "v1" } },
    { "s3", { "g2", "s3" } },
    { "v1", { "s2", "v1" } },
    { "v2", { "A", "B", "v2" } },
    { "v3", { "A", "v3" } },
    { "v4", {} },
    { "v5", { "v5" } },
    { "resource", { "A", "g1", "g2", "s1", "s2", "s3", "v1", "B", "v2", "C", "v3", "E", "v5" } }
  };
  smtkTest(
    testCategories(attRes, "First Pass - ", answers), "Failed checking Categories in First Pass");
  io::AttributeWriter writer;
  io::AttributeReader reader;
  io::Logger logger;
  std::string writeRroot(SMTK_SCRATCH_DIR);
  std::string fname = writeRroot + "/unitAttributeCategoriesTest.sbi";
  std::string rname = writeRroot + "/unitAttributeCategoriesTest.smtk";

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

  // Lets test the validity of a discrete item with enums
  std::set<std::string> cats;
  int status = 0;
  cats.insert("A");
  attRes->setActiveCategories(cats);
  attRes->setActiveCategoriesEnabled(true);

  auto a = attRes->findAttribute("a");
  smtkTest(a != nullptr, "Could not find Attribute a!");
  auto g1 = a->findGroup("g1");
  smtkTest(g1 != nullptr, "Could not find a:g1!");
  auto s1 = g1->findAs<StringItem>(0, "s1");
  smtkTest(s1 != nullptr, "Could not find a:g1:s1!");

  s1->setDiscreteIndex(2); // This enum has no category constraints
  if (!s1->isValid())
  {
    std::cerr << "Setting s1 to e3 - FAILED!!\n";
    status = -1;
  }

  s1->setDiscreteIndex(0); // This enum requires ec1
  if (s1->isValid())
  {
    std::cerr << "Setting s1 to e1 - FAILED!!\n";
    status = -1;
  }
  // Lets add e1's category
  cats.insert("ec1");
  attRes->setActiveCategories(cats);
  if (!s1->isValid())
  {
    std::cerr << "Setting s1 to e1 after adding ec1 to active categories- FAILED!!\n";
    status = -1;
  }
  if (status == 0)
  {
    std::cerr << "All Enum Category Tests - Passed!\n";
  }
  return status;
}
