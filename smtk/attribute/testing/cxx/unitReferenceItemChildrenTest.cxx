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
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/FileItem.h"
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
#include <iostream>

using namespace smtk::attribute;
using namespace smtk;

namespace
{
void setupAttributeResource(ResourcePtr& attRes)
{
  // Create a base material definition, followed by 3 derived
  // definitions for solid, liquid, and void materials
  DefinitionPtr material = attRes->createDefinition("Material");
  DefinitionPtr solidM = attRes->createDefinition("SolidMaterial", material);
  DefinitionPtr liquidM = attRes->createDefinition("LiquidMaterial", material);
  DefinitionPtr voidM = attRes->createDefinition("VoidMaterial", material);

  // Create a body information definition that will have a component item.
  // If the component item is assigned a solid material, then it
  // should have a child item called initialTemp.
  // If it is assigned a liquid material, it should also have a child item called
  // initialFlow.
  // If it is assigned a void material, it should have no children.
  DefinitionPtr body = attRes->createDefinition("BodyInfo");

  ComponentItemDefinitionPtr materialItem =
    body->addItemDefinition<ComponentItemDefinition>("material");
  materialItem->setNumberOfRequiredValues(1);
  std::string attQuery = smtk::attribute::Resource::createAttributeQuery(material);
  materialItem->setAcceptsEntries(
    smtk::common::typeName<smtk::attribute::Resource>(), attQuery, true);

  // Setup the children/conditionals
  auto flowItem = materialItem->addItemDefinition<DoubleItemDefinition>("initialFlow");
  flowItem->setNumberOfRequiredValues(3);
  auto tempItem = materialItem->addItemDefinition<DoubleItemDefinition>("initialTemp");
  tempItem->setNumberOfRequiredValues(1);
  std::vector<std::string> children;
  children.emplace_back("initialTemp");
  std::size_t index = materialItem->addConditional(
    smtk::common::typeName<smtk::attribute::Resource>(),
    smtk::attribute::Resource::createAttributeQuery(solidM),
    children);
  if (index != 0)
  {
    std::cerr << "Incorrect Index returned for Solid Conditional = " << index << " should be 0\n";
  }

  children.emplace_back("initialFlow");
  index = materialItem->addConditional(
    "", smtk::attribute::Resource::createAttributeQuery(liquidM), children);
  if (index != 1)
  {
    std::cerr << "Incorrect Index returned for Liquid Conditional = " << index << " should be 1\n";
  }

  //Lets try to create an invalid conditional and make sure it wasn't added.
  children.emplace_back("foo"); // invalid item name
  index = materialItem->addConditional(
    "", smtk::attribute::Resource::createAttributeQuery(voidM), children);
  if (index != ReferenceItemDefinition::s_invalidIndex)
  {
    std::cerr << "Incorrect Index returned for Invalid Conditional = " << index << std::endl;
  }

  auto solAtt = attRes->createAttribute("solid", solidM);
  attRes->createAttribute("liquid", liquidM);
  attRes->createAttribute("void", voidM);
  auto binfoAtt = attRes->createAttribute("bodyInfo", body);
  ComponentItemPtr matItem = binfoAtt->findComponent("material");
  // Lets set the material item to be initially set to solid
  matItem->setValue(solAtt);
}

// Tests to see if the item is set to the proper value and if
// the number of conditional children match
bool testMaterialItem(
  const ComponentItemPtr& matItem,
  const AttributePtr matAtt,
  std::size_t numChildren)
{
  if (matItem->value() != matAtt)
  {
    std::cerr << "\tmaterialItem not set to " << matAtt->name() << std::endl;
    if (matItem->value())
    {
      std::cerr << matItem->value()->name() << std::endl;
    }
    else
    {
      std::cerr << " is not set!\n";
    }
    return false;
  }

  if (matItem->numberOfActiveChildrenItems() != numChildren)
  {
    std::cerr << "\tmaterialItem returned incorrect number of active children for "
              << matAtt->name() << " returned: " << matItem->numberOfActiveChildrenItems()
              << " should have been " << numChildren << std::endl;
    return false;
  }
  return true;
}

bool testResource(const ResourcePtr& attRes, const std::string& prefix)
{
  bool status = true;
  // Lets find the Attributes
  std::cerr << prefix << std::endl;
  AttributePtr sm = attRes->findAttribute("solid");
  smtkTest(sm != nullptr, "Could not find solid material attribute!");
  AttributePtr lm = attRes->findAttribute("liquid");
  smtkTest(lm != nullptr, "Could not find liquid material attribute!");
  AttributePtr vm = attRes->findAttribute("void");
  smtkTest(vm != nullptr, "Could not find void material attribute!");
  AttributePtr binfo = attRes->findAttribute("bodyInfo");
  smtkTest(binfo != nullptr, "Could not find bodyInfo attribute!");
  ComponentItemPtr matItem = binfo->findComponent("material");
  smtkTest(matItem != nullptr, "Could not find bodyInfo's material item!");
  std::cerr << "\tAll Attributes/Items found!\n";
  // The initial setting should be set to solid
  status = status && testMaterialItem(matItem, sm, 1);
  matItem->setValue(vm);
  status = status && testMaterialItem(matItem, vm, 0);
  matItem->setValue(lm);
  status = status && testMaterialItem(matItem, lm, 2);
  matItem->setValue(sm);
  status = status && testMaterialItem(matItem, sm, 1);
  if (status)
  {
    std::cerr << "\tTests Passed!\n";
  }
  return status;
}
} // namespace

int unitReferenceItemChildrenTest(int /*unused*/, char* /*unused*/[])
{
  //
  // I. Let's create an attribute resource and some definitions
  attribute::ResourcePtr attRes = attribute::Resource::create();
  setupAttributeResource(attRes);

  smtkTest(
    testResource(attRes, "First Pass - "), "Failed checking Conditional Children in First Pass");
  io::AttributeWriter writer;
  io::AttributeReader reader;
  io::Logger logger;
  std::string writeRroot(SMTK_SCRATCH_DIR);
  std::string fname = writeRroot + "/unitAttributeReferenceItemChildrenTest.sbi";
  std::string rname = writeRroot + "/unitAttributeReferenceItemChildrenTest.smtk";

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
  smtkTest(
    testResource(attRes, "JSON Pass - "), "Failed checking Conditional Children in JSON Pass");

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
  smtkTest(testResource(attRes, "XML Pass - "), "Failed checking Conditional Children in XML Pass");

  return 0;
}
