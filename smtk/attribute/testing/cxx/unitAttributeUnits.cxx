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

namespace
{

bool testAttributeUnits(const attribute::ResourcePtr& attRes, const std::string& prefix)
{
  bool status = true;
  // Lets find the Attributes
  AttributePtr a = attRes->findAttribute("a");
  AttributePtr b = attRes->findAttribute("b");
  AttributePtr b1 = attRes->findAttribute("b1");
  AttributePtr c = attRes->findAttribute("c");

  if (!a->definition()->units().empty())
  {
    std::cerr << prefix << "Definition A has units: " << a->definition()->units()
              << " but should have none!\n";
    status = false;
  }
  if (b->definition()->units() != "meters")
  {
    std::cerr << prefix << "Definition B has units: " << b->definition()->units()
              << " but should be in meters!\n";
    status = false;
  }
  if (c->definition()->units() != "foo")
  {
    std::cerr << prefix << "Definition C has units: " << c->definition()->units()
              << " but should be in foo!\n";
    status = false;
  }
  if (!a->units().empty())
  {
    std::cerr << prefix << "Attribute a has units: " << a->units() << " but should have none!\n";
    status = false;
  }
  if (b->units() != "meters")
  {
    std::cerr << prefix << "Attribute b has units: " << b->units() << " but should be in meters!\n";
    status = false;
  }
  if (b1->units() != "feet")
  {
    std::cerr << prefix << "Attribute b has units: " << b1->units() << " but should be in feet!\n";
    status = false;
  }
  if (c->units() != "foo")
  {
    std::cerr << prefix << "Attribute c has units: " << c->units() << " but should be in foo!\n";
    status = false;
  }
  return status;
}

} // namespace

int unitAttributeUnits(int /*unused*/, char* /*unused*/[])
{
  int status = 0;
  // I. Let's create an attribute resource and some definitions and attributes
  attribute::ResourcePtr attRes = attribute::Resource::create();

  // A will not have any units specified
  DefinitionPtr A = attRes->createDefinition("A");
  // B will have a unit of length (meters)
  DefinitionPtr B = attRes->createDefinition("B");
  B->setUnits("meters");
  // C will have non supported units (foo)
  DefinitionPtr C = attRes->createDefinition("C");
  C->setUnits("foo");

  // Lets create some attributes
  auto a = attRes->createAttribute("a", "A");
  auto b = attRes->createAttribute("b", "B");
  auto b1 = attRes->createAttribute("b1", "B");
  auto c = attRes->createAttribute("c", "C");

  // Lets do some unit assignments
  // Should not be able to assign any units to an attribute whose
  // definition has no units
  if (a->setLocalUnits("feet"))
  {
    std::cerr << "Error - was able to set a to be in feet!\n";
    status = -1;
  }
  // Should not be able to assign temperature units to an attribute
  // whose definition is in meters
  if (b->setLocalUnits("K"))
  {
    std::cerr << "Error - was able to set b to be in Kelvin!\n";
    status = -1;
  }
  // Should not be able to assign unsupported units to an attribute
  // whose definition is in meters
  if (b->setLocalUnits("foo"))
  {
    std::cerr << "Error - was able to set b to be in foo!\n";
    status = -1;
  }
  // Should be able to assign the units of feet to an attribute
  // whose definition is in meters
  if (!b1->setLocalUnits("feet"))
  {
    std::cerr << "Error - was not able to set b1 to be in feet!\n";
    status = -1;
  }
  // Should not be able to assign units to an attribute
  // whose definition's units are not supported
  if (c->setLocalUnits("feet"))
  {
    std::cerr << "Error - was able to set c to be in feet!\n";
    status = -1;
  }

  if (status == 0)
  {
    std::cerr << "Initial Unit Assignment Tests - Passed!\n";
  }

  if (testAttributeUnits(attRes, "First Pass - "))
  {
    std::cerr << "Creation Pass - Passed!\n";
  }
  else
  {
    std::cerr << "Creation Pass - Failed!\n";
    status = -1;
  }
  io::AttributeWriter writer;
  io::AttributeReader reader;
  io::Logger logger;
  std::string writeRroot(SMTK_SCRATCH_DIR);
  std::string fname = writeRroot + "/unitAttributeUnits.sbi";
  std::string rname = writeRroot + "/unitAttributeUnits.smtk";

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
  if (testAttributeUnits(attRes, "JSON Pass - "))
  {
    std::cerr << "JSON Pass - Passed!\n";
  }
  else
  {
    std::cerr << "JSON Pass - Failed!\n";
    status = -1;
  }

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
  if (testAttributeUnits(attRes, "XML Pass - "))
  {
    std::cerr << "XML Pass - Passed!\n";
  }
  else
  {
    std::cerr << "XML Pass - Failed!\n";
    status = -1;
  }

  return status;
}
