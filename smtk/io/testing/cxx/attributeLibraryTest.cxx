//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <iostream>
#include <sstream>

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/common/UUID.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

int main()
{
  smtk::attribute::ResourcePtr resource = smtk::attribute::Resource::create();
  // Lets create some definitions and attributes
  auto aDef = resource->createDefinition("A");
  auto bDef = resource->createDefinition("B", aDef);
  auto cDef = resource->createDefinition("C", aDef);
  auto dDef = resource->createDefinition("D", bDef);
  auto eDef = resource->createDefinition("E", bDef);
  auto fDef = resource->createDefinition("F");
  auto gDef = resource->createDefinition("G", fDef);
  auto hDef = resource->createDefinition("H");

  resource->finalizeDefinitions();
  auto att = resource->createAttribute("a", aDef);
  att = resource->createAttribute("b", bDef);
  att = resource->createAttribute("c", cDef);
  att = resource->createAttribute("d", dDef);
  att = resource->createAttribute("e", eDef);
  att = resource->createAttribute("f", fDef);
  att = resource->createAttribute("g", gDef);
  att = resource->createAttribute("h", hDef);

  smtk::io::Logger logger;
  smtk::io::AttributeReader reader;
  std::string fileName = SMTK_SCRATCH_DIR;
  fileName += "/attributeLibraryTest.sbi";

  smtk::io::AttributeWriter writer;
  std::vector<smtk::attribute::DefinitionPtr> defs;
  defs.push_back(gDef);
  defs.push_back(eDef);
  defs.push_back(cDef);
  defs.push_back(bDef);
  defs.push_back(aDef);

  writer.treatAsLibrary(defs);
  // Lets see f it calculated the correct set of definitions
  auto iDefs = writer.includedDefinitions();
  bool passed = true;
  if (iDefs.size() != 2)
  {
    std::cerr << "Number of included Definitions = " << iDefs.size() << " - should be 2!\n";
    std::cerr << "Included Definitions = { ";
    for (const auto& def : iDefs)
    {
      std::cerr << def->type() << " ";
    }
    std::cerr << "}\n";
    passed = false;
  }
  else
  {
    // This should only contain A and G
    if (
      ((iDefs[0]->type() != "A") && (iDefs[1]->type() != "A")) ||
      ((iDefs[0]->type() != "G") && (iDefs[1]->type() != "G")))
    {
      std::cerr << "Incorrect included Definitions returned : { " << iDefs[0]->type() << ", "
                << iDefs[1]->type() << " } instead of {A, G}\n";
      passed = false;
    }
    else
    {
      std::cerr << "Included Definitions - PASSED!\n";
    }
  }
  // Save library
  if (writer.write(resource, fileName, logger))
  {
    std::cerr << "Failed to write to " << fileName << "\n";
    std::cerr << logger.convertToString();
    return -1;
  }

  // Lets remove all attributes from the resource
  std::vector<smtk::attribute::AttributePtr> atts;
  resource->attributes(atts);
  for (auto& a : atts)
  {
    resource->removeAttribute(a);
  }

  // Now lets load in the Library
  if (reader.read(resource, fileName, logger))
  {
    std::cerr << "Failed to read from " << fileName << "\n";
    std::cerr << logger.convertToString();
    return -1;
  }

  resource->attributes(atts);
  if (atts.size() != 6)
  {
    std::cerr << "Unexpected number of attributes in Library: " << atts.size()
              << " - should be 6\n";
    std::cerr << "Atts = { ";
    for (const auto& a : atts)
    {
      std::cerr << a->name() << " ";
    }
    std::cerr << "} - should be { a b c d e g }\n";
    passed = false;
  }
  else
  {
    std::vector<std::string> names{ "a", "b", "c", "d", "e", "g" };
    bool foundAll = true;
    for (const auto& n : names)
    {
      att = resource->findAttribute(n);
      if (att == nullptr)
      {
        std::cerr << "Could not find attribute: " << n << std::endl;
        foundAll = false;
      }
    }
    if (!foundAll)
    {
      std::cerr << "Incorrect Attributes read from Library = { ";
      for (const auto& a : atts)
      {
        std::cerr << a->name() << " ";
      }
      std::cerr << "} - should be { a b c e g }\n";
      passed = false;
    }
  }
  if (passed)
  {
    std::cerr << "Test Passed!\n";
    return 0;
  }
  return -1;
}
