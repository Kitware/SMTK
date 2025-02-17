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

bool testItem(const ItemPtr& item, bool isOptional, bool isEnabled, bool isLocalEnabled)
{
  bool status = true;
  std::cerr << "\t" << item->name() << " : ";
  // for non-optional items we only check enabled
  if (!item->isOptional())
  {
    if (isOptional)
    {
      std::cerr << " isOptional(FAILED) ";
      status = false;
    }
    if (item->isEnabled() != isEnabled)
    {
      std::cerr << " isEnabled(FAILED) ";
      status = false;
    }
  }
  else
  {
    if (!isOptional)
    {
      std::cerr << " isOptional(FAILED) ";
      status = false;
    }
    if (item->isEnabled() != isEnabled)
    {
      std::cerr << " isEnabled(FAILED) ";
      status = false;
    }
    if (item->localEnabledState() != isLocalEnabled)
    {
      std::cerr << " isLocalEnabled(FAILED) ";
      status = false;
    }
  }
  if (status)
  {
    std::cerr << " - PASSED\n";
  }
  else
  {
    std::cerr << "\n";
  }
  return status;
}

bool testResource(const attribute::ResourcePtr& attRes, const std::string& prefix)
{
  // Lets find the Attributes and test their  Items
  bool status = true;
  AttributePtr a = attRes->findAttribute("a");
  smtkTest(a != nullptr, "Could not find attribute a!");
  std::cerr << prefix << "Testing Attribute a: \n";
  ItemPtr item;
  // We expect the following:
  // s0 - not optional, enabled
  // s1 - optional, enabled, localEnabled
  // s2 - optional, not enabled, not local enabled
  // g0 - optional, enabled, localEnabled
  // g0s0 - optional, enabled, localEnabled
  // g0s1 - optional, not enabled, not local enabled
  // g1 - optional, not enabled, not local enabled
  // g1s0 - optional, not enabled,  local enabled
  // g1s1 - not optional, not enabled
  item = a->find("s0");
  // note that for non-optional the third arg is ignored
  status = status && testItem(item, false, true, true);
  item = a->find("s1");
  status = status && testItem(item, true, true, true);
  item = a->find("s2");
  status = status && testItem(item, true, false, false);

  GroupItemPtr gitem = a->findAs<GroupItem>("g0");
  status = status && testItem(gitem, true, true, true);
  item = gitem->find("g0s0");
  status = status && testItem(item, true, true, true);
  item = gitem->find("g0s1");
  status = status && testItem(item, true, false, false);

  gitem = a->findAs<GroupItem>("g1");
  status = status && testItem(gitem, true, false, false);
  item = gitem->find("g1s0");
  status = status && testItem(item, true, false, true);
  item = gitem->find("g1s1");
  status = status && testItem(item, false, false, false);

  AttributePtr b = attRes->findAttribute("b");
  smtkTest(b != nullptr, "Could not find attribute b!");
  std::cerr << prefix << "Testing Attribute b: \n";
  // We only need to find s2 since that one was forceRequired
  item = b->find("s2");
  if (!item->forceRequired())
  {
    std::cerr << "\ts2 is not marked Force Required\n";
    status = false;
  }
  status = status && testItem(item, false, true, false);
  return status;
}

void setupAttributeResource(attribute::ResourcePtr& attRes)
{
  // Lets create a definition with the following content:
  // s0 - string
  // s1 - optional string which is by default is enabled
  // s2 - optional string which is by default is not enabled
  // g0 - optional group which is by default is enabled with the following
  //  g0s0 - optional string which is by default is enabled
  //  g0s1 - optional string which is by default is not enabled
  // g1 - optional group which is by default is not enabled with the following
  //  g1s0 - optional string which is by default is enabled
  //  g1s1 -  string
  DefinitionPtr A = attRes->createDefinition("A");
  auto sItemDef0 = A->addItemDefinition<StringItemDefinition>("s0");
  auto sItemDef1 = A->addItemDefinition<StringItemDefinition>("s1");
  sItemDef1->setIsOptional(true);
  sItemDef1->setIsEnabledByDefault(true);
  auto sItemDef2 = A->addItemDefinition<StringItemDefinition>("s2");
  sItemDef2->setIsOptional(true);
  auto gItemDef0 = A->addItemDefinition<GroupItemDefinition>("g0");
  gItemDef0->setIsOptional(true);
  gItemDef0->setIsEnabledByDefault(true);
  auto sItemDefg0s0 = gItemDef0->addItemDefinition<StringItemDefinition>("g0s0");
  sItemDefg0s0->setIsOptional(true);
  sItemDefg0s0->setIsEnabledByDefault(true);
  auto sItemDefg0s1 = gItemDef0->addItemDefinition<StringItemDefinition>("g0s1");
  sItemDefg0s1->setIsOptional(true);
  auto gItemDef1 = A->addItemDefinition<GroupItemDefinition>("g1");
  gItemDef1->setIsOptional(true);
  auto sItemDefg1s0 = gItemDef1->addItemDefinition<StringItemDefinition>("g1s0");
  sItemDefg1s0->setIsOptional(true);
  sItemDefg1s0->setIsEnabledByDefault(true);
  auto sItemDefg1s1 = gItemDef1->addItemDefinition<StringItemDefinition>("g1s1");
  attRes->finalizeDefinitions();

  // Now create 2 attributes and set the second's s2 to be force required
  auto att = attRes->createAttribute("a", "A");
  auto att1 = attRes->createAttribute("b", "A");
  ItemPtr item = att1->find("s2");
  item->setForceRequired(true);
}
} // namespace

int unitOptionalItems(int /*unused*/, char* /*unused*/[])
{
  //
  // I. Let's create an attribute resource and some definitions
  attribute::ResourcePtr attRes = attribute::Resource::create();
  setupAttributeResource(attRes);

  smtkTest(testResource(attRes, "First Pass - "), "Failed testing Optional in First Pass");
  io::AttributeWriter writer;
  io::AttributeReader reader;
  io::Logger logger;
  std::string writeRroot(SMTK_SCRATCH_DIR);
  std::string fname = writeRroot + "/unitAttributeOptionalItemsTest.sbi";
  std::string rname = writeRroot + "/unitAttributeOptionalItemsTest.smtk";

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
  smtkTest(testResource(attRes, "JSON Pass - "), "Failed testing Optional in JSON Pass");

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
  smtkTest(testResource(attRes, "XML Pass - "), "Failed testing Optional in XML Pass");

  return 0;
}
