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
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
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
bool testResource(const attribute::ResourcePtr& attRes, const std::string& prefix)
{
  bool status = true;
  // Lets find the test Attributes and the definition we want to test

  AttributePtr testAtt1 = attRes->findAttribute("testAtt1");
  smtkTest(testAtt1 != nullptr, "Could not find testAtt1!");
  AttributePtr testAtt2 = attRes->findAttribute("testAtt2");
  smtkTest(testAtt2 != nullptr, "Could not find testAtt12");

  DefinitionPtr adef = attRes->findDefinition("A");
  smtkTest(adef != nullptr, "Could not find A");

  // Create 2 attributes of A so we can test
  AttributePtr a = attRes->createAttribute("a", adef);
  smtkTest(a != nullptr, "Could not create a");
  AttributePtr b = attRes->createAttribute("b", adef);
  smtkTest(b != nullptr, "Could not create b");
  ComponentItemPtr compsA[2], compsB[2];

  compsA[0] = a->findAs<ComponentItem>("comp0");
  compsA[1] = a->findAs<ComponentItem>("comp1");
  compsB[0] = b->findAs<ComponentItem>("comp0");
  compsB[1] = b->findAs<ComponentItem>("comp1");

  GroupItemPtr grp = a->findAs<GroupItem>("group1");
  smtkTest(grp != nullptr, "Could not find grp in attribute a");

  int i;
  for (i = 0; i < 2; i++)
  {
    smtkTest(compsA[i] != nullptr, "Could not find comp" << i << " in attribute a");
    smtkTest(compsB[i] != nullptr, "Could not find comp" << i << " in attribute b");
  }

  //Lets test the extensible comp0
  if (!compsA[0]->isValueValid(0, testAtt1))
  {
    std::cerr << prefix << "a's comp0[0] said testAtt1 was not valid and it should have been\n";
    status = false;
  }

  compsA[0]->setValue(0, testAtt1);

  if (!compsA[0]->isValueValid(0, testAtt1))
  {
    std::cerr << prefix
              << "(after setting testAtt1) a's comp0[0] said testAtt1 was not valid and it should "
                 "not have been\n";
    status = false;
  }

  if (compsA[0]->isValueValid(1, testAtt1))
  {
    std::cerr << prefix << "a's comp0[1] said testAtt1 was valid and it should not have been\n";
    status = false;
  }

  if (!compsA[0]->isValueValid(1, testAtt2))
  {
    std::cerr << prefix << "a's comp0[1] said testAtt2 was not valid and it should have been\n";
    status = false;
  }

  compsA[0]->setValue(1, testAtt2);

  for (i = 0; i < 2; i++)
  {
    if (compsB[0]->isValueValid(i, testAtt1))
    {
      std::cerr << prefix << "b's comp0[" << i
                << "] said testAtt1 was valid and it should not have been\n";
      status = false;
    }

    if (compsB[0]->isValueValid(i, testAtt2))
    {
      std::cerr << prefix << "b's comp0[" << i
                << "] said testAtt2 was valid and it should not have been\n";
      status = false;
    }
  }

  compsA[0]->unset(0);

  for (i = 0; i < 2; i++)
  {
    if (!compsA[0]->isValueValid(i, testAtt1))
    {
      std::cerr << prefix << "a's comp0[" << i
                << "] said testAtt1 was not valid and it should have been after unset\n";
      status = false;
    }

    if (!compsB[0]->isValueValid(i, testAtt1))
    {
      std::cerr << prefix << "b's comp0[" << i
                << "] said testAtt1 was not valid and it should have been after unset\n";
      status = false;
    }
  }
  // Lets test comp1
  if (!compsA[1]->isValueValid(0, testAtt1))
  {
    std::cerr << prefix << "a's comp1 said testAtt1 was not valid and it should have been\n";
    status = false;
  }

  compsA[1]->setValue(0, testAtt1);

  if (!compsA[1]->isValueValid(1, testAtt1))
  {
    std::cerr
      << prefix
      << "a's comp1[1] said testAtt1 was not valid and it should have been after assignment\n";
    status = false;
  }

  compsA[1]->setValue(1, testAtt1);

  if (!compsB[1]->isValueValid(0, testAtt1))
  {
    std::cerr << prefix << "b's comp1[0] said testAtt1 was not valid and it should have been\n";
    status = false;
  }

  compsB[1]->setValue(0, testAtt1);

  if (!compsB[1]->isValueValid(1, testAtt1))
  {
    std::cerr
      << prefix
      << "b's comp1[1] said testAtt1 was not valid and it should have been after assignment\n";
    status = false;
  }

  // Lets test the group item - lets create 2 sub groups (hence 2 component items)
  grp->appendGroup();
  grp->appendGroup();

  ComponentItemPtr c0 = dynamic_pointer_cast<ComponentItem>(grp->item(0, 0));
  smtkTest(c0 != nullptr, "Could not find first comp in attribute group1");
  ComponentItemPtr c1 = dynamic_pointer_cast<ComponentItem>(grp->item(1, 0));
  smtkTest(c1 != nullptr, "Could not find second comp in attribute group1");

  if (!c0->isValueValid(testAtt1))
  {
    std::cerr << prefix << "group1[0,0] said testAtt1 was not valid and it should have been\n";
    status = false;
  }

  c0->setValue(testAtt1);

  if (!c0->isValueValid(testAtt1))
  {
    std::cerr
      << prefix
      << "group1[0,0] said testAtt1 was not valid and it should have been after being set\n";
    status = false;
  }

  if (c1->isValueValid(testAtt1))
  {
    std::cerr << prefix << "group1[1,0] said testAtt1 was valid and it shouldn't have been\n";
    status = false;
  }

  if (!c1->isValueValid(testAtt2))
  {
    std::cerr << prefix << "group1[1,0] said testAtt2 was not valid and it should have been\n";
    status = false;
  }

  c1->setValue(testAtt2);

  if (!c1->isValueValid(testAtt2))
  {
    std::cerr
      << prefix
      << "group1[1,0] said testAtt2 was not valid and it should have been after being set\n";
    status = false;
  }

  if (c0->isValueValid(testAtt2))
  {
    std::cerr << prefix << "group1[0,0] said testAtt2 was valid and it shouldn't have been\n";
    status = false;
  }

  // Remove the second group which should allow us to set c0 to testAtt2
  grp->removeGroup(1);
  c1 = nullptr;

  if (!c0->isValueValid(testAtt2))
  {
    std::cerr << prefix
              << "group1[0,0] said testAtt2 was valid and it should have been after deletion\n";
    status = false;
  }

  attRes->removeAttribute(a);
  attRes->removeAttribute(b);
  return status;
}
} // namespace

int unitComponentItemConstraints(int /*unused*/, char* /*unused*/[])
{
  // ----
  // I. Let's create an attribute resource and some definitions
  // The first definition will have three component items:

  // comp0 will have an unique role and have 2 values so we
  // can uniqueness when trying to assign to the same item

  // comp1 will not be unique so we can verify non-uniqueness

  // group1 will contain a component Item that is unique
  // The other definition will be our test definition whose attributes
  // can be assigned to the component items
  attribute::ResourcePtr attRes = attribute::Resource::create();
  attRes->addUniqueRole(10);
  attRes->addUniqueRole(20);
  DefinitionPtr aDef, a1Def, bDef, cDef, dDef, testDef;
  ComponentItemDefinitionPtr compDef;
  testDef = attRes->createDefinition("testDef");
  aDef = attRes->createDefinition("A");
  compDef = aDef->addItemDefinition<ComponentItemDefinition>("comp0");
  compDef->setRole(10);
  compDef->setIsExtensible(false);
  compDef->setNumberOfRequiredValues(2);
  compDef->setAcceptsEntries(
    smtk::common::typeName<attribute::Resource>(), "attribute[type='testDef']", true);

  compDef = aDef->addItemDefinition<ComponentItemDefinition>("comp1");
  compDef->setRole(30);
  compDef->setIsExtensible(false);
  compDef->setNumberOfRequiredValues(2);
  compDef->setAcceptsEntries(
    smtk::common::typeName<attribute::Resource>(), "attribute[type='testDef']", true);

  GroupItemDefinitionPtr groupDef = aDef->addItemDefinition<GroupItemDefinition>("group1");
  groupDef->setIsExtensible(true);
  compDef = groupDef->addItemDefinition<ComponentItemDefinition>("comp");
  compDef->setRole(20);

  attRes->createAttribute("testAtt1", testDef);
  attRes->createAttribute("testAtt2", testDef);

  smtkTest(testResource(attRes, "First Pass - "), "Failed checking Categories in First Pass");
  io::AttributeWriter writer;
  io::AttributeReader reader;
  io::Logger logger;
  std::string writeRroot(SMTK_SCRATCH_DIR);
  std::string fname = writeRroot + "/unitAtttributeComponentItemConstraintsTest.sbi";
  std::string rname = writeRroot + "/unitAtttributeComponentItemConstraintsTest.smtk";

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
