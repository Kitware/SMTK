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
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include <array>
#include <iostream>

namespace
{
const char* testInput =
  "<?xml version=\"1.0\" encoding=\"utf-8\" ?>                                   "
  "<SMTK_AttributeResource Version=\"3\">                                          "
  "  <Definitions>                                                               "
  "    <AttDef Type=\"att1\" BaseType=\"\">                                      "
  "      <Tags>                                                                  "
  "        <Tag Name=\"My Tag\" />                                               "
  "        <Tag Name=\"My Tag with Values\">value1,value2,value3</Tag>           "
  "      </Tags>                                                                 "
  "      <ItemDefinitions>                                                       "
  "	  <String Name=\"normalString\" Extensible=\"0\"                         "
  "               NumberOfRequiredValues=\"1\">                                  "
  "         <DefaultValue>normal</DefaultValue>                                  "
  "	  </String>                                                              "
  "      </ItemDefinitions>                                                      "
  "    </AttDef>                                                                 "
  "  </Definitions>                                                              "
  "  <Attributes>                                                                "
  "    <Att Name=\"att\" Type=\"att1\"/>                                         "
  "  </Attributes>                                                               "
  "</SMTK_AttributeResource>                                                       ";
}

int main()
{
  smtk::attribute::ResourcePtr resptr = smtk::attribute::Resource::create();
  smtk::attribute::Resource& resource(*resptr.get());
  smtk::io::Logger logger;
  smtk::io::AttributeReader reader;

  if (reader.readContents(resptr, testInput, logger))
  {
    std::cerr << "Encountered Errors while reading input data\n";
    std::cerr << logger.convertToString();
    return -2;
  }

  std::vector<smtk::attribute::AttributePtr> atts;
  resource.attributes(atts);
  if (atts.size() != 1)
  {
    std::cerr << "Unexpected number of attributes: " << atts.size() << "\n";
    std::cerr << logger.convertToString();
    return -2;
  }
  smtk::attribute::AttributePtr att = atts[0];

  smtk::attribute::DefinitionPtr def = att->definition();

  for (auto& tag : def->tags())
  {
    std::cout << "tag: " << tag.name() << std::endl;
  }

  // Test 1: make sure there are 2 tags
  {
    if (def->tags().size() != 2)
    {
      std::cerr << "Incorrect number of tags: expected 2, got " << def->tags().size() << "\n";
      std::cerr << logger.convertToString();
      return -1;
    }
  }

  // Test 2: access a tag
  const smtk::attribute::Tag* tag;
  {
    tag = def->tag("My Tag");
    if (!tag)
    {
      std::cerr << "Could not find tag \"My Tag\"\n";
      std::cerr << logger.convertToString();
      return -1;
    }
  }

  // Test 3: access a tag and its values
  {
    tag = def->tag("My Tag with Values");
    if (!tag)
    {
      std::cerr << "Could not find tag \"My Tag\"\n";
      std::cerr << logger.convertToString();
      return -1;
    }

    if (tag->values().size() != 3)
    {
      std::cerr << "Incorrect number of tag values\n";
      std::cerr << logger.convertToString();
      return -1;
    }

    std::array<std::string, 3> value{ { "value1", "value2", "value3" } };

    for (std::size_t i = 0; i < 3; i++)
    {
      if (!tag->has(value[i]))
      {
        std::cerr << "Expected tag value not found: \"" << value[i] << "\"\n";
        std::cerr << logger.convertToString();
        return -1;
      }
    }
  }

  // Test 4: make sure accessing an invalid tag returns a null pointer
  {
    tag = def->tag("My Nonexistent Tag");
    if (tag)
    {
      std::cerr << "Found \"My Nonexistent Tag\" which doesn't exist\n";
      std::cerr << logger.convertToString();
      return -1;
    }
  }

  return 0;
}
