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

void testLoadedAttributeResource(attribute::ResourcePtr& attRes, const std::string& prefix)
{
  DefinitionPtr preDef;
  AttributePtr probAtt;
  // Find all of the definitions
  auto testDef = attRes->findDefinition("testDef");
  auto aDef = attRes->findDefinition("A");
  auto a1Def = attRes->findDefinition("A1");
  auto bDef = attRes->findDefinition("B");
  auto cDef = attRes->findDefinition("C");

  smtkTest(testDef != nullptr, prefix << "- Could not retrieve testDef");
  smtkTest(aDef != nullptr, prefix << "Could not retrieve A");
  smtkTest(bDef != nullptr, prefix << "Could not retrieve B");
  smtkTest(cDef != nullptr, prefix << "Could not retrieve C");
  smtkTest(a1Def != nullptr, prefix << "Could not retrieve A1");

  smtkTest(!aDef->isUsedAsAPrerequisite(), prefix << "- A thinks its used as a prerequisite");
  smtkTest(aDef->hasPrerequisites(), prefix << "- A has no prerequisites");
  smtkTest(aDef->hasPrerequisite(cDef) != nullptr, prefix << "- C is not a prerequisite of A");
  smtkTest(a1Def->hasPrerequisites(), prefix << "- A1 has no prerequisites");
  smtkTest(a1Def->hasPrerequisite(cDef) != nullptr, prefix << "- C is not a prerequisite of A1");
  smtkTest(cDef->isUsedAsAPrerequisite(), prefix << "- C doesn't think its used as a prerequisite");
  smtkTest(cDef->hasPrerequisite(aDef) == nullptr, prefix << "- A thinks its a prerequisite of C");

  // Find the attributes
  auto attTest = attRes->findAttribute("testAtt");
  smtkTest(attTest != nullptr, prefix << "Could not retrieve testAtt");
  auto attA = attRes->findAttribute("a");
  smtkTest(attA != nullptr, prefix << "Could not retrieve attA");
  auto attC = attRes->findAttribute("c");
  smtkTest(attC != nullptr, prefix << "Could not retrieve attC");

  auto result = bDef->canBeAssociated(attTest, probAtt, preDef);
  smtkTest(
    result == Definition::AssociationResultType::Conflict,
    prefix << " - B did not return conflict");
  smtkTest(
    (probAtt == attC) || (probAtt == attA), prefix << "- B did not return attC as conflicting");
  result = cDef->canBeAssociated(attTest, probAtt, preDef);
  smtkTest(
    result == Definition::AssociationResultType::Valid, prefix << "- C did not return valid");
  result = aDef->canBeAssociated(attTest, probAtt, preDef);
  smtkTest(
    result == Definition::AssociationResultType::Conflict, prefix << "- A did not return conflict");
  smtkTest(probAtt == attA, prefix << "- A did not return attA as conflicting");
  result = a1Def->canBeAssociated(attTest, probAtt, preDef);
  smtkTest(
    result == Definition::AssociationResultType::Conflict,
    prefix << "- A1 did not return conflict");
  smtkTest(probAtt == attA, prefix << "- A1 did not return attA as conflicting");
  smtkTest(
    attC->associations()->contains(attTest),
    prefix << "- C does not think it is associated with attTest")
}

int unitAttributeAssociationConstraints(int /*unused*/, char* /*unused*/[])
{
  // ----
  // I. Let's create an attribute resource and some definitions
  attribute::ResourcePtr attRes = attribute::Resource::create();
  DefinitionPtr aDef, a1Def, bDef, cDef, dDef, testDef;
  testDef = attRes->createDefinition("testDef");
  aDef = attRes->createDefinition("A");
  auto arule = aDef->createLocalAssociationRule();
  arule->setAcceptsEntries(
    smtk::common::typeName<attribute::Resource>(), "attribute[type='testDef']", true);
  arule->setIsExtensible(true);
  aDef->setIsUnique(true);
  a1Def = attRes->createDefinition("A1", aDef);

  bDef = attRes->createDefinition("B");
  arule = bDef->createLocalAssociationRule();
  arule->setAcceptsEntries(
    smtk::common::typeName<attribute::Resource>(), "attribute[type='testDef']", true);
  arule->setIsExtensible(true);

  cDef = attRes->createDefinition("C");
  arule = cDef->createLocalAssociationRule();
  arule->setAcceptsEntries(
    smtk::common::typeName<attribute::Resource>(), "attribute[type='testDef']", true);
  arule->setIsExtensible(true);

  dDef = attRes->createDefinition("D");
  // Let's setup some exclusions and prerequisites
  // So A excludes B, B excludes A and C, C excludes B
  // and A has C as a prerequisite

  aDef->addExclusion(bDef);
  aDef->addPrerequisite(cDef);
  bDef->addExclusion(cDef);
  AttributePtr attTest = attRes->createAttribute("testAtt", "testDef");
  AttributePtr attA = attRes->createAttribute("a", "A");
  AttributePtr attA1 = attRes->createAttribute("a1", "A1");
  AttributePtr attB = attRes->createAttribute("b", "B");
  AttributePtr attC = attRes->createAttribute("c", "C");
  AttributePtr attC1 = attRes->createAttribute("c1", "C");

  // First tests - Illegal associations:
  // dDef should not be allowed to be associated with testAtt but bDef and cDef should
  DefinitionPtr preDef;
  AttributePtr probAtt;
  smtkTest(
    aDef->hasPrerequisite(cDef) != nullptr, "Association Rule Test - C is not a prerequisite of A");
  smtkTest(
    a1Def->hasPrerequisite(cDef) != nullptr,
    "Association Rule Test - C is not a prerequisite of A1");
  smtkTest(
    cDef->hasPrerequisite(aDef) == nullptr,
    "Association Rule Test - A thinks its a prerequisite of C");
  auto result = bDef->canBeAssociated(attTest, probAtt, preDef);
  smtkTest(
    result == Definition::AssociationResultType::Valid,
    "Association Rule Test - B did not return valid");
  result = cDef->canBeAssociated(attTest, probAtt, preDef);
  smtkTest(
    result == Definition::AssociationResultType::Valid,
    "Association Rule Test - C did not return valid");
  result = dDef->canBeAssociated(attTest, probAtt, preDef);
  smtkTest(
    result == Definition::AssociationResultType::Illegal,
    "Association Rule Test - D did not return illegal");
  result = aDef->canBeAssociated(attTest, probAtt, preDef);
  smtkTest(
    result == Definition::AssociationResultType::Prerequisite,
    "Association Rule Test - A did not return prerequisite");
  smtkTest(preDef == cDef, "Association Rule Test - A did not return cDef as missing prerequisite");
  result = a1Def->canBeAssociated(attTest, probAtt, preDef);
  smtkTest(
    result == Definition::AssociationResultType::Prerequisite,
    "Association Rule Test - A1 did not return prerequisite");
  smtkTest(
    preDef == cDef, "Association Rule Test - A1 did not return cDef as missing prerequisite");
  // Next Tests - Associating C should allow A,  A1, C but not B
  // Let's associate c to attTest
  smtkTest(attC->associate(attTest), "Could not associate attC to attTest");
  result = bDef->canBeAssociated(attTest, probAtt, preDef);
  smtkTest(
    result == Definition::AssociationResultType::Conflict,
    "Association Rule Test Pass 2 - B did not return conflict");
  smtkTest(probAtt == attC, "Association Rule Test Pass 2 - B did not return attC as conflicting");
  result = cDef->canBeAssociated(attTest, probAtt, preDef);
  smtkTest(
    result == Definition::AssociationResultType::Valid,
    "Association Rule Test Pass 2 - C did not return valid");
  result = aDef->canBeAssociated(attTest, probAtt, preDef);
  smtkTest(
    result == Definition::AssociationResultType::Valid,
    "Association Rule Test Pass 2 - A did not return valid");
  result = a1Def->canBeAssociated(attTest, probAtt, preDef);
  smtkTest(
    result == Definition::AssociationResultType::Valid,
    "Association Rule Test Pass 2 - A1 did not return valid");
  // Next let's associate A
  smtkTest(attA->associate(attTest), "Could not associate attA to attTest");
  result = aDef->canBeAssociated(attTest, probAtt, preDef);
  smtkTest(
    result == Definition::AssociationResultType::Conflict,
    "Association Rule Test Pass 3 - A did not return conflict");
  smtkTest(probAtt == attA, "Association Rule Test Pass 3 - A did not return attA as conflicting");
  result = a1Def->canBeAssociated(attTest, probAtt, preDef);
  smtkTest(
    result == Definition::AssociationResultType::Conflict,
    "Association Rule Test Pass 3 - A1 did not return conflict");
  smtkTest(probAtt == attA, "Association Rule Test Pass 3 - A1 did not return attA as conflicting");

  // Let's associate another C and test disassociating a prerequiste
  smtkTest(attC1->associate(attTest), "Could not associate attC1 to attTest");
  smtkTest(attC1->disassociate(attTest, probAtt), "Could not disassociate attC1 from attTest");
  smtkTest(!attC->disassociate(attTest, probAtt), "Was able to disassociate attC from attTest");
  smtkTest(
    probAtt == attA, "Did not think attA was the attribute preventing attC's disassociation");
  io::AttributeWriter writer;
  io::AttributeReader reader;
  io::Logger logger;
  std::string writeRroot(SMTK_SCRATCH_DIR);
  std::string fname = writeRroot + "/unitAttributeAssociationTest.sbi";
  std::string rname = writeRroot + "/unitAttributeAssociationTest.smtk";

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
  testLoadedAttributeResource(attRes, "Association Rule Test (JSON)");

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
  testLoadedAttributeResource(attRes, "Association Rule Test (XML)");

  return 0;
}
