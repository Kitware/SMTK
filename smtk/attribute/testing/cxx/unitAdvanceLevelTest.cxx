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

bool testDefinitionValidity(
  const DefinitionPtr& def,
  bool hlr,
  bool hlw,
  unsigned int lr,
  unsigned int lw,
  unsigned int r,
  unsigned int w,
  const std::string& prefix)
{
  bool status = true;
  if (hlr)
  {
    if (!def->hasLocalAdvanceLevelInfo(0))
    {
      std::cerr << prefix << "Failed - Def:" << def->type()
                << " does not have local read advance level\n";
      status = false;
    }
    if (def->localAdvanceLevel(0) != lr)
    {
      std::cerr << prefix << "Failed - Def:" << def->type()
                << " has local read advance level: " << def->localAdvanceLevel(0) << " instead of "
                << lr << std::endl;
      status = false;
    }
  }
  else if (def->hasLocalAdvanceLevelInfo(0))
  {
    std::cerr << prefix << "Failed - Def:" << def->type() << " has local read advance level\n";
    status = false;
  }

  if (hlw)
  {
    if (!def->hasLocalAdvanceLevelInfo(1))
    {
      std::cerr << prefix << "Failed - Def:" << def->type()
                << " does not have local write advance level\n";
      status = false;
    }
    if (def->localAdvanceLevel(1) != lw)
    {
      std::cerr << prefix << "Failed - Def:" << def->type()
                << " has local write advance level: " << def->localAdvanceLevel(1) << " instead of "
                << lw << std::endl;
      status = false;
    }
  }
  else if (def->hasLocalAdvanceLevelInfo(1))
  {
    std::cerr << prefix << "Failed - Def:" << def->type() << " has local write advance level\n";
    status = false;
  }

  if (def->advanceLevel(0) != r)
  {
    std::cerr << prefix << "Failed - Def:" << def->type()
              << " has read advance level: " << def->advanceLevel(0) << " instead of " << r
              << std::endl;
    status = false;
  }

  if (def->advanceLevel(1) != w)
  {
    std::cerr << prefix << "Failed - Def:" << def->type()
              << " has read advance level: " << def->advanceLevel(1) << " instead of " << w
              << std::endl;
    status = false;
  }

  if (status)
  {
    std::cerr << prefix << "Def:" << def->type() << " Passed!\n";
  }
  return status;
}

bool testItemDefinitionValidity(
  const ConstItemDefinitionPtr& def,
  bool hlr,
  bool hlw,
  unsigned int lr,
  unsigned int lw,
  unsigned int r,
  unsigned int w,
  const std::string& prefix)
{
  bool status = true;
  if (hlr)
  {
    if (!def->hasLocalAdvanceLevelInfo(0))
    {
      std::cerr << prefix << "Failed - ItemDef:" << def->name()
                << " does not have local read advance level\n";
      status = false;
    }
    if (def->localAdvanceLevel(0) != lr)
    {
      std::cerr << prefix << "Failed - ItemDef:" << def->name()
                << " has local read advance level: " << def->localAdvanceLevel(0) << " instead of "
                << lr << std::endl;
      status = false;
    }
  }
  else if (def->hasLocalAdvanceLevelInfo(0))
  {
    std::cerr << prefix << "Failed - ItemDef:" << def->name() << " has local read advance level\n";
    status = false;
  }

  if (hlw)
  {
    if (!def->hasLocalAdvanceLevelInfo(1))
    {
      std::cerr << prefix << "Failed - ItemDef:" << def->name()
                << " does not have local write advance level\n";
      status = false;
    }
    if (def->localAdvanceLevel(1) != lw)
    {
      std::cerr << prefix << "Failed - ItemDef:" << def->name()
                << " has local write advance level: " << def->localAdvanceLevel(1) << " instead of "
                << lw << std::endl;
      status = false;
    }
  }
  else if (def->hasLocalAdvanceLevelInfo(1))
  {
    std::cerr << prefix << "Failed - ItemDef:" << def->name() << " has local write advance level\n";
    status = false;
  }

  if (def->advanceLevel(0) != r)
  {
    std::cerr << prefix << "Failed - ItemDef:" << def->name()
              << " has read advance level: " << def->advanceLevel(0) << " instead of " << r
              << std::endl;
    status = false;
  }

  if (def->advanceLevel(1) != w)
  {
    std::cerr << prefix << "Failed - ItemDef:" << def->name()
              << " has read advance level: " << def->advanceLevel(1) << " instead of " << w
              << std::endl;
    status = false;
  }

  if (status)
  {
    std::cerr << prefix << "ItemDef:" << def->name() << " Passed!\n";
  }
  return status;
}

bool testItemValidity(
  const ItemPtr& item,
  bool hlr,
  bool hlw,
  unsigned int lr,
  unsigned int lw,
  unsigned int r,
  unsigned int w,
  const std::string& prefix)
{
  bool status = true;
  if (hlr)
  {
    if (!item->hasLocalAdvanceLevelInfo(0))
    {
      std::cerr << prefix << "Failed - Item:" << item->name()
                << " does not have local read advance level\n";
      status = false;
    }
    if (item->localAdvanceLevel(0) != lr)
    {
      std::cerr << prefix << "Failed - Item:" << item->name()
                << " has local read advance level: " << item->localAdvanceLevel(0) << " instead of "
                << lr << std::endl;
      status = false;
    }
  }
  else if (item->hasLocalAdvanceLevelInfo(0))
  {
    std::cerr << prefix << "Failed - Item:" << item->name() << " has local read advance level\n";
    status = false;
  }

  if (hlw)
  {
    if (!item->hasLocalAdvanceLevelInfo(1))
    {
      std::cerr << prefix << "Failed - Item:" << item->name()
                << " does not have local write advance level\n";
      status = false;
    }
    if (item->localAdvanceLevel(1) != lw)
    {
      std::cerr << prefix << "Failed - Item:" << item->name()
                << " has local write advance level: " << item->localAdvanceLevel(1)
                << " instead of " << lw << std::endl;
      status = false;
    }
  }
  else if (item->hasLocalAdvanceLevelInfo(1))
  {
    std::cerr << prefix << "Failed - Item:" << item->name() << " has local write advance level\n";
    status = false;
  }

  if (item->advanceLevel(0) != r)
  {
    std::cerr << prefix << "Failed - Item:" << item->name()
              << " has read advance level: " << item->advanceLevel(0) << " instead of " << r
              << std::endl;
    status = false;
  }

  if (item->advanceLevel(1) != w)
  {
    std::cerr << prefix << "Failed - Item:" << item->name()
              << " has read advance level: " << item->advanceLevel(1) << " instead of " << w
              << std::endl;
    status = false;
  }

  if (status)
  {
    std::cerr << prefix << "Item:" << item->name() << " Passed!\n";
  }
  return status;
}

bool testAttributeValidity(
  const AttributePtr& att,
  bool hlr,
  bool hlw,
  unsigned int lr,
  unsigned int lw,
  unsigned int r,
  unsigned int w,
  const std::string& prefix)
{
  bool status = true;
  if (hlr)
  {
    if (!att->hasLocalAdvanceLevelInfo(0))
    {
      std::cerr << prefix << "Failed - Attribute:" << att->name()
                << " does not have local read advance level\n";
      status = false;
    }
    if (att->localAdvanceLevel(0) != lr)
    {
      std::cerr << prefix << "Failed - Attribute:" << att->name()
                << " has local read advance level: " << att->localAdvanceLevel(0) << " instead of "
                << lr << std::endl;
      status = false;
    }
  }
  else if (att->hasLocalAdvanceLevelInfo(0))
  {
    std::cerr << prefix << "Failed - Attribute:" << att->name()
              << " has local read advance level\n";
    status = false;
  }

  if (hlw)
  {
    if (!att->hasLocalAdvanceLevelInfo(1))
    {
      std::cerr << prefix << "Failed - Attribute:" << att->name()
                << " does not have local write advance level\n";
      status = false;
    }
    if (att->localAdvanceLevel(1) != lw)
    {
      std::cerr << prefix << "Failed - Attribute:" << att->name()
                << " has local write advance level: " << att->localAdvanceLevel(1) << " instead of "
                << lw << std::endl;
      status = false;
    }
  }
  else if (att->hasLocalAdvanceLevelInfo(1))
  {
    std::cerr << prefix << "Failed - Attribute:" << att->name()
              << " has local write advance level\n";
    status = false;
  }

  if (att->advanceLevel(0) != r)
  {
    std::cerr << prefix << "Failed - Attribute:" << att->name()
              << " has read advance level: " << att->advanceLevel(0) << " instead of " << r
              << std::endl;
    status = false;
  }

  if (att->advanceLevel(1) != w)
  {
    std::cerr << prefix << "Failed - Attribute:" << att->name()
              << " has read advance level: " << att->advanceLevel(1) << " instead of " << w
              << std::endl;
    status = false;
  }

  if (status)
  {
    std::cerr << prefix << "Attribute:" << att->name() << " Passed!\n";
  }
  return status;
}

bool testResource(const attribute::ResourcePtr& attRes, const std::string& prefix)
{
  bool status = true;
  // Lets find the Attribute and its Items
  AttributePtr att = attRes->findAttribute("TestAtt");
  smtkTest(att != nullptr, "Could not find attribute!");

  // Lets first test the definitions
  // This should be "C"
  DefinitionPtr def = att->definition();
  status &= testDefinitionValidity(def, true, true, 3, 4, 3, 4, prefix);

  // Lets get C's base - should be B
  def = def->baseDefinition();
  status &= testDefinitionValidity(def, true, false, 2, 4, 2, 1, prefix);

  // Lets get B's base - should be A
  def = def->baseDefinition();
  status &= testDefinitionValidity(def, false, true, 2, 1, 0, 1, prefix);

  ItemPtr item = att->find("sA0");
  status &= testItemDefinitionValidity(item->definition(), false, false, 0, 0, 0, 1, prefix);
  status &= testItemValidity(item, false, false, 0, 0, 3, 4, prefix);

  GroupItemPtr gitem = att->findGroup("gA0");
  status &= testItemDefinitionValidity(gitem->definition(), true, false, 6, 0, 6, 1, prefix);
  status &= testItemValidity(gitem, false, false, 0, 0, 6, 4, prefix);
  item = gitem->item(0);
  status &= testItemDefinitionValidity(item->definition(), false, true, 0, 7, 6, 7, prefix);
  status &= testItemValidity(item, false, false, 0, 0, 6, 7, prefix);

  item = att->find("sB0");
  status &= testItemDefinitionValidity(item->definition(), false, false, 0, 0, 2, 1, prefix);
  status &= testItemValidity(item, false, false, 0, 0, 3, 4, prefix);

  gitem = att->findGroup("gB0");
  status &= testItemDefinitionValidity(gitem->definition(), false, true, 0, 6, 2, 6, prefix);
  status &= testItemValidity(gitem, false, false, 0, 0, 3, 6, prefix);
  item = gitem->item(0);
  status &= testItemDefinitionValidity(item->definition(), true, false, 5, 0, 5, 6, prefix);
  status &= testItemValidity(item, false, false, 0, 0, 5, 6, prefix);

  item = att->find("sC0");
  status &= testItemDefinitionValidity(item->definition(), false, false, 0, 0, 3, 4, prefix);
  status &= testItemValidity(item, false, false, 0, 0, 3, 4, prefix);

  gitem = att->findGroup("gC0");
  status &= testItemDefinitionValidity(gitem->definition(), true, false, 0, 6, 0, 4, prefix);
  status &= testItemValidity(gitem, false, false, 0, 0, 3, 4, prefix);
  item = gitem->item(0);
  status &= testItemDefinitionValidity(item->definition(), false, true, 0, 9, 0, 9, prefix);
  status &= testItemValidity(item, false, false, 0, 0, 3, 9, prefix);

  status &= testAttributeValidity(att, false, false, 0, 0, 3, 4, prefix);

  // Lets try changing the attribute's levels
  att->setLocalAdvanceLevel(0, 1);
  std::cerr << prefix << "Adding local read access on attribute\n";
  status &= testAttributeValidity(att, true, false, 1, 0, 1, 4, prefix);
  item = att->find("sA0");
  status &= testItemValidity(item, false, false, 0, 0, 1, 4, prefix);
  std::cerr << prefix << "Adding local write access on item\n";
  item->setLocalAdvanceLevel(1, 5);
  status &= testItemValidity(item, false, true, 0, 5, 1, 5, prefix);
  att->unsetLocalAdvanceLevel(0);
  std::cerr << prefix << "Unsetting local read access on attribute\n";
  status &= testAttributeValidity(att, false, false, 3, 0, 3, 4, prefix);
  status &= testItemValidity(item, false, true, 0, 5, 3, 5, prefix);
  item->unsetLocalAdvanceLevel(1);
  std::cerr << prefix << "Unsetting local write access on item\n";
  status &= testItemValidity(item, false, false, 0, 0, 3, 4, prefix);
  return status;
}

void setupAttributeResource(attribute::ResourcePtr& attRes)
{
  for (int i = 0; i < 10; i++)
  {
    attRes->addAdvanceLevel(i, std::to_string(i));
  }

  // A will not have a read level (assume 0) but will have write = 1
  DefinitionPtr A = attRes->createDefinition("A");
  A->setLocalAdvanceLevel(1, 1);
  // B will not have a write level (should be A) but will have read = 2
  DefinitionPtr B = attRes->createDefinition("B", A);
  B->setLocalAdvanceLevel(0, 2);
  // C will have read=3, write = 4
  DefinitionPtr C = attRes->createDefinition("C", B);
  C->setLocalAdvanceLevel(0, 3);
  C->setLocalAdvanceLevel(1, 4);

  // Lets set some items
  StringItemDefinitionPtr sItemDef = A->addItemDefinition<StringItemDefinition>("sA0");
  GroupItemDefinitionPtr gItemDef = A->addItemDefinition<GroupItemDefinition>("gA0");
  gItemDef->setLocalAdvanceLevel(0, 6);
  sItemDef = gItemDef->addItemDefinition<StringItemDefinition>("gA0s0");
  sItemDef->setLocalAdvanceLevel(1, 7);

  sItemDef = B->addItemDefinition<StringItemDefinition>("sB0");
  gItemDef = B->addItemDefinition<GroupItemDefinition>("gB0");
  gItemDef->setLocalAdvanceLevel(1, 6);
  sItemDef = gItemDef->addItemDefinition<StringItemDefinition>("gB0s0");
  sItemDef->setLocalAdvanceLevel(0, 5);

  sItemDef = C->addItemDefinition<StringItemDefinition>("sC0");
  gItemDef = C->addItemDefinition<GroupItemDefinition>("gC0");
  gItemDef->setLocalAdvanceLevel(0, 0);
  sItemDef = gItemDef->addItemDefinition<StringItemDefinition>("gC0s0");
  sItemDef->setLocalAdvanceLevel(1, 9);

  attRes->finalizeDefinitions();
  attRes->createAttribute("TestAtt", "C");
}
} // namespace

int unitAdvanceLevelTest(int /*unused*/, char* /*unused*/[])
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
  std::string fname = writeRroot + "/unitAttributeAdvanceLevelTest.sbi";
  std::string rname = writeRroot + "/unitAttributeAdvanceLevelTest.smtk";

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
