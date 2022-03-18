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
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
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

const std::vector<std::pair<int, int>> g_minMaxRanges = { { 2, 0 },
                                                          { 0, 2 },
                                                          { 2, 2 },
                                                          { 1, 2 },
                                                          { 0, 0 } };

bool testGroupItem(
  GroupItemPtr& gitem,
  const std::pair<int, int>& minMax,
  const std::string& prefix)
{
  smtkTest(gitem != nullptr, prefix << "Failed to find groupItem");
  if (!gitem->isConditional())
  {
    std::cerr << "\n\tItem: " << gitem->name() << " is not conditional\n";
    return false;
  }

  std::size_t numEnabled = 0;
  // Now lets turn the items on one at a time
  for (std::size_t i = 0; i < 3; ++i)
  {
    gitem->item(i)->setIsEnabled(true);
    ++numEnabled;
    if (
      ((numEnabled < static_cast<std::size_t>(minMax.first)) ||
       ((minMax.second != 0) && (numEnabled > static_cast<std::size_t>(minMax.second)))) &&
      gitem->conditionalsSatisfied())
    {
      std::cerr << "\n\tItem: " << gitem->name() << " incorrectly satisfied its conditionals. Had "
                << numEnabled << "items enabled but its requirements are (" << minMax.first << ", "
                << minMax.second << ")\n";
      return false;
    }
    else if (
      ((numEnabled >= static_cast<std::size_t>(minMax.first)) &&
       ((minMax.second != 0) && (numEnabled <= static_cast<std::size_t>(minMax.second)))) &&
      !gitem->conditionalsSatisfied())
    {
      std::cerr << "\n\tItem: " << gitem->name() << " incorrectly failed its conditionals. Had "
                << numEnabled << " items enabled but its requirements are (" << minMax.first << ", "
                << minMax.second << ")\n";
      return false;
    }
  }
  // Now lets turn the items off one at a time
  for (std::size_t i = 0; i < 3; ++i)
  {
    gitem->item(i)->setIsEnabled(false);
    --numEnabled;
    if (
      ((numEnabled < static_cast<std::size_t>(minMax.first)) ||
       ((minMax.second != 0) && (numEnabled > static_cast<std::size_t>(minMax.second)))) &&
      gitem->conditionalsSatisfied())
    {
      std::cerr << "\n\tItem: " << gitem->name() << " incorrectly satisfied its conditionals. Had "
                << numEnabled << " items enabled but its requirements are (" << minMax.first << ", "
                << minMax.second << ")\n";
      return false;
    }
    else if (
      ((numEnabled >= static_cast<std::size_t>(minMax.first)) &&
       ((minMax.second != 0) && (numEnabled <= static_cast<std::size_t>(minMax.second)))) &&
      !gitem->conditionalsSatisfied())
    {
      std::cerr << "\n\tItem: " << gitem->name() << " incorrectly failed its conditionals. Had "
                << numEnabled << " items enabled but its requirements are (" << minMax.first << ", "
                << minMax.second << ")\n";
      return false;
    }
  }
  return true;
}

bool testResource(const attribute::ResourcePtr& attRes, const std::string& prefix)
{

  // Find test attribute
  auto testAtt = attRes->findAttribute("Test1");

  if (testAtt == nullptr)
  {
    std::cerr << prefix << " Failed to find test attribute\n";
    return false;
  }

  // Lets test the group items contained in the test attribute
  std::cerr << prefix << " Testing Attribute's Conditional Groups: ";
  bool status = true;
  for (int i = 0; i < 5; ++i)
  {
    auto gitem = std::dynamic_pointer_cast<GroupItem>(testAtt->item(i));
    status = status && testGroupItem(gitem, g_minMaxRanges[i], prefix);
  }
  if (status)
  {
    std::cerr << "- Passed\n";
  }
  return status;
}
} // namespace

int unitConditionalGroup(int /*unused*/, char* /*unused*/[])
{
  // Read in the test configurations files
  std::string attFile;
  attFile = SMTK_DATA_DIR;
  attFile += "/attribute/attribute_collection/choiceGroupExample.sbt";
  io::AttributeReader reader;
  io::Logger logger;
  auto attRes = attribute::Resource::create();
  reader.read(attRes, attFile, logger);
  if (logger.hasErrors())
  {
    std::cerr << "Errors Generated when reading SBT file :\n" << logger.convertToString();
    return -1;
  }
  std::cerr << std::boolalpha; // To print out booleans
  //
  // Let's create an attribute of type Test
  auto testAtt = attRes->createAttribute("Test1", "Test");
  smtkTest(testAtt != nullptr, "Failed to create test attribute");

  smtkTest(
    testResource(attRes, "First Pass - "), "Failed testing Conditional Groups in First Pass");
  io::AttributeWriter writer;
  std::string writeRroot(SMTK_SCRATCH_DIR);
  std::string fname = writeRroot + "/unitAttributeConditionalGroupTest.sbi";
  std::string rname = writeRroot + "/unitAttributeConditionalGroupTest.smtk";

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
  smtkTest(testResource(attRes, "JSON Pass - "), "Failed testing Conditional Groups in JSON Pass");

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
  smtkTest(testResource(attRes, "XML Pass - "), "Failed testing Conditional Groups in XML Pass");

  return 0;
}
