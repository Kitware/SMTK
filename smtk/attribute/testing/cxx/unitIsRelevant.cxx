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
bool testAttriubuteResource(
  const attribute::ResourcePtr& attRes,
  bool checkCategories,
  bool checkAdvanceLevel,
  unsigned int readAccessLevel,
  bool attRelevance,
  std::vector<bool> itemRelevances,
  bool hasRelevantChildrenAnswer)
{
  bool result = true;
  const attribute::AttributePtr& att = attRes->findAttribute("a");
  if (att == nullptr)
  {
    std::cerr << "Could not find attribute a\n";
    return false;
  }
  if (att->isRelevant(checkCategories, checkAdvanceLevel, readAccessLevel) != attRelevance)
  {
    std::cerr << "\t Attribute: " << att->name() << " had incorrect relevance! Should have been "
              << attRelevance << std::endl;
    result = false;
  }
  int n = (int)itemRelevances.size();
  for (int i = 0; i < n; i++)
  {
    if (
      att->item(i)->isRelevant(checkCategories, checkAdvanceLevel, readAccessLevel) !=
      itemRelevances[i])
    {
      std::cerr << "\t Attribute: " << att->name() << "'s Item: " << att->item(i)->name()
                << " had incorrect relevance! Should have been " << itemRelevances[i] << std::endl;
      result = false;
    }
    auto groupItem = std::dynamic_pointer_cast<GroupItem>(att->item(i));
    if (
      groupItem &&
      (groupItem->hasRelevantChildren(checkCategories, checkAdvanceLevel, readAccessLevel) !=
       hasRelevantChildrenAnswer))
    {
      std::cerr << "\t Attribute: " << att->name() << "'s Item: " << att->item(i)->name()
                << " had incorrect hasRelevantChildren response! Should have been "
                << hasRelevantChildrenAnswer << std::endl;
      result = false;
    }
  }
  return result;
}

bool runTests(const attribute::ResourcePtr& attRes, const std::string& prefix)
{
  bool result = true;
  // First lets test not using advance levels but active categories set to A
  // Lets define what the categories should be for this resource
  std::set<std::string> cats;
  cats.insert("A");
  attRes->setActiveCategories(cats);
  attRes->setActiveCategoriesEnabled(true);
  // in this case the attribute should be relevant and all but the first item
  // which is set to be ignored and the groupItem which has no categories (hence it has no relevant children)
  std::vector<bool> test1 = { false, true, true, false };
  if (testAttriubuteResource(attRes, true, false, 0, true, test1, false))
  {
    std::cerr << prefix << " Test 1 - Passed\n";
  }
  else
  {
    std::cerr << prefix << " Test 1 - Failed\n";
    result = false;
  }
  // Second - same thing but with advance level testing on and access  = 0
  // In this case everyone should fail since the only access level 0 items are the
  // item is set to be ignored and the groupItem which has no categories
  std::vector<bool> test2 = { false, false, false, false };
  if (testAttriubuteResource(attRes, true, true, 0, false, test2, false))
  {
    std::cerr << prefix << " Test 2 - Passed\n";
  }
  else
  {
    std::cerr << prefix << " Test 2 - Failed\n";
    result = false;
  }
  // Third - same thing but with advance level testing on and access  = 1
  // In this case the attribute and second item should pass
  std::vector<bool> test3 = { false, true, false, false };
  if (testAttriubuteResource(attRes, true, true, 1, true, test3, false))
  {
    std::cerr << prefix << " Test 3 - Passed\n";
  }
  else
  {
    std::cerr << prefix << " Test 3 - Failed\n";
    result = false;
  }
  // Fourth - same thing but with advance level testing on and access  = 2
  // In this case the attribute and second and third items should pass
  std::vector<bool> test4 = { false, true, true, false };
  if (testAttriubuteResource(attRes, true, true, 2, true, test4, false))
  {
    std::cerr << prefix << " Test 4 - Passed\n";
  }
  else
  {
    std::cerr << prefix << " Test 4 - Failed\n";
    result = false;
  }
  // Finally lets change the active categories to B - in this case all should fail
  cats.clear();
  cats.insert("B");
  attRes->setActiveCategories(cats);
  std::vector<bool> test5 = { false, false, false, false };
  if (testAttriubuteResource(attRes, true, false, 0, false, test5, false))
  {
    std::cerr << prefix << " Test 5 - Passed\n";
  }
  else
  {
    std::cerr << prefix << " Test 5 - Failed\n";
    result = false;
  }

  // Finally lets turn off category filtering and test with advance level = 0
  // In this case only the groupItem should pass and it should have relevant children
  std::vector<bool> test6 = { false, false, false, true };
  if (testAttriubuteResource(attRes, false, true, 0, true, test6, true))
  {
    std::cerr << prefix << " Test 6 - Passed\n";
  }
  else
  {
    std::cerr << prefix << " Test 6 - Failed\n";
    result = false;
  }

  return result;
}
void setupAttributeResource(attribute::ResourcePtr& attRes)
{
  DefinitionPtr A = attRes->createDefinition("A");
  A->localCategories().insertInclusion("A");
  // Lets create 3 items of read access levels 0, 1, 2 respectively
  // and a group item that has no categories
  auto vItemDef = A->addItemDefinition<VoidItemDefinition>("i0");
  vItemDef = A->addItemDefinition<VoidItemDefinition>("i1");
  vItemDef->setLocalAdvanceLevel(1);
  vItemDef = A->addItemDefinition<VoidItemDefinition>("i2");
  vItemDef->setLocalAdvanceLevel(2);
  auto gItemDef = A->addItemDefinition<GroupItemDefinition>("g3");
  // let make sure this group has no categories
  gItemDef->setCategoryInheritanceMode(Categories::CombinationMode::LocalOnly);
  vItemDef = gItemDef->addItemDefinition<VoidItemDefinition>("g0i0");
  attRes->finalizeDefinitions();

  auto att = attRes->createAttribute("a", "A");
  // Lets set the first item to be ignored
  att->item(0)->setIsIgnored(true);
}
} // namespace

int unitIsRelevant(int /*unused*/, char* /*unused*/[])
{
  std::cerr << std::boolalpha; // To print out booleans
  //
  // I. Let's create an attribute resource and some definitions
  attribute::ResourcePtr attRes = attribute::Resource::create();
  setupAttributeResource(attRes);

  smtkTest(runTests(attRes, "First Pass - "), "Failed isRelevant Tests in First Pass");
  io::AttributeWriter writer;
  io::AttributeReader reader;
  io::Logger logger;
  std::string writeRroot(SMTK_SCRATCH_DIR);
  std::string fname = writeRroot + "/unitIsRelevantTest.sbi";
  std::string rname = writeRroot + "/unitIsRelevantTest.smtk";

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
  smtkTest(runTests(attRes, "JSON Pass - "), "Failed isRelevant Tests in JSON Pass");

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
  smtkTest(runTests(attRes, "XML Pass - "), "Failed isRelevant Tests in XML Pass");

  return 0;
}
