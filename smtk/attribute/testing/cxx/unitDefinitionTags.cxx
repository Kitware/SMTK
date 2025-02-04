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
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/attribute/operators/Read.h"
#include "smtk/attribute/operators/Write.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include <array>
#include <iostream>

void prepResource(smtk::attribute::ResourcePtr& resource)
{
  auto def = resource->createDefinition("att1");
  std::set<std::string> values{ { "value1", "value2", "value3" } };
  smtk::attribute::Tag t1("My Tag"), t2("My Tag with Values", values);
  def->addTag(t1);
  def->addTag(t2);
  auto sdef = def->addItemDefinition<smtk::attribute::StringItemDefinition>("str");
  std::set<std::string> svalues{ { "v1", "v2", "v3" } };
  smtk::attribute::Tag ts1("Empty Tag"), ts2("Str Tag with Values", svalues);
  sdef->addTag(ts1);
  sdef->addTag(ts2);
  resource->finalizeDefinitions();
  auto att = resource->createAttribute("att", def);
}

bool testTags(const smtk::attribute::ResourcePtr& resource, const std::string& prefix)
{
  std::vector<smtk::attribute::AttributePtr> atts;
  resource->attributes(atts);
  if (atts.size() != 1)
  {
    std::cerr << prefix << "Unexpected number of attributes: " << atts.size() << std::endl;
    return false;
  }
  smtk::attribute::AttributePtr att = atts[0];

  smtk::attribute::DefinitionPtr def = att->definition();
  smtk::attribute::ItemDefinitionPtr idef = def->itemDefinition(0);

  bool status = true;
  const smtk::attribute::Tag* tag;
  // Test 1: make sure there are 2 tags
  {
    if (def->tags().size() != 2)
    {
      std::cerr << prefix << "Incorrect number of tags: expected 2, got " << def->tags().size()
                << std::endl;
      std::cerr << "\tTags found: ";
      for (const auto& dtag : def->tags())
      {
        std::cerr << dtag.name() << " ";
      }
      std::cerr << std::endl;
      status = false;
    }
  }

  // Test 2: make sure accessing an invalid tag returns a null pointer
  {
    tag = def->tag("My Nonexistent Tag");
    if (tag)
    {
      std::cerr << prefix << "Found \"My Nonexistent Tag\" which shouldn't exist\n";
      status = false;
    }
  }
  // Test 3: access a tag
  {
    tag = def->tag("My Tag");
    if (!tag)
    {
      std::cerr << prefix << "Could not find tag \"My Tag\"\n";
      status = false;
    }
  }

  // Test 4: access a tag and its values
  {
    tag = def->tag("My Tag with Values");
    if (!tag)
    {
      std::cerr << prefix << "Could not find tag \"My Tag with Values\"\n";
      status = false;
    }

    else if (tag->values().size() != 3)
    {
      std::cerr << prefix << "Incorrect number of tag values: " << tag->values().size()
                << " found, should have been 3\n";
      status = false;
    }
    else
    {
      std::array<std::string, 3> value{ { "value1", "value2", "value3" } };

      for (std::size_t i = 0; i < 3; i++)
      {
        if (!tag->contains(value[i]))
        {
          std::cerr << prefix << "Expected tag value not found: \"" << value[i] << "\"\n";
          status = false;
        }
      }
    }
  }
  if (idef == nullptr)
  {
    std::cerr << prefix << "Could not find the 0th ItemDefinition\n";
    return false;
  }
  // Test 1s: make sure there are 2 tags
  {
    if (idef->tags().size() != 2)
    {
      std::cerr << prefix << "Incorrect number of tags on ItemDef: expected 2, got "
                << idef->tags().size() << std::endl;
      std::cerr << "\tTags found: ";
      for (const auto& dtag : idef->tags())
      {
        std::cerr << dtag.name() << " ";
      }
      std::cerr << std::endl;
      status = false;
      ;
    }
  }

  // Test 2s: make sure accessing an invalid tag returns a null pointer
  {
    tag = idef->tag("My Nonexistent String Tag");
    if (tag)
    {
      std::cerr << prefix << "Found \"My Nonexistent String Tag\" which shouldn't exist\n";
      status = false;
    }
  }
  // Test 3s: access a tag
  {
    tag = idef->tag("Empty Tag");
    if (!tag)
    {
      std::cerr << prefix << "Could not find tag \"Empty Tag\" on ItemDef\n";
      status = false;
    }
  }

  // Test 4s: access a tag and its values
  {
    tag = idef->tag("Str Tag with Values");
    if (!tag)
    {
      std::cerr << prefix << "Could not find tag \"Str Tag with Values\" on ItemDef\n";
      status = false;
    }

    else if (tag->values().size() != 3)
    {
      std::cerr << prefix << "Incorrect number of tag values on ItemDef: " << tag->values().size()
                << " found, should have been 3\n";
      std::cerr << "\tValues are : ";
      for (const auto& v : tag->values())
      {
        std::cerr << "\"" << v << "\" ";
      }
      std::cerr << std::endl;
      status = false;
    }
    else
    {
      std::array<std::string, 3> value{ { "v1", "v2", "v3" } };

      for (std::size_t i = 0; i < 3; i++)
      {
        if (!tag->contains(value[i]))
        {
          std::cerr << prefix << "Expected tag value not found: \"" << value[i] << "\"\n";
          status = false;
        }
      }
    }
  }
  return status;
}
int unitDefinitionTags(int /*unused*/, char** const /*unused*/)
{
  smtk::attribute::ResourcePtr resource = smtk::attribute::Resource::create();
  prepResource(resource);
  smtk::io::Logger logger;
  smtk::io::AttributeReader reader;
  smtk::io::AttributeWriter writer;

  smtkTest(testTags(resource, "Initial Pass - "), "Failed testing Tags in Initial Pass");
  std::cout << "Initial Pass - Testing Tags Passed\n";

  std::string writeRroot(SMTK_SCRATCH_DIR);
  std::string fname = writeRroot + "/unitDefinitionTagsTest.sbi";
  std::string rname = writeRroot + "/unitDefinitionTagsTest.smtk";

  //Test JSON File I/O
  resource->setLocation(rname);
  smtk::attribute::Write::Ptr writeOp = smtk::attribute::Write::create();
  writeOp->parameters()->associate(resource);
  auto opresult = writeOp->operate();

  smtkTest(
    opresult->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "JSON Write operation failed\n"
      << writeOp->log().convertToString());
  resource = nullptr;
  smtk::attribute::Read::Ptr readOp = smtk::attribute::Read::create();
  readOp->parameters()->findFile("filename")->setValue(rname);
  opresult = readOp->operate();
  smtkTest(
    opresult->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "JSON Read operation failed\n"
      << writeOp->log().convertToString());
  resource = std::dynamic_pointer_cast<smtk::attribute::Resource>(
    opresult->findResource("resourcesCreated")->value());
  //Test the resource created using JSON
  smtkTest(testTags(resource, "JSON Pass - "), "Failed testing Tags in JSON Pass");
  std::cout << "JSON Pass - Testing Tags Passed\n";

  //Test XML File I/O
  writer.write(resource, fname, logger);
  smtkTest(
    !logger.hasErrors(),
    "Error Generated when XML writing file (" << fname << "):\n"
                                              << logger.convertToString());

  resource = smtk::attribute::Resource::create();
  reader.read(resource, fname, logger);
  smtkTest(
    !logger.hasErrors(),
    "Error Generated when XML reading file (" << fname << "):\n"
                                              << logger.convertToString());
  //Test the resource created using XML
  smtkTest(testTags(resource, "XML Pass - "), "Failed testing Tags in XML Pass");
  std::cout << "XML Pass - Testing Tags Passed\n";

  return 0;
}
